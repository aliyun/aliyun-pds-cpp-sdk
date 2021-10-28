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

#include <alibabacloud/pds/PdsResult.h>
#include <sstream>

using namespace AlibabaCloud::PDS;

PdsResult::PdsResult() :
    parseDone_(false)
{
}

PdsResult::PdsResult(const HeaderCollection& header):
    PdsResult()
{
    if (header.find("x-oss-request-id") != header.end()) {
        requestId_ = header.at("x-oss-request-id");
    }
    if (header.find("x-ca-request-id") != header.end()) {
        requestId_ = header.at("x-ca-request-id");
    }
}

PdsDataResult::PdsDataResult() :
    PdsResult()
{
}

PdsDataResult::PdsDataResult(const HeaderCollection& header) :
    PdsResult(header)
{
    if (header.find("x-oss-version-id") != header.end()) {
        versionId_ = header.at("x-oss-version-id");
    }
}
