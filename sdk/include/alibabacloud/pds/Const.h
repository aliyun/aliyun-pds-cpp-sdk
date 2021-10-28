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

namespace AlibabaCloud
{
namespace PDS
{
    const int64_t MaxFileSize = 5LL * 1024LL * 1024LL * 1024LL;
    const int32_t PartNumberUpperLimit = 10000;
    const int32_t DefaultPartSize = 8 * 1024 * 1024;
    const int32_t PartSizeLowerLimit = 100 * 1024;
    const int32_t DefaultResumableThreadNum = 3;
    const int32_t ProgressControlContinue = 0;
    const int32_t ProgressControlStop = 1;
    const int32_t ProgressControlCancel = 2;

    const std::string DefaultImageThumbnailProcess = "image/resize,w_160/format,jpeg";
    const std::string DefaultImageUrlProcess = "image/resize,w_1920/format,jpeg";
    const std::string DefaultVideoThumbnailProcess = "video/snapshot,t_0,f_jpg,w_100,ar_auto";

#ifdef _WIN32
    const char PATH_DELIMITER = '\\';
    const wchar_t WPATH_DELIMITER = L'\\';
#else
    const char PATH_DELIMITER = '/';
    const wchar_t WPATH_DELIMITER = L'/';
#endif
}
}
