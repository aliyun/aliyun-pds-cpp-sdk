/*
 * Copyright 2009-2021 Alibaba Cloud All rights reserved.
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
    FileCompleteOutcome validateOutcome;
    if (0 != validate(validateOutcome)) {
        return validateOutcome;
    }

    PdsError err;

    UploadPartRecordList partsToUpload;
    UploadPartRecordList uploadedParts;
    if (getPartsToUpload(err, uploadedParts, partsToUpload) != 0){
        return FileCompleteOutcome(err);
    }

    std::vector<DataPutOutcome> outcomes;

    // 顺序上传分片
    UploadPartRecord part;
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

        // check resumable progress control
        if (UploadPartProcessControlCallback((void *)this) != 0)
            break;

        uint64_t offset = partSize_ * (part.partNumber - 1);
        uint64_t length = part.size;

        auto content = GetFstreamByPath(request_.FilePath(), request_.FilePathW(),
            std::ios::in | std::ios::binary);
        content->seekg(offset, content->beg);

        PartInfoReqList partInfoReqList;
        PartInfoReq info(part.partNumber, part.size, offset, offset+length);
        partInfoReqList.push_back(info);
        FileGetUploadUrlRequest getUploadPartUrlRequest(record_.driveID, record_.fileID, record_.uploadID, partInfoReqList);
        auto getUploadPartUrlOutcome = FileGetUploadUrlWrap(getUploadPartUrlRequest);
        if (!getUploadPartUrlOutcome.isSuccess()) {
            return FileCompleteOutcome(getUploadPartUrlOutcome.error());
        }

        auto partInfoResp = getUploadPartUrlOutcome.result().PartInfoRespList();
        if (partInfoResp.empty()) {
            return FileCompleteOutcome(PdsError("GetUploadUrlError", "Get Upload url empty."));
        }

        DataPutByUrlRequest putPartRequest(partInfoResp[0].UploadUrl(), content);
        putPartRequest.setContentLength(length);
        putPartRequest.setFlags(putPartRequest.Flags() | REQUEST_FLAG_CHECK_CRC64 | REQUEST_FLAG_SAVE_CLIENT_CRC64);
        auto process = request_.TransferProgress();
        if (process.Handler) {
            TransferProgress uploadPartProcess = { UploadPartProcessCallback, (void *)this };
            putPartRequest.setTransferProgress(uploadPartProcess);
        }
        auto progressControl = request_.ProgressControl();
        if (progressControl.Handler) {
            ProgressControl uploadPartProgressControl = { UploadPartProcessControlCallback, (void *)this };
            putPartRequest.setProgressControl(uploadPartProgressControl);
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

        // local record lack of this uploaded part info, cause repeated upload
        bool partAlreadyExist = false;
        if (!putPartOutcome.isSuccess() && putPartOutcome.error().Code() == "PartAlreadyExist" &&
            putPartOutcome.error().Message().find("sequential")) {
                partAlreadyExist = true;
        }

        // need lock when parallel upload
        {
            // std::lock_guard<std::mutex> lck(lock_);
            if (putPartOutcome.isSuccess() || partAlreadyExist) {
                part.crc64 = putPartOutcome.result().CRC64();
                uploadedParts.push_back(part);
            }

            outcomes.push_back(putPartOutcome);

            //update record
            if (hasRecordPath() && (putPartOutcome.isSuccess() || partAlreadyExist)) {
                auto &record = record_;
                record.parts = uploadedParts;

                Json::Value root;
                root["opType"] = record_.opType;
                root["driveID"] = record_.driveID;
                root["fileID"] = record_.fileID;
                root["uploadID"] = record_.uploadID;
                root["name"] = record_.name;
                root["filePath"] = record_.filePath;
                root["taskID"] = record_.taskID;
                root["mtime"] = record_.mtime;
                root["size"] = record_.size;
                root["partSize"] = record_.partSize;

                int index = 0;
                for (UploadPartRecord& partR : record.parts) {
                    root["parts"][index]["partNumber"] = partR.partNumber;
                    root["parts"][index]["size"] = partR.size;
                    root["parts"][index]["crc64"] = partR.crc64;
                    index++;
                }

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
        }
    }

    if (!client_->isEnableRequest()) {
        return FileCompleteOutcome(PdsError("ClientError:100002", "Disable all requests by upper."));
    }

    int32_t controlFlag = UploadPartProcessControlCallback((void *)this);
    if (controlFlag == ProgressControlStop) {
        return FileCompleteOutcome(PdsError("ClientError:100003", "Upload stop by upper."));
    }
    if (controlFlag == ProgressControlCancel) {
        removeRecordFile();
        return FileCompleteOutcome(PdsError("ClientError:100004", "Upload cancel by upper."));
    }

    for (const auto& outcome : outcomes) {
        if (!outcome.isSuccess()) {
            // ignore PartAlreadyExist error
            if (outcome.error().Code() == "PartAlreadyExist" && outcome.error().Message().find("sequential")) {
                continue;
            }
            return FileCompleteOutcome(outcome.error());
        }
    }

    if (uploadedParts.size() < outcomes.size()) {
        return FileCompleteOutcome(PdsError("UploadNotComplete", "Not all parts are uploaded."));
    }

    // sort uploadedParts
    std::sort(uploadedParts.begin(), uploadedParts.end(), [&](const UploadPartRecord& a, const UploadPartRecord& b)
    {
        return a.partNumber < b.partNumber;
    });

    FileCompleteRequest completeReq(record_.driveID, record_.fileID, record_.uploadID);
    auto completeOutcome = FileCompleteWrap(completeReq);
    if (!completeOutcome.isSuccess()) {
        removeRecordFile();
        return FileCompleteOutcome(completeOutcome.error());
    }

    // check size
    uint64_t uploadedfileSize = completeOutcome.result().Size();
    if (fileSize_ != uploadedfileSize) {
        return FileCompleteOutcome(PdsError("FileSizeCheckError", "Resumable Upload data check size fail."));
    }

    //check crc64
    std::string crc64Hash = completeOutcome.result().Crc64Hash();
    if (client_->configuration().enableCrc64 && !crc64Hash.empty()) {
        uint64_t localCRC64 = uploadedParts[0].crc64;
        for (size_t i = 1; i < uploadedParts.size(); i++) {
            localCRC64 = CRC64::CombineCRC(localCRC64, uploadedParts[i].crc64, uploadedParts[i].size);
        }
        uint64_t serverCRC64 = std::strtoull(crc64Hash.c_str(), nullptr, 10);
        if (localCRC64 != serverCRC64) {
            // check sha1
            std::string cHash = completeOutcome.result().ContentHash();
            std::string cHashName = completeOutcome.result().ContentHashName();
            if (cHashName == "sha1" and !cHash.empty()) {
                std::string hashSHA1;
                if (0 != ComputeFileSha1(completeOutcome, hashSHA1)){
                    return completeOutcome;
                }
                transform(cHash.begin(), cHash.end(), cHash.begin(), ::toupper);
                transform(hashSHA1.begin(), hashSHA1.end(), hashSHA1.begin(), ::toupper);
                if (0 != cHash.compare(hashSHA1)) {
                    removeRecordFile();
                    return FileCompleteOutcome(PdsError("Sha1CheckError", "Resumable Upload data SHA1 checksum fail."));
                }
            }
            else {
                removeRecordFile();
                return FileCompleteOutcome(PdsError("CrcCheckError", "Resumable Upload data CRC checksum fail."));
            }
        }
    }

    removeRecordFile();

    return completeOutcome;
}

int ResumableUploader::validate(FileCompleteOutcome& completeOutcome)
{
    genRecordPath();

    if (hasRecordPath()) {
        if (0 != loadRecord()) {
            removeRecordFile();
        }
    }

    if (hasRecord_) {
        if (0 != validateRecord()) {
            removeRecordFile();
            if (0 != prepare(completeOutcome)) {
                return -1;
            }
        }
    }
    else {
        if (0 != prepare(completeOutcome)) {
            return -1;
        }
    }
    return 0;
}

int ResumableUploader::prepare(FileCompleteOutcome& completeOutcome)
{
    determinePartSize();

    FileCreateOutcome outcome;
    // rapid upload
    if (client_->configuration().enableRapidUpload) {
        // caculate pre hash
        uint64_t preHashBlockSize = 1024;
        if (preHashBlockSize > request_.FileSize()) {
            preHashBlockSize = request_.FileSize();
        }

        auto content = GetFstreamByPath(request_.FilePath(), request_.FilePathW(),
            std::ios::in | std::ios::binary);
        char streamBuffer[1024];
        uint64_t readSize = 0;
        if (!content->good()) {
            completeOutcome = FileCompleteOutcome(PdsError("ReadFileError", "Failed to read input file"));
            return -1;
        }
        content->read(streamBuffer, preHashBlockSize);
        readSize = static_cast<uint64_t>(content->gcount());
        content->close();

        if (readSize != preHashBlockSize) {
            completeOutcome = FileCompleteOutcome(PdsError("ReadFileError", "Failed to read enough size for caculate pre hash"));
            return -1;
        }
        std::string preHashSHA1 = ComputeContentSha1(streamBuffer, preHashBlockSize);

        // check pre hash request
        auto fileCreatePreCheckReq = FileCreateRequest(request_.DriveID(), request_.ParentFileID(), request_.Name(),
            request_.FileID(), request_.CheckNameMode(), request_.FileSize());
        fileCreatePreCheckReq.setPreHash(preHashSHA1);
        fileCreatePreCheckReq.setUserTags(request_.UserTags());
        auto fileCreatePreCheckOutcome = FileCreateWrap(fileCreatePreCheckReq);

        if (!fileCreatePreCheckOutcome.isSuccess()) {
            // check pre hash error
            if (fileCreatePreCheckOutcome.error().Code() != "PreHashMatched") {
                completeOutcome = FileCompleteOutcome(fileCreatePreCheckOutcome.error());
                return -1;
            }

            // check pre hash matched, rapid upload
            std::string hashSHA1;
            if (0 != ComputeFileSha1(completeOutcome, hashSHA1)){
                return -1;
            }

            // rapid upload request
            auto fileCreateRapidUploadReq = FileCreateRequest(request_.DriveID(), request_.ParentFileID(), request_.Name(),
                request_.FileID(), request_.CheckNameMode(), request_.FileSize());
            fileCreateRapidUploadReq.setContentHash(hashSHA1);
            fileCreateRapidUploadReq.setUserTags(request_.UserTags());
            auto fileCreateRapidUploadOutcome = FileCreateWrap(fileCreateRapidUploadReq);

            if (!fileCreateRapidUploadOutcome.isSuccess()) {
                completeOutcome = FileCompleteOutcome(fileCreateRapidUploadOutcome.error());
                return -1;
            }

            // rapid upload success
            if (fileCreateRapidUploadOutcome.result().RapidUpload()) {
                FileCompleteResult result(fileCreateRapidUploadOutcome.result());
                completeOutcome = FileCompleteOutcome(result);
                return -1;
            }

            // failed to rapid upload, upload data
            outcome = fileCreateRapidUploadOutcome.result();
        }
        else {
            // pre hash check not matched, upload data
            outcome = fileCreatePreCheckOutcome.result();
        }
    }
    else {
        // create by upload data
        FileCreateRequest fileCreateReq = FileCreateRequest(request_.DriveID(), request_.ParentFileID(), request_.Name(),
            request_.FileID(), request_.CheckNameMode(), fileSize_);
        fileCreateReq.setUserTags(request_.UserTags());
        auto fileCreateOutcome = FileCreateWrap(fileCreateReq);
        if (!fileCreateOutcome.isSuccess()) {
            completeOutcome = FileCompleteOutcome(fileCreateOutcome.error());
            return -1;
        }
        outcome = fileCreateOutcome.result();
    }

    if (outcome.result().Exist()) {
        completeOutcome = FileCompleteOutcome(PdsError("SameNameFileExist", "Same name file exist."));
        return -1;
    }

    if (hasRecordPath()) {
        initRecordInfo();
        record_.fileID = outcome.result().FileID();
        record_.uploadID = outcome.result().UploadID();

        Json::Value root;
        root["opType"] = record_.opType;
        root["driveID"] = record_.driveID;
        root["fileID"] = record_.fileID;
        root["uploadID"] = record_.uploadID;
        root["name"] = record_.name;
        root["filePath"] = record_.filePath;
        root["taskID"] = record_.taskID;
        root["mtime"] = record_.mtime;
        root["size"] = record_.size;
        root["partSize"] = record_.partSize;
        root["parts"].resize(0);

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
    auto record = record_;

    if (record.size != fileSize_ || record.mtime != request_.FileMtime()){
        return ARG_ERROR_UPLOAD_FILE_MODIFIED;
    }

    Json::Value root;
    root["opType"] = record.opType;
    root["driveID"] = record.driveID;
    root["fileID"] = record.fileID;
    root["uploadID"] = record.uploadID;
    root["name"] = record.name;
    root["filePath"] = record.filePath;
    root["taskID"] = record.taskID;
    root["mtime"] = record.mtime;
    root["size"] = record.size;
    root["partSize"] = record.partSize;
    root["parts"].resize(0);
    int index = 0;
    for (UploadPartRecord& part : record.parts) {
        root["parts"][index]["partNumber"] = part.partNumber;
        root["parts"][index]["size"] = part.size;
        root["parts"][index]["crc64"] = part.crc64;
        index++;
    }

    std::stringstream recordStream;
    recordStream << root;

    std::string md5Sum = ComputeContentETag(recordStream);
    if (md5Sum != record.md5Sum){
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

        record_.opType = root["opType"].asString();
        record_.driveID = root["driveID"].asString();
        record_.fileID = root["fileID"].asString();
        record_.uploadID = root["uploadID"].asString();
        record_.name = root["name"].asString();
        record_.filePath = root["filePath"].asString();
        record_.taskID = root["taskID"].asString();
        record_.size = root["size"].asUInt64();
        record_.mtime = root["mtime"].asString();
        record_.partSize = root["partSize"].asUInt64();

        UploadPartRecord part;
        for (uint32_t i = 0; i < root["parts"].size(); i++) {
            Json::Value partValue = root["parts"][i];
            part.partNumber = partValue["partNumber"].asInt();
            part.size = partValue["size"].asInt64();
            part.crc64 = partValue["crc64"].asUInt64();
            record_.parts.push_back(part);
        }
        record_.md5Sum = root["md5Sum"].asString();

        partSize_ = record_.partSize;
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
        ss << "pds://" << request_.DriveID() << "/" << request_.ParentFileID() << "/" << request_.Name() <<
            "/" << request_.TaskID();
    }
    else {
        // edit file
        ss << "pds://" << request_.DriveID() << "/" << request_.FileID() << "/" << request_.TaskID();
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

int ResumableUploader::getPartsToUpload(PdsError &err, UploadPartRecordList &partsUploaded, UploadPartRecordList &partsToUpload)
{
    UNUSED_PARAM(err);

    std::set<uint64_t> partNumbersUploaded;

    if(hasRecord_){
        for (UploadPartRecord &part : record_.parts) {
            partNumbersUploaded.insert(part.partNumber);
            partsUploaded.emplace_back(part);
            consumedSize_ += part.size;
        }
    }

    int32_t partCount = (int32_t)((fileSize_ - 1)/ partSize_ + 1);
    for(int32_t i = 0; i < partCount; i++){
        UploadPartRecord part;
        part.partNumber = i+1;
        if (i == partCount -1 ){
            part.size = fileSize_ - partSize_ * (partCount - 1);
        }
        else {
            part.size = partSize_;
        }

        auto iterNum = partNumbersUploaded.find(part.partNumber);
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
    record_.driveID = request_.DriveID();
    record_.name = request_.Name();
    record_.filePath = filePath;
    record_.taskID = request_.TaskID();
    record_.mtime = request_.FileMtime();
    record_.size = fileSize_;
    record_.partSize = partSize_;
    record_.parts.resize(0);
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

int32_t ResumableUploader::UploadPartProcessControlCallback(void *userData)
{
    auto uploader = (ResumableUploader*)userData;
    auto controller = uploader->request_.ProgressControl();
    if (controller.Handler) {
        return controller.Handler(controller.UserData);
    }
    return 0;
}

int ResumableUploader::ComputeFileSha1(FileCompleteOutcome& completeOutcome, std::string &hashSHA1)
{
    auto content = GetFstreamByPath(request_.FilePath(), request_.FilePathW(),
        std::ios::in | std::ios::binary);
    hashSHA1 = ComputeContentSha1(*content, request_.ProgressControl());
    content->close();

    // cancel/stop compute sha1 by upper
    if (hashSHA1.empty()) {
        int32_t controlFlag = UploadPartProcessControlCallback((void *)this);
        if (controlFlag == ProgressControlStop) {
            completeOutcome = FileCompleteOutcome(PdsError("ClientError:100003", "Upload stop by upper."));
            return -1;
        }
        if (controlFlag == ProgressControlCancel) {
            completeOutcome =  FileCompleteOutcome(PdsError("ClientError:100004", "Upload cancel by upper."));
            return -1;
        }
    }

    return 0;
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
