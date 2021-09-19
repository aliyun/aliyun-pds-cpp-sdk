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


#include <alibabacloud/pds/model/FileCreateRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

FileCreateRequest::FileCreateRequest(const std::string& driveID, const std::string& parentFileID, const std::string& name,
    const std::string& fileID, const std::string& checkNameMode, const int64_t size):
        driveID_(driveID),
        parentFileID_(parentFileID),
        name_(name),
        fileID_(fileID),
        checkNameMode_(checkNameMode),
        size_(size),
        type_("file")
{
    setPath("/v2/file/create");
}

std::string FileCreateRequest::Path() const
{
    return path_;
}

std::shared_ptr<std::iostream> FileCreateRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["parent_file_id"] = parentFileID_;
    root["name"] = name_;
    root["check_name_mode"] = checkNameMode_;
    root["size"] = size_;
    root["type"] = type_;
    if (!fileID_.empty()) {
        root["file_id"] = fileID_;
    }

    auto content = std::make_shared<std::stringstream>();
    *content << root;
    return content;
}

int FileCreateRequest::validate() const
{
    if (checkNameMode_ == "auto_rename") {
        return -1;
    }
    return 0;
}
