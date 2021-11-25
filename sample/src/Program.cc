#include <alibabacloud/pds/PdsClient.h>
#include <iostream>
#include "Config.h"

#include "dir/DirSample.h"
#include "file/FileSample.h"
#include "resumable/ResumableSample.h"
#include "sharelink/ShareLinkSample.h"

using namespace AlibabaCloud::PDS;

void LogCallbackFunc(LogLevel level, const std::string &stream)
{
    if (level == LogLevel::LogOff)
        return;

    std::cout << stream;
}

int main(void)
{
    std::cout << "pds-cpp-sdk samples" << std::endl;

    InitializeSdk();

    SetLogLevel(LogLevel::LogDebug);
    SetLogCallback(LogCallbackFunc);

    DirSample dirSample;
    std::string dirID = dirSample.DirCreate();
    if (!dirID.empty()) {
        dirSample.DirRename(dirID);
        dirSample.DirList(Config::RootParentID);
        dirSample.DirTrash(dirID);
        std::string asyncTaskID = dirSample.DirDelete(dirID);
        if (!asyncTaskID.empty()) {
            dirSample.AsyncTaskGet(asyncTaskID);
        }
    }

    FileSample fileSample;
    std::string fileID = fileSample.FileCreate();
    if (!fileID.empty()) {
        fileSample.FileDownload(fileID);
        fileSample.FileRename(fileID);
        fileSample.UserTagsPut(fileID);
        fileSample.FileGet(fileID);
        fileSample.UserTagsDelete(fileID);
        fileSample.FileGet(fileID);
        fileSample.FileTrash(fileID);
        fileSample.FileDelete(fileID);
    }

    ResumableSample resumableSample;
    std::string rFileID = resumableSample.ResumableFileUpload();
    if (!rFileID.empty()) {
        resumableSample.ResumableFileDownload(rFileID);
        fileSample.FileTrash(rFileID);
        fileSample.FileDelete(rFileID);
    }

    ShareLinkSample shareLinkSample;
    std::string shareFileID = Config::FileID;
    if (!shareFileID.empty()) {
        shareLinkSample.FileDownload(shareFileID);
        shareLinkSample.ResumableFileDownload(shareFileID);
    }
    if (!Config::ShareID.empty()) {
        shareLinkSample.DirList(Config::RootParentID);
    }

    ShutdownSdk();
    return 0;
}
