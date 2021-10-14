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


#include <alibabacloud/pds/model/DirCreateRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

DirCreateRequest::DirCreateRequest(const std::string& driveID, const std::string& parentFileID, const std::string& name,
    const std::string& checkNameMode):
        driveID_(driveID),
        parentFileID_(parentFileID),
        name_(name),
        checkNameMode_(checkNameMode),
        type_("folder")
{
    setPath("/v2/file/create");
}

std::string DirCreateRequest::Path() const
{
    return path_;
}

std::shared_ptr<std::iostream> DirCreateRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["parent_file_id"] = parentFileID_;
    root["name"] = name_;
    root["check_name_mode"] = checkNameMode_;
    root["type"] = type_;

    int index = 0;
    for (const UserTag& userTag : userTags_) {
        root["user_tags"][index]["key"] = userTag.Key();
        root["user_tags"][index]["value"] = userTag.Value();
        index++;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

void DirCreateRequest::setUserTags(const AlibabaCloud::PDS::UserTagList& userTags)
{
    userTags_ = userTags;
}

int DirCreateRequest::validate() const
{
    return 0;
}
