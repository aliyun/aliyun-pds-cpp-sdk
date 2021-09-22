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


#include <alibabacloud/pds/model/FileGetRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

FileGetRequest::FileGetRequest(const std::string& driveID, const std::string& fileID):
        driveID_(driveID),
        fileID_(fileID),
        urlExpireSec_(0)
{
    setPath("/v2/file/get");
}

std::string FileGetRequest::Path() const
{
    return path_;
}

std::shared_ptr<std::iostream> FileGetRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["file_id"] = fileID_;

    if (0 != urlExpireSec_) {
        root["url_expire_sec"] = urlExpireSec_;
    }

    auto content = std::make_shared<std::stringstream>();
    *content << root;
    return content;
}

int FileGetRequest::validate() const
{
    return 0;
}

void FileGetRequest::setUrlExpireSec(int64_t urlExpireSec)
{
    urlExpireSec_ = urlExpireSec;
}
