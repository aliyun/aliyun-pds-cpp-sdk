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
#include <ctime>
#include <iostream>
#include <alibabacloud/pds/Types.h>

namespace AlibabaCloud
{
namespace PDS
{
    #define UNUSED_PARAM(x) ((void)(x))

    std::string ComputeContentMD5(const std::string& data);
    std::string ComputeContentMD5(const char *data, size_t size);
    std::string ComputeContentMD5(std::istream & stream);

    std::string ComputeContentSha1(const std::string& data);
    std::string ComputeContentSha1(const char *data, size_t size);
    std::string ComputeContentSha1(std::istream & stream);
    std::string ComputeContentSha1(std::istream & stream, ProgressControl progressControl);

    std::string ComputeContentETag(const std::string& data);
    std::string ComputeContentETag(const char *data, size_t size);
    std::string ComputeContentETag(std::istream & stream);

    std::string GenerateUuid();
    std::string UrlEncode(const std::string &src);
    std::string UrlDecode(const std::string &src);

    std::string Base64Encode(const std::string &src);
    std::string Base64Encode(const ByteBuffer& buffer);
    std::string Base64Encode(const char *src, int len);
    std::string Base64EncodeUrlSafe(const std::string &src);
    std::string Base64EncodeUrlSafe(const char *src, int len);

    std::string XmlEscape(const std::string& value);

    ByteBuffer Base64Decode(const char *src, int len);
    ByteBuffer Base64Decode(const std::string &src);

    void StringReplace(std::string &src, const std::string &s1, const std::string &s2);
    std::string LeftTrim(const char* source);
    std::string RightTrim(const char* source);
    std::string Trim(const char* source);
    std::string LeftTrimQuotes(const char* source);
    std::string RightTrimQuotes(const char* source);
    std::string TrimQuotes(const char* source);
    std::string ToLower(const char* source);
    std::string ToUpper(const char* source);
    std::string ToGmtTime(std::time_t &t);
    std::string ToUtcTime(std::time_t &t);
    std::time_t UtcToUnixTime(const std::string &t);

    bool IsValidEndpoint(const std::string &value);
    std::string GetHostString(const std::string &endpoint);

    const std::string &LookupMimeType(const std::string& name);

    std::streampos GetIOStreamLength(std::iostream &stream);
}
}
