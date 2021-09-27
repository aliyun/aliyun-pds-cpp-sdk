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


#include <alibabacloud/pds/model/MetaUserTagsDeleteRequest.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"
#include "ModelError.h"
#include <sstream>
using namespace AlibabaCloud::PDS;

MetaUserTagsDeleteRequest::MetaUserTagsDeleteRequest(const std::string& driveID, const std::string& fileID,
    const std::vector<std::string>& keyList):
        driveID_(driveID),
        fileID_(fileID),
        keyList_(keyList)
{
    setPath("/v2/file/delete_usertags");
}

std::string MetaUserTagsDeleteRequest::Path() const
{
    return path_;
}

std::shared_ptr<std::iostream> MetaUserTagsDeleteRequest::Body() const
{
    Json::Value root;
    root["drive_id"] = driveID_;
    root["file_id"] = fileID_;

    int index = 0;
    for (const std::string& key : keyList_) {
        root["key_list"][index] = key;
        index++;
    }

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::shared_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    auto content = std::make_shared<std::stringstream>();
    writer->write(root, content.get());
    return content;
}

int MetaUserTagsDeleteRequest::validate() const
{
    return 0;
}
