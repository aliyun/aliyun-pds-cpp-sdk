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

#include <fstream>
#include <algorithm>
#include <set>
#include <alibabacloud/pds/Const.h>
#include "../utils/Utils.h"
#include "../utils/LogUtils.h"
#include "../utils/Crc64.h"
#include "../utils/FileSystemUtils.h"
#include "../external/json/json.h"
#include "ResumableDownloader.h"
#include "../model/ModelError.h"

using namespace AlibabaCloud::PDS;

DataGetOutcome ResumableDownloader::Download()
{
    PdsError err;

    if (0 != validate(err)) {
        return DataGetOutcome(err);
    }

    PartRecordList partsToDownload;
    if (getPartsToDownload(err, partsToDownload) != 0) {
        return DataGetOutcome(err);
    }

    if (url_.empty()) {
        FileGetRequest getRequest(record_.driveID, record_.shareID, record_.fileID);
        getRequest.setShareToken(request_.ShareToken());

        auto fileGetOutcome = FileGetWrap(getRequest);
        if (!fileGetOutcome.isSuccess()) {
            return DataGetOutcome(fileGetOutcome.error());
        }
        url_ = fileGetOutcome.result().Url();
    }

    //task queue
    PartRecordList downloadedParts;
    if (hasRecord_) {
        downloadedParts = record_.parts;
    }
    std::vector<DataGetOutcome> outcomes;
    std::vector<std::thread> threadPool;

    for (uint32_t i = 0; i < request_.ThreadNum(); i++) {
        threadPool.emplace_back(std::thread([&]() {
            PartRecord part;
            while (true) {
                {
                    std::lock_guard<std::mutex> lck(lock_);
                    if (partsToDownload.empty())
                        break;
                    part = partsToDownload.front();
                    partsToDownload.erase(partsToDownload.begin());
                }

                if (!client_->isEnableRequest())
                    break;

                // check resumable progress control
                if (DownloadPartProcessControlCallback((void *)this) != 0)
                    break;

                uint64_t pos = partSize_ * (part.partNumber - 1);
                uint64_t start = part.offset;
                uint64_t end = start + part.size - 1;

                auto getDataReq = DataGetByUrlRequest(url_);
                getDataReq.setResponseStreamFactory([=]() {
                    auto tmpFstream = GetFstreamByPath(request_.TempFilePath(), request_.TempFilePathW(),
                        std::ios_base::in | std::ios_base::out | std::ios_base::binary);
                    tmpFstream->seekp(pos, tmpFstream->beg);
                    return tmpFstream;
                });
                getDataReq.setRange(start, end);
                getDataReq.setFlags(getDataReq.Flags() | REQUEST_FLAG_CHECK_CRC64 | REQUEST_FLAG_SAVE_CLIENT_CRC64);

                auto process = request_.TransferProgress();
                if (process.Handler) {
                    TransferProgress downloadPartProcess = { DownloadPartProcessCallback, (void *)this };
                    getDataReq.setTransferProgress(downloadPartProcess);
                }
                auto progressControl = request_.ProgressControl();
                if (progressControl.Handler) {
                    ProgressControl downloadPartProgressControl = { DownloadPartProcessControlCallback, (void *)this };
                    getDataReq.setProgressControl(downloadPartProgressControl);
                }
                if (request_.TrafficLimit() != 0) {
                    getDataReq.setTrafficLimit(request_.TrafficLimit());
                }
                auto outcome = DataGetByUrlWrap(getDataReq);

                // lock
                bool needRetry = false;
                {
                    std::lock_guard<std::mutex> lck(lock_);
                    if (!outcome.isSuccess() && outcome.error().Code() == "AccessDenied" && outcome.error().Message().find("expired")) {
                        FileGetRequest getRequest(record_.driveID, record_.shareID ,record_.fileID);
                        getRequest.setShareToken(request_.ShareToken());

                        auto fileGetOutcome = FileGetWrap(getRequest);
                        if (!fileGetOutcome.isSuccess()) {
                            outcomes.push_back(fileGetOutcome.error());
                            break;
                        }
                        // check file content-hash
                        auto contentHash = fileGetOutcome.result().ContentHash();
                        if (contentHash != contentHash_ ) {
                            outcomes.push_back(PdsError("SourceFileModified","Source file has been modified since last download."));
                            break;
                        }
                        url_ = fileGetOutcome.result().Url();
                        needRetry = true;
                    }
                }
                if (needRetry){
                    getDataReq.setUrl(url_);
                    getDataReq.setResponseStreamFactory([=]() {
                        auto tmpFstream = GetFstreamByPath(request_.TempFilePath(), request_.TempFilePathW(),
                            std::ios_base::in | std::ios_base::out | std::ios_base::binary);
                        tmpFstream->seekp(pos, tmpFstream->beg);
                        return tmpFstream;
                    });
                    outcome = DataGetByUrlWrap(getDataReq);
                }
#ifdef ENABLE_PDS_TEST
                if (!!(request_.Flags() & 0x40000000) && part.partNumber == 2) {
                    const char* TAG = "ResumableDownloadClient";
                    PDS_LOG(LogLevel::LogDebug, TAG, "NO.2 part data download failed.");
                    outcome = DataGetOutcome();
                }
#endif // ENABLE_PDS_TEST

                // lock
                {
                    std::lock_guard<std::mutex> lck(lock_);
                    if (outcome.isSuccess()) {
                        part.crc64 = std::strtoull(outcome.result().Metadata().HttpMetaData().at("x-oss-hash-crc64ecma-by-client").c_str(), nullptr, 10);
                        downloadedParts.push_back(part);
                    }
                    outcomes.push_back(outcome);

                    //update record
                    if (hasRecordPath() && outcome.isSuccess()) {
                        auto &record = record_;
                        record.parts = downloadedParts;

                        Json::Value root;
                        root["opType"] = record.opType;
                        root["driveID"] = record.driveID;
                        root["shareID"] = record.shareID;
                        root["fileID"] = record.fileID;
                        root["contentHash"] = record.contentHash;
                        root["filePath"] = record.filePath;
                        root["mtime"] = record.mtime;
                        root["size"] = record.size;
                        root["partSize"] = record.partSize;

                        int index = 0;
                        for (PartRecord& partR : record.parts) {
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
        }));
    }

    for (auto& worker : threadPool) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    if (!client_->isEnableRequest()) {
        return DataGetOutcome(PdsError("ClientError:100002", "Disable all requests by upper."));
    }

    int32_t controlFlag = DownloadPartProcessControlCallback((void *)this);
    if (controlFlag == ProgressControlStop) {
        return DataGetOutcome(PdsError("ClientError:100003", "Download stop by upper."));
    }
    if (controlFlag == ProgressControlCancel) {
        removeRecordFile();
        removeTempFile();
        return DataGetOutcome(PdsError("ClientError:100004", "Download cancel by upper."));
    }

    std::shared_ptr<std::iostream> content = nullptr;
    for (auto& outcome : outcomes) {
        if (!outcome.isSuccess()) {
            return DataGetOutcome(outcome.error());
        }
        outcome.result().setContent(content);
    }

    if (downloadedParts.size() < outcomes.size()) {
        return DataGetOutcome(PdsError("DownloadNotComplete", "Not all parts are downloaded."));
    }

    std::sort(downloadedParts.begin(), downloadedParts.end(), [&](const PartRecord& a, const PartRecord& b)
    {
        return a.partNumber < b.partNumber;
    });

    // check size
    uint64_t localFileSize = GetFileSize(request_.TempFilePath(), request_.TempFilePathW());
    if (fileSize_ != localFileSize) {
        return DataGetOutcome(PdsError("FileSizeCheckError", "Resumable Download data check size fail."));
    }

    //check crc64
    if (client_->configuration().enableCrc64 && !crc64Hash_.empty()) {
        uint64_t localCRC64 = downloadedParts[0].crc64;
        for (size_t i = 1; i < downloadedParts.size(); i++) {
            localCRC64 = CRC64::CombineCRC(localCRC64, downloadedParts[i].crc64, downloadedParts[i].size);
        }
        uint64_t serverCRC64 = std::strtoull(crc64Hash_.c_str(), nullptr, 10);
        if (localCRC64 != serverCRC64) {
            return DataGetOutcome(PdsError("CrcCheckError", "Resumable Download data CRC checksum fail."));
        }
    }

    if (!renameTempFile()) {
        std::stringstream ss;
        ss << "rename temp file failed";
        return DataGetOutcome(PdsError("RenameError", ss.str()));
    }

    removeRecordFile();

    DataGetResult result;
    return DataGetOutcome(result);
}

int ResumableDownloader::prepare(PdsError& err)
{
    UNUSED_PARAM(err);

    determinePartSize();
    if (hasRecordPath()) {
        initRecord();

        Json::Value root;
        root["opType"] = record_.opType;
        root["driveID"] = record_.driveID;
        root["shareID"] = record_.shareID;
        root["fileID"] = record_.fileID;
        root["contentHash"] = record_.contentHash;
        root["filePath"] = record_.filePath;
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

int ResumableDownloader::validateRecord()
{
    auto record = record_;

    if (record.size != fileSize_ || record.mtime != request_.FileMtime() ||
        record.contentHash != contentHash_) {
        return ARG_ERROR_DOWNLOAD_SOURCE_FILE_MODIFIED;
    }

    Json::Value root;
    root["opType"] = record.opType;
    root["driveID"] = record.driveID;
    root["shareID"] = record.shareID;
    root["fileID"] = record.fileID;
    root["contentHash"] = record.contentHash;
    root["filePath"] = record.filePath;
    root["mtime"] = record.mtime;
    root["size"] = record.size;
    root["partSize"] = record.partSize;
    root["parts"].resize(0);
    int index = 0;
    for (PartRecord& part : record.parts) {
        root["parts"][index]["partNumber"] = part.partNumber;
        root["parts"][index]["size"] = part.size;
        root["parts"][index]["crc64"] = part.crc64;
        index++;
    }

    std::stringstream recordStream;
    recordStream << root;

    std::string md5Sum = ComputeContentETag(recordStream);
    if (md5Sum != record.md5Sum) {
        return -1;
    }
    return 0;
}

int ResumableDownloader::loadRecord()
{
    auto recordStream = GetFstreamByPath(recordPath_, recordPathW_, std::ios::in);
    if (recordStream->is_open()) {
        Json::Value root;
        Json::CharReaderBuilder rbuilder;
        std::string errMsg;
        if (!Json::parseFromStream(rbuilder, *recordStream, &root, &errMsg))
        {
            return ARG_ERROR_PARSE_DOWNLOAD_RECORD_FILE;
        }

        record_.opType = root["opType"].asString();
        record_.driveID = root["driveID"].asString();
        record_.shareID = root["shareID"].asString();
        record_.fileID = root["fileID"].asString();
        record_.contentHash = root["contentHash"].asString();
        record_.filePath = root["filePath"].asString();
        record_.mtime = root["mtime"].asString();
        record_.size = root["size"].asUInt64();
        record_.partSize = root["partSize"].asUInt64();

        PartRecord part;
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

void ResumableDownloader::genRecordPath()
{
    recordPath_ = "";
    recordPathW_ = L"";

    if (!request_.hasCheckpointDir())
        return;

    std::stringstream ss;
    ss << "pds://" << request_.DriveID() << "/" << request_.ShareID() << "/" << request_.FileID() << "/" << contentHash_;
    if (!request_.VersionId().empty()) {
        ss << "?versionId=" << request_.VersionId();
    }

    auto srcPath = ss.str();
    auto destPath = !request_.FilePathW().empty() ? toString(request_.FilePathW()) : request_.FilePath();
    auto safeFileName = ComputeContentETag(srcPath) + "--" + ComputeContentETag(destPath);

    if (!request_.CheckpointDirW().empty()) {
        recordPathW_ = request_.CheckpointDirW() + WPATH_DELIMITER + toWString(safeFileName);;
    }
    else {
        recordPath_ = request_.CheckpointDir() + PATH_DELIMITER + safeFileName;
    }
}

int ResumableDownloader::getPartsToDownload(PdsError &err, PartRecordList &partsToDownload)
{
    UNUSED_PARAM(err);

    std::set<uint64_t> partNumbersDownloaded;
    if (hasRecord_) {
        for (PartRecord &part : record_.parts) {
            partNumbersDownloaded.insert(part.partNumber);
            consumedSize_ += part.size;
        }
    }

    int64_t start = 0;
    int64_t end = fileSize_ - 1;

    int32_t index = 1;
    for (int64_t offset = start; offset < end + 1; offset += partSize_, index++) {
        PartRecord part;
        part.partNumber = index;
        part.offset = offset;
        if (offset + (int64_t)partSize_ > end) {
            part.size = end - offset + 1;
        }
        else {
            part.size = partSize_;
        }

        auto iterNum = partNumbersDownloaded.find(part.partNumber);
        if (iterNum == partNumbersDownloaded.end()) {
            partsToDownload.push_back(part);
        }
    }
    return 0;
}

void ResumableDownloader::initRecord()
{
    auto filePath = request_.FilePath();
    if (!request_.FilePathW().empty()) {
        filePath = toString(request_.FilePathW());
    }

    record_.opType = "ResumableDownload";
    record_.driveID = request_.DriveID();
    record_.shareID = request_.ShareID();
    record_.fileID = request_.FileID();
    record_.contentHash = contentHash_;
    record_.filePath = filePath;
    record_.mtime = request_.FileMtime();
    record_.size = fileSize_;
    record_.partSize = partSize_;
}

void ResumableDownloader::DownloadPartProcessCallback(size_t increment, int64_t transfered, int64_t total, void *userData)
{
    UNUSED_PARAM(transfered);
    UNUSED_PARAM(total);

    auto downloader = (ResumableDownloader*)userData;
    std::lock_guard<std::mutex> lck(downloader->lock_);
    downloader->consumedSize_ += increment;

    auto process = downloader->request_.TransferProgress();
    if (process.Handler) {
        process.Handler(increment, downloader->consumedSize_, downloader->fileSize_, process.UserData);
    }
}

int32_t ResumableDownloader::DownloadPartProcessControlCallback(void *userData)
{
    auto downloader = (ResumableDownloader*)userData;
    std::lock_guard<std::mutex> lck(downloader->lock_);

    auto controller = downloader->request_.ProgressControl();
    if (controller.Handler) {
        return controller.Handler(controller.UserData);
    }
    return 0;
}

bool ResumableDownloader::renameTempFile()
{
#ifdef _WIN32
    if (!request_.TempFilePathW().empty()) {
        return RenameFile(request_.TempFilePathW(), request_.FilePathW());
    }
    else
#endif
    {
        return RenameFile(request_.TempFilePath(), request_.FilePath());
    }
}

void ResumableDownloader::removeTempFile()
{
#ifdef _WIN32
    if (!request_.TempFilePathW().empty()) {
        RemoveFile(request_.TempFilePathW());
    }
    else
#endif
    {
        RemoveFile(request_.TempFilePath());
    }
}

FileGetOutcome ResumableDownloader::FileGetWrap(const FileGetRequest &request) const
{
    return client_->FileGet(request);
}

DataGetOutcome ResumableDownloader::DataGetByUrlWrap(const DataGetByUrlRequest &request) const
{
    return client_->DataGetByUrl(request);
}

