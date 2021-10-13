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
#include <alibabacloud/pds/PdsResult.h>
#include <alibabacloud/pds/Types.h>

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT FileGetDownloadUrlResult :public PdsResult
    {
    public:
        FileGetDownloadUrlResult();
        FileGetDownloadUrlResult(const std::shared_ptr<std::iostream>& content);
        const std::string& ContentHash() const { return contentHash_; }
        const std::string& ContentHashName() const { return contentHashName_; }
        const std::string& Crc64Hash() const { return crc64Hash_; }
        const std::string& CdnUrl() const { return cdnUrl_; }
        const std::string& Expiration() const { return expiration_; }
        const std::string& InternalUrl() const { return internalUrl_; }
        const std::string& Method() const { return method_; }
        int64_t Size() const { return size_; }
        const std::string& Url() const { return url_; }
    private:
        std::string contentHash_;
        std::string contentHashName_;
        std::string crc64Hash_;
        std::string cdnUrl_;
        std::string expiration_;
        std::string internalUrl_;
        std::string method_;
        int64_t size_;
        std::string url_;
    };
}
}