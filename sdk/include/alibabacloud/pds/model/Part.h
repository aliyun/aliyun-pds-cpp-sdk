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
#include <alibabacloud/pds/Types.h>

namespace AlibabaCloud
{
namespace PDS
{
    class FileListUploadedPartsRequest;
    class ResumableUploader;
    class ALIBABACLOUD_PDS_EXPORT Part
    {
    public:
        Part() :partNumber_(0), paretSize_(0), cRC64_(0) {}
        Part(int32_t partNumber, int64_t partSize, const std::string& eTag):
            partNumber_(partNumber),
            paretSize_(partSize),
            eTag_(eTag){}
        int32_t PartNumber() const { return partNumber_; }
        int64_t PartSize() const { return paretSize_; }
        uint64_t CRC64() const { return cRC64_; }
        const std::string& ETag() const { return eTag_; }
    private:
        friend class FileListUploadedPartsRequest;
        friend class ResumableUploader;
        int32_t partNumber_;
        int64_t paretSize_;
        std::string eTag_;
        uint64_t cRC64_;
    };
    using PartList = std::vector<Part>;
}
}
