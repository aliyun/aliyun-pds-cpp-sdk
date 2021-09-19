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
#include <alibabacloud/pds/model/PutObjectByUrlRequest.h>
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
    ResumableBaseWorker(request.ObjectSize(), request.PartSize()),
    request_(request),
    client_(client)
{
    if (!request.FilePath().empty()) {
        time_t lastMtime;
        std::streamsize fileSize;
        if (GetPathInfo(request.FilePath(), lastMtime, fileSize)) {
            objectSize_ = static_cast<uint64_t>(fileSize);
        }
    }
#ifdef _WIN32
    else if (!request.FilePathW().empty()) {
        time_t lastMtime;
        std::streamsize fileSize;
        if (GetPathInfo(request.FilePathW(), lastMtime, fileSize)) {
            objectSize_ = static_cast<uint64_t>(fileSize);
        }
    }
#endif
}

FileCompleteOutcome ResumableUploader::Upload()
{
    FileCompleteOutcome result;
    return result;
}

int ResumableUploader::validateRecord()
{
    return 0;
}

int ResumableUploader::loadRecord()
{
    return 0;
}

void ResumableUploader::genRecordPath()
{
}

int ResumableUploader::getPartsToUpload(PdsError &err, PartList &partsUploaded, PartList &partsToUpload)
{
    UNUSED_PARAM(err);
    UNUSED_PARAM(partsUploaded);
    UNUSED_PARAM(partsToUpload);
    return 0;
}

void ResumableUploader::initRecordInfo()
{
}

void ResumableUploader::buildRecordInfo(const AlibabaCloud::PDS::Json::Value& root)
{
    UNUSED_PARAM(root);
}

void ResumableUploader::dumpRecordInfo(AlibabaCloud::PDS::Json::Value& root)
{
    UNUSED_PARAM(root);
}

void ResumableUploader::UploadPartProcessCallback(size_t increment, int64_t transfered, int64_t total, void *userData)
{
    UNUSED_PARAM(transfered);
    UNUSED_PARAM(total);

    auto uploader = (ResumableUploader*)userData;
    std::lock_guard<std::mutex> lck(uploader->lock_);
    uploader->consumedSize_ += increment;

    auto process = uploader->request_.TransferProgress();
    if (process.Handler) {
        process.Handler(increment, uploader->consumedSize_, uploader->objectSize_, process.UserData);
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

PutObjectOutcome ResumableUploader::UploadPartWrap(const PutObjectByUrlRequest &request) const
{
    return client_->PutObjectByUrl(request);
}

FileListUploadedPartsOutcome ResumableUploader::ListUploadedPartsWrap(const FileListUploadedPartsRequest &request) const
{
    return client_->FileListUploadedParts(request);
}

FileCompleteOutcome ResumableUploader::FileCompleteWrap(const FileCompleteRequest &request) const
{
    return client_->FileComplete(request);
}


