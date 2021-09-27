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
#include <string>
#include <alibabacloud/pds/PdsResult.h>
#include <alibabacloud/pds/model/RequestMetaData.h>
#include <alibabacloud/pds/ServiceRequest.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT DataGetResult :public PdsDataResult
    {
    public:
        DataGetResult();
        DataGetResult(const std::shared_ptr<std::iostream>& content,
            const HeaderCollection& headers);
        DataGetResult(const RequestMetaData& metaData);
        const RequestMetaData& Metadata()  const { return metaData_; }
        const std::shared_ptr<std::iostream>& Content() const { return content_; }
        void setContent(const std::shared_ptr<std::iostream>& content) { content_ = content; }
        void setMetaData(const RequestMetaData& meta) { metaData_ = meta; }
    private:
        RequestMetaData metaData_;
        std::shared_ptr<std::iostream> content_;
    };
}
}
