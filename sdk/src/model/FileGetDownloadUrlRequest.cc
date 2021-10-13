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


#include <alibabacloud/pds/model/FileGetDownloadUrlRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

FileGetDownloadUrlRequest::FileGetDownloadUrlRequest(const std::string& driveID, const std::string& shareID,
    const std::string& fileID):
        driveID_(driveID),
        shareID_(shareID),
        fileID_(fileID),
        urlExpireSec_(0)
{
    setPath("/v2/file/get_download_url");
}

std::string FileGetDownloadUrlRequest::Path() const
{
    return path_;
}

std::shared_ptr<std::iostream> FileGetDownloadUrlRequest::Body() const
{
    Json::Value root;
    if (!driveID_.empty()) {
        root["drive_id"] = driveID_;
    }
    if (!shareID_.empty()) {
        root["share_id"] = shareID_;
    }
    root["file_id"] = fileID_;

    if (0 != urlExpireSec_) {
        root["url_expire_sec"] = urlExpireSec_;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

int FileGetDownloadUrlRequest::validate() const
{
    return 0;
}

void FileGetDownloadUrlRequest::setUrlExpireSec(int64_t urlExpireSec)
{
    urlExpireSec_ = urlExpireSec;
}

void FileGetDownloadUrlRequest::setShareToken(const std::string& shareToken)
{
    shareToken_ = shareToken;
}

HeaderCollection FileGetDownloadUrlRequest::specialHeaders() const
{
    auto headers = PdsRequest::specialHeaders();
    if (!shareToken_.empty()) {
        headers["x-share-token"] = shareToken_;
    }
    return headers;
}