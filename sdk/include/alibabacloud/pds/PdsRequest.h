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
#include <alibabacloud/pds/ServiceRequest.h>

namespace AlibabaCloud
{
namespace PDS
{
    class PdsClientImpl;
    class  ALIBABACLOUD_PDS_EXPORT PdsRequest : public ServiceRequest
    {
    public:
        PdsRequest();
        virtual ~ PdsRequest() = default;
        virtual HeaderCollection Headers() const;
        virtual ParameterCollection Parameters() const;
        virtual std::shared_ptr<std::iostream> Body() const;
    protected:
        friend class PdsClientImpl;

        virtual int validate() const;
        const char *validateMessage(int code) const;

        virtual std::string payload() const;
        virtual HeaderCollection specialHeaders() const;
        virtual ParameterCollection specialParameters() const;
    };

    class ALIBABACLOUD_PDS_EXPORT PdsObjectRequest : public PdsRequest
    {
    public:
        PdsObjectRequest() :
            PdsRequest(),
            requestPayer_(AlibabaCloud::PDS::RequestPayer::NotSet),
            versionId_()
        {}
        void setRequestPayer(AlibabaCloud::PDS::RequestPayer value);
        AlibabaCloud::PDS::RequestPayer RequestPayer() const;
        void setVersionId(const std::string& versionId);
        const std::string& VersionId() const;
    protected:
        virtual int validate() const;
        virtual HeaderCollection specialHeaders() const;
        virtual ParameterCollection specialParameters() const;
        AlibabaCloud::PDS::RequestPayer requestPayer_;
        std::string versionId_;
    };

    class ALIBABACLOUD_PDS_EXPORT PdsResumableBaseRequest : public PdsRequest
    {
    public:
        PdsResumableBaseRequest(const std::string& checkpointDir, const uint64_t partSize, const uint32_t threadNum) :
            PdsRequest(),
            partSize_(partSize),
            checkpointDir_(checkpointDir),
            requestPayer_(AlibabaCloud::PDS::RequestPayer::NotSet),
            trafficLimit_(0),
            versionId_()
        {
            threadNum_ = threadNum == 0 ? 1 : threadNum;
        }

        PdsResumableBaseRequest(const std::wstring& checkpointDir, const uint64_t partSize, const uint32_t threadNum) :
            PdsRequest(),
            partSize_(partSize),
            checkpointDirW_(checkpointDir),
            requestPayer_(AlibabaCloud::PDS::RequestPayer::NotSet),
            trafficLimit_(0),
            versionId_()
        {
            threadNum_ = threadNum == 0 ? 1 : threadNum;
        }

        void setPartSize(uint64_t partSize);
        uint64_t PartSize() const;

        void setObjectSize(uint64_t objectSize);
        uint64_t ObjectSize() const;

        void setThreadNum(uint32_t threadNum);
        uint32_t ThreadNum() const;

        void setCheckpointDir(const std::string& checkpointDir);
        const std::string& CheckpointDir() const;

        void setCheckpointDir(const std::wstring& checkpointDir);
        const std::wstring& CheckpointDirW() const;

        bool hasCheckpointDir() const;

        void setObjectMtime(const std::string& mtime);
        const std::string& ObjectMtime() const;

        void setRequestPayer(RequestPayer value);
        AlibabaCloud::PDS::RequestPayer RequestPayer() const;

        void setTrafficLimit(uint64_t value);
        uint64_t TrafficLimit() const;

        void setVersionId(const std::string& versionId);
        const std::string& VersionId() const;

    protected:
        friend class PdsClientImpl;

        virtual int validate() const;
        const char *validateMessage(int code) const;

    protected:
        uint64_t partSize_;
        uint64_t objectSize_;
        uint32_t threadNum_;
        std::string checkpointDir_;
        std::wstring checkpointDirW_;
        std::string mtime_;
        AlibabaCloud::PDS::RequestPayer requestPayer_;
        uint64_t trafficLimit_;
        std::string versionId_;
    };
}
}
