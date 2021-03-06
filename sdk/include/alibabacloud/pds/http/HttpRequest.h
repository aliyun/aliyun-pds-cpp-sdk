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

#include <string>
#include <alibabacloud/pds/Types.h>
#include <alibabacloud/pds/ServiceRequest.h>
#include <alibabacloud/pds/http/HttpMessage.h>
#include <alibabacloud/pds/http/Url.h>

namespace AlibabaCloud
{
namespace PDS
{

    class ALIBABACLOUD_PDS_EXPORT HttpRequest : public HttpMessage
    {
        public:
            HttpRequest(Http::Method method = Http::Method::Get);
            ~HttpRequest();

            Http::Method method() const;
            Url url() const;
            void setMethod(Http::Method method);
            void setUrl(const Url &url);

            const IOStreamFactory& ResponseStreamFactory() const { return responseStreamFactory_; }
            void setResponseStreamFactory(const IOStreamFactory& factory) { responseStreamFactory_ = factory; }

            const AlibabaCloud::PDS::TransferProgress & TransferProgress() const {  return transferProgress_; }
            void setTransferProgress(const AlibabaCloud::PDS::TransferProgress &arg) { transferProgress_ = arg;}

            const AlibabaCloud::PDS::ProgressControl & ProgressControl() const {  return progressControl_; }
            void setProgressControl(const AlibabaCloud::PDS::ProgressControl &arg) { progressControl_ = arg;}

            void setCheckCrc64(bool enable) { hasCheckCrc64_ = enable; }
            bool hasCheckCrc64() const { return hasCheckCrc64_; }
            void setCrc64Result(uint64_t crc) { crc64Result_ = crc; }
            uint64_t Crc64Result() const { return crc64Result_; }

            void setTransferedBytes(int64_t value) { transferedBytes_ = value; }
            uint64_t TransferedBytes() const { return transferedBytes_;}

        private:
            Http::Method method_;
            Url url_;
            IOStreamFactory responseStreamFactory_;
            AlibabaCloud::PDS::TransferProgress transferProgress_;
            bool hasCheckCrc64_;
            uint64_t crc64Result_;
            int64_t transferedBytes_;
            AlibabaCloud::PDS::ProgressControl progressControl_;
    };
}
}
