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

namespace AlibabaCloud
{
namespace PDS
{
    class ALIBABACLOUD_PDS_EXPORT DirListRequest: public PdsRequest
    {
    public:
        DirListRequest(const std::string& driveID, const std::string& parentFileID,
            const std::string& orderBy, const std::string& orderDirection,
            const std::string& fields, const std::string& marker, int64_t limit);
        DirListRequest(const std::string& driveID, const std::string& shareID, const std::string& parentFileID,
            const std::string& orderBy, const std::string& orderDirection,
            const std::string& fields, const std::string& marker, int64_t limit);
        virtual std::shared_ptr<std::iostream> Body() const;

        void setType(const std::string& type);
        void setCategory(const std::string& category);
        void setUrlExpireSec(int64_t urlExpireSec);
        void setStarred(bool* starred);
        void setAll(bool* all);
        void setShareToken(const std::string& shareToken);

    protected:
        virtual HeaderCollection specialHeaders() const;
        int validate() const;
    private:
        std::string driveID_;
        std::string shareID_;
        std::string parentFileID_;
        std::string orderBy_;
        std::string orderDirection_;
        std::string marker_;
        int64_t limit_;
        std::string fields_;

        std::string type_;
        std::string category_;
        int64_t urlExpireSec_;
        bool* starred_;
        bool* all_;
        std::string shareToken_;
    };
}
}
