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

#include <string>
#include <map>
#include <ctime>
#include <fstream>
#include <memory>
#include <alibabacloud/pds/Types.h>
#include "FileSystemUtils.h"
#include <alibabacloud/pds/Const.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#define  pds_access(a)  ::_access((a), 0)
#define  pds_mkdir(a)   ::_mkdir(a)
#define  pds_rmdir(a)   ::_rmdir(a)
#define  pds_stat       ::_stat64
#define  pds_waccess(a)  ::_waccess((a), 0)
#define  pds_wmkdir(a)   ::_wmkdir(a)
#define  pds_wrmdir(a)   ::_wrmdir(a)
#define  pds_wstat       ::_wstat64
#define  pds_wremove     ::_wremove
#define  pds_wrename     ::_wrename
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define  pds_access(a)  ::access(a, 0)
#define  pds_mkdir(a)   ::mkdir((a), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define  pds_rmdir(a)   ::rmdir(a)
#define  pds_stat       stat
#endif
using namespace AlibabaCloud::PDS;

bool AlibabaCloud::PDS::CreateDirectory(const std::string &folder)
{
    std::string folder_builder;
    std::string sub;
    sub.reserve(folder.size());
    for (auto it = folder.begin(); it != folder.end(); ++it) {
        const char c = *it;
        sub.push_back(c);
        if (c == PATH_DELIMITER || it == folder.end() - 1) {
            folder_builder.append(sub);
            if (pds_access(folder_builder.c_str()) != 0) {
                if (pds_mkdir(folder_builder.c_str()) != 0) {
                    return false;
                }
            }
            sub.clear();
        }
    }
    return true;
}

bool AlibabaCloud::PDS::IsDirectoryExist(std::string folder) {
    if (folder[folder.length() - 1] != PATH_DELIMITER && folder[folder.length() - 1] != '/') {
        folder += PATH_DELIMITER;
    }
    return !pds_access(folder.c_str());
}

bool AlibabaCloud::PDS::RemoveDirectory(const std::string &folder)
{
    return !pds_rmdir(folder.c_str());
}

bool AlibabaCloud::PDS::RemoveFile(const std::string &filepath)
{
    int ret = ::remove(filepath.c_str());
    return !ret;
}

bool AlibabaCloud::PDS::RenameFile(const std::string &from, const std::string &to)
{
    return !::rename(from.c_str(), to.c_str());
}

bool AlibabaCloud::PDS::GetPathLastModifyTime(const std::string& path, time_t& t)
{
    std::streamsize size;
    return GetPathInfo(path, t, size);
}

bool AlibabaCloud::PDS::GetPathInfo(const std::string& path, time_t& t, std::streamsize& size)
{
    struct pds_stat buf;
    auto filename = path.c_str();
#if defined(_WIN32) && _MSC_VER < 1900
    std::string tmp;
    if (!path.empty() && (path.rbegin()[0] == PATH_DELIMITER)) {
        tmp = path.substr(0, path.size() - 1);
        filename = tmp.c_str();
    }
#endif
    if (pds_stat(filename, &buf) != 0)
        return false;

    t = buf.st_mtime;
    size = static_cast<std::streamsize>(buf.st_size);
    return true;
}

bool AlibabaCloud::PDS::IsFileExist(const std::string& file)
{
    std::streamsize size;
    time_t t;
    return GetPathInfo(file, t, size);
}


//wchar path
#ifdef _WIN32

bool AlibabaCloud::PDS::CreateDirectory(const std::wstring &folder)
{
    std::wstring folder_builder;
    std::wstring sub;
    sub.reserve(folder.size());
    for (auto it = folder.begin(); it != folder.end(); ++it) {
        auto c = *it;
        sub.push_back(c);
        if (c == WPATH_DELIMITER || it == folder.end() - 1) {
            folder_builder.append(sub);
            if (pds_waccess(folder_builder.c_str()) != 0) {
                if (pds_wmkdir(folder_builder.c_str()) != 0) {
                    return false;
                }
            }
            sub.clear();
        }
    }
    return true;
}

bool AlibabaCloud::PDS::IsDirectoryExist(std::wstring folder) {
    if (folder[folder.length() - 1] != WPATH_DELIMITER && folder[folder.length() - 1] != '/') {
        folder += WPATH_DELIMITER;
    }
    return !pds_waccess(folder.c_str());
}

bool AlibabaCloud::PDS::RemoveDirectory(const std::wstring& folder)
{
    return !pds_wrmdir(folder.c_str());
}

bool AlibabaCloud::PDS::RemoveFile(const std::wstring& filepath)
{
    int ret = pds_wremove(filepath.c_str());
    return !ret;
}

bool AlibabaCloud::PDS::RenameFile(const std::wstring& from, const std::wstring& to)
{
    return !pds_wrename(from.c_str(), to.c_str());
}

bool AlibabaCloud::PDS::GetPathLastModifyTime(const std::wstring& path, time_t& t)
{
    std::streamsize size;
    return GetPathInfo(path, t, size);
}

bool AlibabaCloud::PDS::GetPathInfo(const std::wstring& path, time_t& t, std::streamsize& size)
{
    struct pds_stat buf;
    auto filename = path.c_str();
#if defined(_WIN32) && _MSC_VER < 1900
    std::wstring tmp;
    if (!path.empty() && (path.rbegin()[0] == WPATH_DELIMITER)) {
        tmp = path.substr(0, path.size() - 1);
        filename = tmp.c_str();
    }
#endif
    if (pds_wstat(filename, &buf) != 0)
        return false;

    t = buf.st_mtime;
    size = static_cast<std::streamsize>(buf.st_size);
    return true;
}

bool AlibabaCloud::PDS::IsFileExist(const std::wstring& file)
{
    std::streamsize size;
    time_t t;
    return GetPathInfo(file, t, size);
}
#endif

std::shared_ptr<std::fstream> AlibabaCloud::PDS::GetFstreamByPath(
    const std::string& path, const std::wstring& pathw,
    std::ios_base::openmode mode)
{
#ifdef _WIN32
    if (!pathw.empty()) {
        return std::make_shared<std::fstream>(pathw, mode);
    }
#else
    ((void)(pathw));
#endif
    return std::make_shared<std::fstream>(path, mode);
}
