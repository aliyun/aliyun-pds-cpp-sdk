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


#include <alibabacloud/pds/model/FileCompleteResult.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

FileCompleteResult::FileCompleteResult():
        PdsResult()
{
}

FileCompleteResult::FileCompleteResult(
        const std::shared_ptr<std::iostream>& content)
{
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string errMsg;
    if (!Json::parseFromStream(rbuilder, *content, &root, &errMsg))
    {
        return;
    }

    category_ = root["category"].asString();
    contentHash_ = root["content_hash"].asString();
    contentHashName_ = root["content_hash_name"].asString();
    contentType_ = root["content_type"].asString();
    crc64Hash_ = root["crc64_hash"].asString();
    createdAt_ = root["created_at"].asString();
    domainID_ = root["domain_id"].asString();
    driveID_ = root["drive_id"].asString();
    encryptMode_ = root["encrypt_mode"].asString();
    fileExtension_ = root["file_extension"].asString();
    fileID_ = root["file_id"].asString();
    hidden_ = root["hidden"].asBool();
    name_ = root["name"].asString();
    parentFileID_ = root["parent_file_id"].asString();
    size_ = root["size"].asInt64();
    starred_ = root["starred"].asBool();
    status_ = root["status"].asString();
    type_ = root["type"].asString();
    updatedAT_ = root["updated_at"].asString();
    uploadID_ = root["upload_id"].asString();
}

void FileCompleteResult::PrintString()
{
    std::cout << "domain_id: " << domainID_ <<
        ", drive_id:" << driveID_ <<
        ", file_id: " << fileID_ <<
        ", file_name: " << name_ <<
        ", parent_file_id: " << parentFileID_ <<
        ", type: " << type_ <<
        ", upload_id: " << uploadID_ <<std::endl;
}
