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

#pragma once
#include <alibabacloud/pds/Export.h>
#include <alibabacloud/pds/PdsRequest.h>
#include <alibabacloud/pds/Types.h>
#include <alibabacloud/pds/model/ObjectMetaData.h>
#include <alibabacloud/pds/http/HttpType.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT GetObjectByUrlRequest: public ServiceRequest
    {
    public:
        GetObjectByUrlRequest(const std::string& url);
        GetObjectByUrlRequest(const std::string& url, const ObjectMetaData& metaData);
        void setRange(int64_t start, int64_t end);
        void setProcess(const std::string& process);
        void setTrafficLimit(uint64_t value);
        void setUserAgent(const std::string& ua);
        std::pair<int64_t, int64_t> Range() const;
    protected:
        virtual HeaderCollection Headers() const;
        virtual ParameterCollection Parameters() const;
        virtual std::shared_ptr<std::iostream> Body() const;
    private:
        int64_t range_[2];
        bool rangeIsSet_;
        ObjectMetaData metaData_;
        std::string process_;
        uint64_t trafficLimit_;
        std::string userAgent_;
    };
}
}
