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


#include <alibabacloud/pds/model/FileGetVideoPreviewPlayInfoRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

FileGetVideoPreviewPlayInfoRequest::FileGetVideoPreviewPlayInfoRequest(const std::string& driveID, const std::string& shareID,
    const std::string& fileID, const std::string& category):
        driveID_(driveID),
        shareID_(shareID),
        fileID_(fileID),
        category_(category),
        urlExpireSec_(0)
{
    setPath("/v2/file/get_video_preview_play_info");
}

std::shared_ptr<std::iostream> FileGetVideoPreviewPlayInfoRequest::Body() const
{
    Json::Value root;
    if (!driveID_.empty()) {
        root["drive_id"] = driveID_;
    }
    if (!shareID_.empty()) {
        root["share_id"] = shareID_;
    }
    root["file_id"] = fileID_;
    root["category"] = category_;

    if (0 != urlExpireSec_) {
        root["url_expire_sec"] = urlExpireSec_;
    }
    if (!templateID_.empty()) {
        root["template_id"] = templateID_;
    }
    if (nullptr != getWithoutUrl_) {
        root["get_without_url"] = *getWithoutUrl_;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

int FileGetVideoPreviewPlayInfoRequest::validate() const
{
    return 0;
}

void FileGetVideoPreviewPlayInfoRequest::setTemplateID(const std::string& templateID)
{
    templateID_ = templateID;
}

void FileGetVideoPreviewPlayInfoRequest::setGetWithoutUrl(bool* getWithoutUrl)
{
    getWithoutUrl_ = getWithoutUrl;
}

void FileGetVideoPreviewPlayInfoRequest::setUrlExpireSec(int64_t urlExpireSec)
{
    urlExpireSec_ = urlExpireSec;
}

void FileGetVideoPreviewPlayInfoRequest::setShareToken(const std::string& shareToken)
{
    shareToken_ = shareToken;
}

HeaderCollection FileGetVideoPreviewPlayInfoRequest::specialHeaders() const
{
    auto headers = PdsRequest::specialHeaders();
    if (!shareToken_.empty()) {
        headers["x-share-token"] = shareToken_;
    }
    return headers;
}