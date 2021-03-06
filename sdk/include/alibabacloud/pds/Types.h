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

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <alibabacloud/pds/Export.h>

namespace AlibabaCloud
{
namespace PDS
{
    enum LogLevel
    {
        LogOff = 0,
        LogFatal,
        LogError,
        LogWarn,
        LogInfo,
        LogDebug,
        LogTrace,
        LogAll,
    };

    typedef void(*LogCallback)(LogLevel level, const std::string& stream);

    struct  ALIBABACLOUD_PDS_EXPORT caseSensitiveLess
    {
        bool operator() (const std::string& lhs, const std::string& rhs) const
        {
            return lhs < rhs;
        }
    };

    struct  ALIBABACLOUD_PDS_EXPORT caseInsensitiveLess
    {
        bool operator() (const std::string& lhs, const std::string& rhs) const
        {
            auto first1 = lhs.begin(), last1 = lhs.end();
            auto first2 = rhs.begin(), last2 = rhs.end();
            while (first1 != last1) {
                if (first2 == last2)
                    return false;
                auto first1_ch = ::tolower(*first1);
                auto first2_ch = ::tolower(*first2);
                if (first1_ch != first2_ch) {
                    return (first1_ch < first2_ch);
                }
                ++first1; ++first2;
            }
            return (first2 != last2);
        }
    };

    using TransferProgressHandler = std::function<void(size_t increment, int64_t transferred, int64_t total, void *userData)>;
    struct  ALIBABACLOUD_PDS_EXPORT TransferProgress
    {
        TransferProgressHandler Handler;
        void *UserData;
    };

    using ProgressControlHandler = std::function<int32_t(void *userData)>;
    struct  ALIBABACLOUD_PDS_EXPORT ProgressControl
    {
        ProgressControlHandler Handler;
        void *UserData;
    };

    using MetaData = std::map<std::string, std::string, caseInsensitiveLess>;
    using HeaderCollection = std::map<std::string, std::string, caseInsensitiveLess>;
    using ParameterCollection = std::map<std::string, std::string, caseSensitiveLess>;
    using IOStreamFactory = std::function< std::shared_ptr<std::iostream>(void)>;
    using ByteBuffer = std::vector<unsigned char>;
}
}
