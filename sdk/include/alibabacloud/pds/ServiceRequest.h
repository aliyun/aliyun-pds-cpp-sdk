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

#include <memory>
#include <iostream>
#include <alibabacloud/pds/Export.h>
#include <alibabacloud/pds/Types.h>

namespace AlibabaCloud
{
namespace PDS
{
    const int REQUEST_FLAG_CONTENTMD5    = (1 << 0);
    const int REQUEST_FLAG_PARAM_IN_PATH = (1 << 1);
    const int REQUEST_FLAG_CHECK_CRC64   = (1 << 2);
    const int REQUEST_FLAG_SAVE_CLIENT_CRC64 = (1 << 3);
    const int REQUEST_FLAG_PDS_DATA_REQUEST = (1 << 4);

    class ALIBABACLOUD_PDS_EXPORT ServiceRequest
    {
    public:
        virtual ~ServiceRequest() = default;

        std::string Path() const;
        virtual HeaderCollection Headers() const = 0;
        virtual ParameterCollection Parameters() const = 0;
        virtual std::shared_ptr<std::iostream> Body() const = 0;

        int Flags() const;
        void setFlags(int flags);

        const IOStreamFactory& ResponseStreamFactory() const;
        void setResponseStreamFactory(const IOStreamFactory& factory);

        const AlibabaCloud::PDS::TransferProgress& TransferProgress() const;
        void setTransferProgress(const AlibabaCloud::PDS::TransferProgress& arg);

        const AlibabaCloud::PDS::ProgressControl& ProgressControl() const;
        void setProgressControl(const AlibabaCloud::PDS::ProgressControl& arg);

    protected:
        ServiceRequest();
        void setPath(const std::string &path);
    private:
        int flags_;
        std::string path_;
        IOStreamFactory responseStreamFactory_;
        AlibabaCloud::PDS::TransferProgress transferProgress_;
        AlibabaCloud::PDS::ProgressControl progressControl_;
    };
}
}
