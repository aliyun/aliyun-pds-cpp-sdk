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
    class PartInfoResp
    {
    public:
        PartInfoResp() = default;
        PartInfoResp(int64_t partNumber, int64_t partSize, std::string uploadUrl) :
            partNumber_(partNumber),
            partSize_(partSize),
            uploadUrl_(uploadUrl){}
        int64_t PartNumber() const { return partNumber_; }
        int64_t PartSize() const { return partSize_; }
        std::string UploadUrl() const { return uploadUrl_; }
    public:
        int64_t partNumber_;
        int64_t partSize_;
        std::string uploadUrl_;
    };
    using PartInfoRespList = std::vector<PartInfoResp>;
    class ALIBABACLOUD_PDS_EXPORT FileGetUploadUrlResult :public PdsResult
    {
    public:
        FileGetUploadUrlResult();
        FileGetUploadUrlResult(const std::shared_ptr<std::iostream>& content);
        const std::string& DomainID() const { return domainID_; }
        const std::string& DriveID() const { return driveID_; }
        const std::string& FileID() const { return fileID_; }
        const std::string& UploadID() const { return uploadID_; }
        const AlibabaCloud::PDS::PartInfoRespList& PartInfoRespList() const { return partInfoRespList_; }
        void PrintString();
    private:
        std::string domainID_;
        std::string driveID_;
        std::string fileID_;
        std::string uploadID_;
        AlibabaCloud::PDS::PartInfoRespList partInfoRespList_;
    };
}
}