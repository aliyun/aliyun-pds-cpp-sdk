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
#include <alibabacloud/pds/Export.h>
#include <alibabacloud/pds/Types.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT RequestMetaData
    {
    public:
        RequestMetaData() = default;
        RequestMetaData(const HeaderCollection& data);
        RequestMetaData& operator=(const HeaderCollection& data);

        int64_t ContentLength() const ;
        const std::string& ContentType() const;
        const std::string& ContentMd5() const;
        uint64_t CRC64() const;
        const std::string& ETag() const;
        const std::string& VersionId() const;

        void setContentLength(int64_t value);
        void setContentType(const std::string& value);
        void setContentMd5(const std::string& value);
        void setCrc64(uint64_t value);
        void setETag(const std::string& value);

        void addHeader(const std::string& key, const std::string& value);
        bool hasHeader(const std::string& key) const;
        void removeHeader(const std::string& key);
        MetaData& HttpMetaData();
        const MetaData& HttpMetaData() const;
        HeaderCollection toHeaderCollection() const;
    private:
        MetaData metaData_;
    };
}
}
