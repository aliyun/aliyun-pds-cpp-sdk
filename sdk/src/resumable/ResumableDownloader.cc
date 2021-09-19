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
#include "../utils/Crc64.h"
#include "../utils/LogUtils.h"
#include "../utils/FileSystemUtils.h"
#include "../external/json/json.h"
#include "ResumableDownloader.h"
#include "../model/ModelError.h"

using namespace AlibabaCloud::PDS;

GetObjectOutcome ResumableDownloader::Download()
{
    GetObjectResult result;
    return GetObjectOutcome(result);
}

int ResumableDownloader::prepare(PdsError& err)
{
    UNUSED_PARAM(err);
    return 0;
}

int ResumableDownloader::validateRecord()
{
    return 0;
}

int ResumableDownloader::loadRecord()
{
    return 0;
}

void ResumableDownloader::genRecordPath()
{
}

int ResumableDownloader::getPartsToDownload(PdsError &err, PartRecordList &partsToDownload)
{
    UNUSED_PARAM(err);
    UNUSED_PARAM(partsToDownload);
    return 0;
}

void ResumableDownloader::initRecord()
{
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
        process.Handler(increment, downloader->consumedSize_, downloader->contentLength_, process.UserData);
    }
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

GetObjectOutcome ResumableDownloader::GetObjectByUrlWrap(const GetObjectByUrlRequest &request) const
{
    return client_->GetObjectByUrl(request);
}

