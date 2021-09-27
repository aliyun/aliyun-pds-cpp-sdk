/*
 * Copyright 2009-2017 Alibaba Cloud All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <alibabacloud/pds/model/FileUploadRequest.h>
#include <alibabacloud/pds/model/FileCreateRequest.h>
#include <alibabacloud/pds/model/FileGetUploadUrlRequest.h>
#include <alibabacloud/pds/model/DataPutByUrlRequest.h>
#include <alibabacloud/pds/model/FileListUploadedPartsRequest.h>
#include <alibabacloud/pds/model/FileCompleteRequest.h>
#include <alibabacloud/pds/PdsFwd.h>
#include <alibabacloud/pds/Const.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <set>
#include "../external/json/json.h"
#include "../utils/FileSystemUtils.h"
#include "../utils/Utils.h"
#include "../utils/LogUtils.h"
#include "../utils/Crc64.h"
#include "../PdsClientImpl.h"
#include "../model/ModelError.h"
#include "ResumableUploader.h"

using namespace AlibabaCloud::PDS;


ResumableUploader::ResumableUploader(const FileUploadRequest& request, const PdsClientImpl *client) :
    ResumableBaseWorker(request.FileSize(), request.PartSize()),
    request_(request),
    client_(client)
{
    if (!request.FilePath().empty()) {
        time_t lastMtime;
        std::streamsize fileSize;
        if (GetPathInfo(request.FilePath(), lastMtime, fileSize)) {
            fileSize_ = static_cast<uint64_t>(fileSize);
        }
    }
#ifdef _WIN32
    else if (!request.FilePathW().empty()) {
        time_t lastMtime;
        std::streamsize fileSize;
        if (GetPathInfo(request.FilePathW(), lastMtime, fileSize)) {
            fileSize_ = static_cast<uint64_t>(fileSize);
        }
    }
#endif
}

FileCompleteOutcome ResumableUploader::Upload()
{
    PdsError err;

    if (0 != validate(err)) {
        return FileCompleteOutcome(err);
    }

    PartList partsToUpload;
    PartList uploadedParts;
    if (getPartsToUpload(err, uploadedParts, partsToUpload) != 0){
        return FileCompleteOutcome(err);
    }

    std::vector<DataPutOutcome> outcomes;

    // 顺序上传分片
    Part part;
    while (true) {
        // need lock when parallel upload
        {
            // std::lock_guard<std::mutex> lck(lock_);
            if (partsToUpload.empty())
                break;
            part = partsToUpload.front();
            partsToUpload.erase(partsToUpload.begin());
        }

        if (!client_->isEnableRequest())
            break;

        uint64_t offset = partSize_ * (part.PartNumber() - 1);
        uint64_t length = part.PartSize();

        auto content = GetFstreamByPath(request_.FilePath(), request_.FilePathW(),
            std::ios::in | std::ios::binary);
        content->seekg(offset, content->beg);

        PartInfoReqList partInfoReqList;
        PartInfoReq info(part.PartNumber(), part.PartSize(), offset, offset+length);
        partInfoReqList.push_back(info);
        FileGetUploadUrlRequest getUploadPartUrlRequest(driveID_, fileID_, uploadID_, partInfoReqList);
        auto getUploadPartUrlOutcome = FileGetUploadUrlWrap(getUploadPartUrlRequest);
        if (!getUploadPartUrlOutcome.isSuccess()) {
            return FileCompleteOutcome(getUploadPartUrlOutcome.error());
        }

        auto partInfoResp = getUploadPartUrlOutcome.result().PartInfoRespList();
        if (partInfoResp.empty()) {
            return FileCompleteOutcome(PdsError("Get Upload Url error", "Get Upload url empty."));
        }

        DataPutByUrlRequest putPartRequest(partInfoResp[0].UploadUrl(), content);
        putPartRequest.setContentLength(length);
        auto process = request_.TransferProgress();
        if (process.Handler) {
            TransferProgress uploadPartProcess = { UploadPartProcessCallback, (void *)this };
            putPartRequest.setTransferProgress(uploadPartProcess);
        }
        if (request_.TrafficLimit() != 0) {
            putPartRequest.setTrafficLimit(request_.TrafficLimit());
        }
        auto putPartOutcome = UploadPartWrap(putPartRequest);
#ifdef ENABLE_PDS_TEST
        if (!!(request_.Flags() & 0x40000000) && part.PartNumber() == 2) {
            const char* TAG = "ResumableUploadClient";
            PDS_LOG(LogLevel::LogDebug, TAG, "NO.2 part data upload failed.");
            outcome = DataPutOutcome();
        }
#endif // ENABLE_PDS_TEST

        if (putPartOutcome.isSuccess()) {
            part.eTag_  = putPartOutcome.result().ETag();
            part.cRC64_ = putPartOutcome.result().CRC64();
        }

        // need lock when parallel upload
        {
            // std::lock_guard<std::mutex> lck(lock_);
            uploadedParts.push_back(part);
            outcomes.push_back(putPartOutcome);
        }
    }

    if (!client_->isEnableRequest()) {
        return FileCompleteOutcome(PdsError("ClientError:100002", "Disable all requests by upper."));
    }

    for (const auto& outcome : outcomes) {
        if (!outcome.isSuccess()) {
            return FileCompleteOutcome(outcome.error());
        }
    }

    if (uploadedParts.size() < outcomes.size()) {
        return FileCompleteOutcome(PdsError("UploadNotComplete", "Not all parts are uploaded."));
    }

    // sort uploadedParts
    std::sort(uploadedParts.begin(), uploadedParts.end(), [&](const Part& a, const Part& b)
    {
        return a.PartNumber() < b.PartNumber();
    });

    FileCompleteRequest completeReq(driveID_, fileID_, uploadID_);
    auto completeOutcome = FileCompleteWrap(completeReq);
    if (!completeOutcome.isSuccess()) {
        return FileCompleteOutcome(completeOutcome.error());
    }

    // TODO: file check, sha1 cacl, rapidupload

    removeRecordFile();

    return completeOutcome;
}

int ResumableUploader::prepare(PdsError& err)
{
    determinePartSize();
    FileCreateRequest fileCreateReq = FileCreateRequest(request_.DriveID(), request_.ParentFileID(), request_.Name(),
        request_.FileID(), request_.CheckNameMode(), fileSize_);
    auto outcome = FileCreateWrap(fileCreateReq);
    if(!outcome.isSuccess()){
        err = outcome.error();
        return -1;
    }

    if (outcome.result().Exist()) {
        err = PdsError("Same name file exist", "Same name file exist.");
        return -1;
    }

    //init record_
    driveID_ = request_.DriveID();
    fileID_ = outcome.result().FileID();
    uploadID_ = outcome.result().UploadID();

    if (hasRecordPath()) {
        Json::Value root;

        initRecordInfo();
        dumpRecordInfo(root);

        std::stringstream ss;
        ss << root;
        std::string md5Sum = ComputeContentETag(ss);
        root["md5Sum"] = md5Sum;

        auto recordStream = GetFstreamByPath(recordPath_, recordPathW_, std::ios::out);
        if (recordStream->is_open()) {
            *recordStream << root;
            recordStream->close();
        }
    }
    return 0;
}

int ResumableUploader::validateRecord()
{
    if (record_.size != fileSize_ || record_.mtime != request_.FileMtime()){
        return ARG_ERROR_UPLOAD_FILE_MODIFIED;
    }

    Json::Value root;

    dumpRecordInfo(root);

    std::stringstream recordStream;
    recordStream << root;

    std::string md5Sum = ComputeContentETag(recordStream);
    if (md5Sum != record_.md5Sum){
        return ARG_ERROR_UPLOAD_RECORD_INVALID;
    }
    return 0;
}

int ResumableUploader::loadRecord()
{
    auto recordStream = GetFstreamByPath(recordPath_, recordPathW_, std::ios::in);
    if (recordStream->is_open()){
        Json::Value root;
        Json::CharReaderBuilder rbuilder;
        std::string errMsg;
        if (!Json::parseFromStream(rbuilder, *recordStream, &root, &errMsg))
        {
            return ARG_ERROR_PARSE_UPLOAD_RECORD_FILE;
        }

        buildRecordInfo(root);

        partSize_ = record_.partSize;
        driveID_ = record_.driveID;
        fileID_ = record_.fileID;
        uploadID_ = record_.uploadID;
        hasRecord_ = true;

        recordStream->close();
    }

    return 0;
}

void ResumableUploader::genRecordPath()
{
    recordPath_  = "";
    recordPathW_ = L"";

    if (!request_.hasCheckpointDir())
        return;

    auto srcPath = !request_.FilePathW().empty()? toString(request_.FilePathW()): request_.FilePath();
    std::stringstream ss;
    if (request_.FileID().empty()) {
        // create file
        ss << "pds://" << request_.DriveID() << "/" << request_.ParentFileID() << "/" << request_.Name();
    }
    else {
        // edit file
        ss << "pds://" << request_.DriveID() << "/" << request_.FileID();
    }

    auto destPath = ss.str();

    auto safeFileName = ComputeContentETag(srcPath) + "--" + ComputeContentETag(destPath);

    if (!request_.CheckpointDirW().empty()) {
        recordPathW_ = request_.CheckpointDirW() + WPATH_DELIMITER + toWString(safeFileName);;
    }
    else {
        recordPath_ = request_.CheckpointDir() + PATH_DELIMITER + safeFileName;
    }
}

int ResumableUploader::getPartsToUpload(PdsError &err, PartList &partsUploaded, PartList &partsToUpload)
{
    std::set<uint64_t> partNumbersUploaded;

    if(hasRecord_){
        int64_t marker = 0 ;
        auto listPartsRequest = FileListUploadedPartsRequest(driveID_, fileID_, uploadID_, marker, 100);
        while(true){
            listPartsRequest.setMarker(marker);
            auto outcome = ListUploadedPartsWrap(listPartsRequest);
            if(!outcome.isSuccess()){
                err = outcome.error();
                return -1;
            }

            auto parts = outcome.result().PartList();
            for(auto iter = parts.begin(); iter != parts.end(); iter++){
                partNumbersUploaded.insert(iter->PartNumber());
                partsUploaded.emplace_back(*iter);
                consumedSize_ += iter->PartSize();
            }

            if(!outcome.result().NextMarker().empty()){
                marker = atoll(outcome.result().NextMarker().c_str());
            }else{
                break;
            }
        }
    }

    int32_t partCount = (int32_t)((fileSize_ - 1)/ partSize_ + 1);
    for(int32_t i = 0; i < partCount; i++){
        Part part;
        part.partNumber_ = i+1;
        if (i == partCount -1 ){
            part.paretSize_ = fileSize_ - partSize_ * (partCount - 1);
        }else{
            part.paretSize_ = partSize_;
        }

        auto iterNum = partNumbersUploaded.find(part.PartNumber());
        if (iterNum == partNumbersUploaded.end()){
            partsToUpload.push_back(part);
        }
    }

    return 0;
}

void ResumableUploader::initRecordInfo()
{
    auto filePath = request_.FilePath();
    if (!request_.FilePathW().empty()) {
        filePath = toString(request_.FilePathW());
    }

    record_.opType = "ResumableUpload";
    record_.driveID = driveID_;
    record_.fileID = fileID_;
    record_.uploadID = uploadID_;
    record_.name = request_.Name();
    record_.filePath = filePath;
    record_.mtime = request_.FileMtime();
    record_.size = fileSize_;
    record_.partSize = partSize_;
}

void ResumableUploader::buildRecordInfo(const AlibabaCloud::PDS::Json::Value& root)
{
    record_.opType = root["opType"].asString();
    record_.driveID = root["driveID"].asString();
    record_.fileID = root["fileID"].asString();
    record_.uploadID = root["uploadID"].asString();
    record_.name = root["name"].asString();
    record_.filePath = root["filePath"].asString();
    record_.size = root["size"].asUInt64();
    record_.mtime = root["mtime"].asString();
    record_.partSize = root["partSize"].asUInt64();
    record_.md5Sum = root["md5Sum"].asString();
}

void ResumableUploader::dumpRecordInfo(AlibabaCloud::PDS::Json::Value& root)
{
    root["opType"] = record_.opType;
    root["driveID"] = record_.driveID;
    root["fileID"] = record_.fileID;
    root["uploadID"] = record_.uploadID;
    root["name"] = record_.name;
    root["filePath"] = record_.filePath;
    root["mtime"] = record_.mtime;
    root["size"] = record_.size;
    root["partSize"] = record_.partSize;
}

void ResumableUploader::UploadPartProcessCallback(size_t increment, int64_t transfered, int64_t total, void *userData)
{
    UNUSED_PARAM(transfered);
    UNUSED_PARAM(total);

    auto uploader = (ResumableUploader*)userData;
    // need lock when parallel upload
    // std::lock_guard<std::mutex> lck(uploader->lock_);
    uploader->consumedSize_ += increment;

    auto process = uploader->request_.TransferProgress();
    if (process.Handler) {
        process.Handler(increment, uploader->consumedSize_, uploader->fileSize_, process.UserData);
    }
}

FileCreateOutcome ResumableUploader::FileCreateWrap(const FileCreateRequest &request) const
{
    return client_->FileCreate(request);
}

FileGetUploadUrlOutcome ResumableUploader::FileGetUploadUrlWrap(const FileGetUploadUrlRequest &request) const
{
    return client_->FileGetUploadUrl(request);
}

DataPutOutcome ResumableUploader::UploadPartWrap(const DataPutByUrlRequest &request) const
{
    return client_->DataPutByUrl(request);
}

FileListUploadedPartsOutcome ResumableUploader::ListUploadedPartsWrap(const FileListUploadedPartsRequest &request) const
{
    return client_->FileListUploadedParts(request);
}

FileCompleteOutcome ResumableUploader::FileCompleteWrap(const FileCompleteRequest &request) const
{
    return client_->FileComplete(request);
}
