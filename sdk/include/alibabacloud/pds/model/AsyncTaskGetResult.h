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

#pragma once
#include <alibabacloud/pds/Export.h>
#include <alibabacloud/pds/PdsResult.h>
#include <alibabacloud/pds/Types.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT AsyncTaskGetResult :public PdsResult
    {
    public:
        AsyncTaskGetResult();
        AsyncTaskGetResult(const std::shared_ptr<std::iostream>& content);
        const std::string& AsyncTaskID() const { return asyncTaskID_; }
        const std::string& State() const { return state_; }
        const std::string& Message() const { return message_; }
        int64_t ConsumedProcess() const { return consumedProcess_; }
        int64_t TotalProcess() const { return totalProcess_; }
        int64_t ErrCode() const { return errCode_; }
        const std::string& Url() const { return url_; }
    private:
        std::string asyncTaskID_;
        std::string state_;
        std::string message_;
        int64_t consumedProcess_;
        int64_t totalProcess_;
        int64_t errCode_;
        std::string url_;
    };
}
}