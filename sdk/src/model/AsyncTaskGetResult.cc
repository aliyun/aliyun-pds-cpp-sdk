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


#include <alibabacloud/pds/model/AsyncTaskGetResult.h>
#include <alibabacloud/pds/http/HttpType.h>
#include "../utils/Utils.h"
#include "../external/json/json.h"

using namespace AlibabaCloud::PDS;

AsyncTaskGetResult::AsyncTaskGetResult():
        PdsResult()
{
}

AsyncTaskGetResult::AsyncTaskGetResult(
        const std::shared_ptr<std::iostream>& content)
{
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string errMsg;
    if (!Json::parseFromStream(rbuilder, *content, &root, &errMsg))
    {
        return;
    }

    asyncTaskID_ = root["async_task_id"].asString();
    state_ = root["state"].asString();
    message_ = root["message"].asString();
    consumedProcess_ = root["consumed_process"].asInt64();
    totalProcess_ = root["total_process"].asInt64();
    errCode_ = root["err_code"].asInt64();
    url_ = root["url"].asString();
}

void AsyncTaskGetResult::PrintString()
{
    std::cout << "async_task_id: " << asyncTaskID_ <<
        ", state:" << state_ <<
        ", message:" << message_ <<
        ", consumed_process: " << consumedProcess_ <<
        ", total_process: " << totalProcess_ <<
        ", err_code: " << errCode_ <<
        ", url: " << url_ << std::endl;
}
