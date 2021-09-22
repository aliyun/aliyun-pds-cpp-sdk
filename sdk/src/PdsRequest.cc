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

#include <alibabacloud/pds/PdsRequest.h>
#include <alibabacloud/pds/Const.h>
#include <sstream>
#include "http/HttpType.h"
#include "utils/Utils.h"
#include "model/ModelError.h"
#include "utils/FileSystemUtils.h"

using namespace AlibabaCloud::PDS;

 PdsRequest:: PdsRequest():
    ServiceRequest()
{
}

HeaderCollection PdsRequest::Headers() const
{
    auto headers = specialHeaders();
    return headers;
}

ParameterCollection PdsRequest::Parameters() const
{
    return  specialParameters();
}

std::shared_ptr<std::iostream> PdsRequest::Body() const
{
    std::string&& p = payload();
    std::shared_ptr<std::iostream> payloadBody;
    if (!p.empty())
    {
      payloadBody = std::make_shared<std::stringstream>();
      *payloadBody << p;
    }
    return payloadBody;
}

int PdsRequest::validate() const
{
    return 0;
}

const char * PdsRequest::validateMessage(int code) const
{
    return GetModelErrorMsg(code);
}

std::string PdsRequest::payload() const
{
    return "";
}

HeaderCollection PdsRequest::specialHeaders() const
{
    return HeaderCollection();
}

ParameterCollection PdsRequest::specialParameters() const
{
    return ParameterCollection();
}

int PdsObjectRequest::validate() const
{
    return 0;
}

void PdsObjectRequest::setRequestPayer(AlibabaCloud::PDS::RequestPayer key)
{
    requestPayer_ = key;
}

AlibabaCloud::PDS::RequestPayer PdsObjectRequest::RequestPayer() const
{
    return requestPayer_;
}

void PdsObjectRequest::setVersionId(const std::string& versionId)
{
    versionId_ = versionId;
}

const std::string& PdsObjectRequest::VersionId() const
{
    return versionId_;
}

HeaderCollection PdsObjectRequest::specialHeaders() const
{
    auto headers = PdsRequest::specialHeaders();
    if (requestPayer_ == AlibabaCloud::PDS::RequestPayer::Requester) {
        headers["x-oss-request-payer"] = ToLower(ToRequestPayerName(AlibabaCloud::PDS::RequestPayer::Requester));
    }
    return headers;
}

ParameterCollection PdsObjectRequest::specialParameters() const
{
    auto parameters = PdsRequest::specialParameters();
    if (!versionId_.empty()) {
        parameters["versionId"] = versionId_;
    }
    return parameters;
}

int PdsResumableBaseRequest::validate() const
{
    if (partSize_ < PartSizeLowerLimit) {
        return ARG_ERROR_CHECK_PART_SIZE_LOWER;
    }

    if (threadNum_ <= 0) {
        return ARG_ERROR_CHECK_THREAD_NUM_LOWER;
    }

#if !defined(_WIN32)
    if (!checkpointDirW_.empty()) {
        return ARG_ERROR_PATH_NOT_SUPPORT_WSTRING_TYPE;
    }
#endif

    // if directory do not exist, return error
    if (hasCheckpointDir()) {
        if ((!checkpointDir_.empty() && !IsDirectoryExist(checkpointDir_))
#ifdef _WIN32
            || (!checkpointDirW_.empty() && !IsDirectoryExist(checkpointDirW_))
#endif
            ) {
            return ARG_ERROR_CHECK_POINT_DIR_NONEXIST;
        }
    }

    return 0;
}

const char *PdsResumableBaseRequest::validateMessage(int code) const
{
    return GetModelErrorMsg(code);
}

void PdsResumableBaseRequest::setPartSize(uint64_t partSize)
{
    partSize_ = partSize;
}

uint64_t PdsResumableBaseRequest::PartSize() const
{
    return partSize_;
}

void PdsResumableBaseRequest::setObjectSize(uint64_t objectSize)
{
    objectSize_ = objectSize;
}

uint64_t PdsResumableBaseRequest::ObjectSize() const
{
    return objectSize_;
}

void PdsResumableBaseRequest::setThreadNum(uint32_t threadNum)
{
    threadNum_ = threadNum;
}

uint32_t PdsResumableBaseRequest::ThreadNum() const
{
    return threadNum_;
}

void PdsResumableBaseRequest::setCheckpointDir(const std::string &checkpointDir)
{
    checkpointDir_ = checkpointDir;
    checkpointDirW_.clear();
}

const std::string& PdsResumableBaseRequest::CheckpointDir() const
{
    return checkpointDir_;
}

void PdsResumableBaseRequest::setCheckpointDir(const std::wstring& checkpointDir)
{
    checkpointDirW_ = checkpointDir;
    checkpointDir_.clear();
}

const std::wstring& PdsResumableBaseRequest::CheckpointDirW() const
{
    return checkpointDirW_;
}

void PdsResumableBaseRequest::setObjectMtime(const std::string &mtime)
{
    mtime_ = mtime;
}

const std::string& PdsResumableBaseRequest::ObjectMtime() const
{
    return mtime_;
}

void PdsResumableBaseRequest::setRequestPayer(AlibabaCloud::PDS::RequestPayer value)
{
    requestPayer_ = value;
}

AlibabaCloud::PDS::RequestPayer PdsResumableBaseRequest::RequestPayer() const
{
    return requestPayer_;
}

void PdsResumableBaseRequest::setTrafficLimit(uint64_t value)
{
    trafficLimit_ = value;
}

uint64_t PdsResumableBaseRequest::TrafficLimit() const
{
    return trafficLimit_;
}

void PdsResumableBaseRequest::setVersionId(const std::string& versionId)
{
    versionId_ = versionId;
}

const std::string& PdsResumableBaseRequest::VersionId() const
{
    return versionId_;
}

bool PdsResumableBaseRequest::hasCheckpointDir() const
{
    return (!checkpointDir_.empty() || !checkpointDirW_.empty());
}