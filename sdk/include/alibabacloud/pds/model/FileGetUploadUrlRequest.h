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
#include <alibabacloud/pds/model/PartInfoReq.h>

namespace AlibabaCloud
{
namespace PDS
{
    using PartInfoReqList = std::vector<PartInfoReq>;
    class ALIBABACLOUD_PDS_EXPORT FileGetUploadUrlRequest: public PdsRequest
    {
    public:
        FileGetUploadUrlRequest(const std::string& driveID, const std::string& fileID, const std::string& uploadID,
            const AlibabaCloud::PDS::PartInfoReqList& partInfoReqList);
        std::string Path() const;
        virtual std::shared_ptr<std::iostream> Body() const;
        void setTrafficLimit(uint64_t value);
        void setUserAgent(const std::string& ua);
        std::pair<int64_t, int64_t> Range() const;
    protected:
        int validate() const;
    private:
        std::string driveID_;
        std::string fileID_;
        std::string uploadID_;
        AlibabaCloud::PDS::PartInfoReqList partInfoReqList_;
        std::string path_;
        uint64_t trafficLimit_;
        std::string userAgent_;
    };
}
}
