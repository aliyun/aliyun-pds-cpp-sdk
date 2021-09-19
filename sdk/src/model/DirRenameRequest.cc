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


#include <alibabacloud/pds/model/DirRenameRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

DirRenameRequest::DirRenameRequest(const std::string& driveID, const std::string& fileID, const std::string& name,
    const std::string& checkNameMode):
        driveID_(driveID),
        fileID_(fileID),
        name_(name),
        checkNameMode_(checkNameMode)
{
    setPath("/v2/file/update");
}

std::string DirRenameRequest::Path() const
{
    return path_;
}

std::shared_ptr<std::iostream> DirRenameRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["file_id"] = fileID_;
    root["name"] = name_;
    root["check_name_mode"] = checkNameMode_;

    auto content = std::make_shared<std::stringstream>();
    *content << root;
    return content;
}

int DirRenameRequest::validate() const
{
    return 0;
}
