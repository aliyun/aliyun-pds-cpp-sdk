#include <iostream>
#include "../Config.h"
#include "ResumableSample.h"
#include <alibabacloud/pds/Const.h>
#include <memory>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace AlibabaCloud::PDS;

ResumableSample::ResumableSample()
{
    ClientConfiguration conf;
    client = new PdsClient(Config::Endpoint, Config::AccessToken, conf);
}

ResumableSample::~ResumableSample() {
    delete client;
}

void ResumableSample::PrintError(const std::string &funcName, const PdsError &error)
{
    std::cout << funcName << " fail" <<
        ",code:" << error.Code() <<
        ",message:" << error.Message() <<
        ",request_id:" << error.RequestId() << std::endl;
}

static void ProgressCallback(size_t increment, int64_t transfered, int64_t total, void* userData)
{
    std::cout << "ProgressCallback[" << userData << "] => " <<
                 increment <<" ," << transfered << "," << total << std::endl;
}

std::string ResumableSample::ResumableFileUpload()
{
    std::string fileID;
    FileUploadRequest uploadRequest(Config::DriveID, Config::RootParentID, "test_resumable_file", "", "refuse", Config::FileToUpload);
    TransferProgress progressCallback = { ProgressCallback , this };
    uploadRequest.setTransferProgress(progressCallback);
    auto uploadOutcome = client->ResumableFileUpload(uploadRequest);
    if (!uploadOutcome.isSuccess()) {
        PrintError(__FUNCTION__, uploadOutcome.error());
        return fileID;
    }
    uploadOutcome.result().PrintString();
    fileID = uploadOutcome.result().FileID();

    return fileID;
}

void ResumableSample::ResumableFileDownload(const std::string& fileID)
{
    FileDownloadRequest downloadRequest(Config::DriveID, fileID, Config::FileDownloadTo);
    TransferProgress progressCallback = { ProgressCallback , this };
    downloadRequest.setTransferProgress(progressCallback);
    auto getOutcome = client->ResumableFileDownload(downloadRequest);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
    }
    else {
        std::cout << __FUNCTION__ << " download file success" << std::endl;
    }
}
