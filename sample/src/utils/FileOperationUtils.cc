#include "FileOperationUtils.h"
#include <sys/stat.h>

bool GetPathInfo(const std::string& path, time_t& t, std::streamsize& size)
{
    struct stat buf;
    auto filename = path.c_str();
#if defined(_WIN32) && _MSC_VER < 1900
    std::string tmp;
    if (!path.empty() && (path.rbegin()[0] == PATH_DELIMITER)) {
        tmp = path.substr(0, path.size() - 1);
        filename = tmp.c_str();
    }
#endif
    if (stat(filename, &buf) != 0)
        return false;

    t = buf.st_mtime;
    size = static_cast<std::streamsize>(buf.st_size);
    return true;
}