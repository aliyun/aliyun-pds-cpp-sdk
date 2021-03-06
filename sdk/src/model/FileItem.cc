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


#include <alibabacloud/pds/model/FileItem.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

void FileItem::ParseFromJson(const Json::Value& jValue)
{
    category_ = jValue["category"].asString();
    contentHash_ = jValue["content_hash"].asString();
    contentHashName_ = jValue["content_hash_name"].asString();
    contentType_ = jValue["content_type"].asString();
    crc64Hash_ = jValue["crc64_hash"].asString();
    createdAt_ = jValue["created_at"].asString();
    description_ = jValue["description"].asString();
    downloadUrl_ = jValue["download_url"].asString();
    driveID_ = jValue["drive_id"].asString();
    encryptMode_ = jValue["encrypt_mode"].asString();
    fileExtension_ = jValue["file_extension"].asString();
    fileID_ = jValue["file_id"].asString();
    hidden_ = jValue["hidden"].asBool();
    name_ = jValue["name"].asString();
    parentFileID_ = jValue["parent_file_id"].asString();
    punishFlag_ = jValue["punish_flag"].asInt64();
    size_ = jValue["size"].asInt64();
    starred_ = jValue["starred"].asBool();
    status_ = jValue["status"].asString();
    thumbnail_ = jValue["thumbnail"].asString();
    trashed_ = jValue["trashed"].asBool();
    type_ = jValue["type"].asString();
    updatedAT_ = jValue["updated_at"].asString();
    url_ = jValue["url"].asString();
    uploadID_ = jValue["upload_id"].asString();

    if (jValue.isMember("user_tags")) {
        Json::Value tags = jValue["user_tags"];
        Json::Value::Members members;
        members = tags.getMemberNames();
        for (Json::Value::Members::iterator iter = members.begin(); iter != members.end(); iter++) {
            std::string key = *iter;
            if (tags[key.c_str()].isString()) {
                std::string value = tags[key.c_str()].asString();
                userTags_[key] = value;
            }
        }
    }
}
