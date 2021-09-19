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


#include <alibabacloud/pds/model/FileCreateResult.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

FileCreateResult::FileCreateResult():
        PdsResult()
{
}

FileCreateResult::FileCreateResult(
        const std::shared_ptr<std::iostream>& content)
{
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string errMsg;
    if (!Json::parseFromStream(rbuilder, *content, &root, &errMsg))
    {
        return;
    }

    domainID_ = root["domain_id"].asString();
    driveID_ = root["drive_id"].asString();
    fileID_ = root["file_id"].asString();
    fileName_ = root["file_name"].asString();
    parentFileID_ = root["parent_file_id"].asString();
    type_ = root["type"].asString();
    rapidUpload_ = root["rapid_upload"].asBool();
    uploadID_ = root["upload_id"].asString();
}

void FileCreateResult::PrintString()
{
    std::cout << "domain_id: " << domainID_ <<
        ", drive_id:" << driveID_ <<
        ", file_id: " << fileID_ <<
        ", file_name: " << fileName_ <<
        ", parent_file_id: " << parentFileID_ <<
        ", type: " << type_ <<
        ", rapid_upload: " << rapidUpload_ <<
        ", upload_id: " << uploadID_ <<std::endl;
}
