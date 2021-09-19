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


#include <alibabacloud/pds/model/DirDeleteResult.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

DirDeleteResult::DirDeleteResult():
        PdsResult()
{
}

DirDeleteResult::DirDeleteResult(
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
    asyncTaskID_ = root["async_task_id"].asString();
}

void DirDeleteResult::PrintString()
{
    std::cout << "domain_id: " << domainID_ <<
        ", drive_id:" << driveID_ <<
        ", file_id: " << fileID_ <<
        ", async_task_id: " << asyncTaskID_ << std::endl;
}
