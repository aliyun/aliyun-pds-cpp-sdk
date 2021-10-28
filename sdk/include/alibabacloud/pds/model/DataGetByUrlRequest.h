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
#include <alibabacloud/pds/PdsRequest.h>
#include <alibabacloud/pds/Types.h>
#include <alibabacloud/pds/model/RequestMetaData.h>
#include <alibabacloud/pds/http/HttpType.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT DataGetByUrlRequest: public PdsRequest
    {
    public:
        DataGetByUrlRequest(const std::string& url);
        DataGetByUrlRequest(const std::string& url, const RequestMetaData& metaData);
        void setUrl(const std::string& url);
        void setRange(int64_t start, int64_t end);
        void setTrafficLimit(uint64_t value);
        void setUserAgent(const std::string& ua);
    protected:
        virtual HeaderCollection specialHeaders() const;
        virtual int validate() const;
    private:
        int64_t range_[2];
        bool rangeIsSet_;
        RequestMetaData metaData_;
        uint64_t trafficLimit_;
        std::string userAgent_;
    };
}
}
