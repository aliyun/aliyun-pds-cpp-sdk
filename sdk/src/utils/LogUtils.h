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
#include <alibabacloud/pds/Types.h>

namespace AlibabaCloud
{
namespace PDS
{
    void InitLogInner();
    void DeinitLogInner();

    LogLevel    GetLogLevelInner();
    LogCallback GetLogCallbackInner();
    void SetLogLevelInner(LogLevel level);
    void SetLogCallbackInner(LogCallback callback);

    void FormattedLog(LogLevel logLevel, const char* tag, const char* formatStr, ...);

#ifdef DISABLE_PDS_LOGGING

    #define PDS_LOG(level, tag, ...)

#else

    #define PDS_LOG(level, tag, ...) \
    { \
        if ( AlibabaCloud::PDS::GetLogCallbackInner() && AlibabaCloud::PDS::GetLogLevelInner() >= level ) \
        { \
            FormattedLog(level, tag, __VA_ARGS__); \
        } \
    }

#endif // DISABLE_PDS_LOGGING
}
}

