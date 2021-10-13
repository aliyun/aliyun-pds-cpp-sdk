#include <iostream>
#include "../Config.h"
#include "../utils/MyCredentialsProvider.h"
#include "../utils/ResumableCallBack.h"
#include "ResumableSample.h"
#include <alibabacloud/pds/Const.h>
#include <memory>
#include <mutex>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace AlibabaCloud::PDS;

ResumableSample::ResumableSample()
{
    ClientConfiguration conf;
    // enable rapid upload, rapid upload is disabled by default.
    conf.enableRapidUpload = true;
    std::shared_ptr<MyCredentialsProvider> cProvider = std::make_shared<MyCredentialsProvider>("");
    client = new PdsClient(Config::Endpoint, cProvider, conf);
    // update access tokensu
    cProvider->setAccessToken(Config::AccessToken);
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

std::string ResumableSample::ResumableFileUpload()
{
    std::string fileID;
    FileUploadRequest uploadRequest(Config::DriveID, Config::RootParentID, "test_resumable_file", "", "refuse", Config::FileToUpload, "checkpoint_dir");

    // set user tags when create
    UserTagList userTagList;
    UserTag userTag("key", "value");
    userTagList.push_back(userTag);
    uploadRequest.setUserTags(userTagList);

    TransferProgress progressCallback = { ProgressCallback , this };
    uploadRequest.setTransferProgress(progressCallback);
    auto uploadOutcome = client->ResumableFileUpload(uploadRequest);
    if (!uploadOutcome.isSuccess()) {
        PrintError(__FUNCTION__, uploadOutcome.error());
        return fileID;
    }

    fileID = uploadOutcome.result().FileID();
    std::string isRapidUpload = uploadOutcome.result().RapidUpload() ? "is rapid upload" : "is not rapid upload";
    std::cout << __FUNCTION__ << " call ResumableFileUpload success, file id: " << fileID << ", " << isRapidUpload << std::endl;

    return fileID;
}

std::string ResumableSample::ResumableFileUploadStopOnce()
{
    ResetProgressControlCallTimes();

    std::string fileID;
    FileUploadRequest uploadRequest(Config::DriveID, Config::RootParentID, "test_resumable_file_stop_once", "", "refuse", Config::FileToUpload, "checkpoint_dir");
    TransferProgress progressCallback = { ProgressCallback , this };
    uploadRequest.setTransferProgress(progressCallback);

    ProgressControl progressControlStopCallback = { ProgressControlStopCallback , this };
    uploadRequest.setProgressControl(progressControlStopCallback);
    auto uploadOutcome = client->ResumableFileUpload(uploadRequest);
    if (!uploadOutcome.isSuccess()) {
        PrintError(__FUNCTION__, uploadOutcome.error());
    } else {
        fileID = uploadOutcome.result().FileID();
        std::string isRapidUpload = uploadOutcome.result().RapidUpload() ? "is rapid upload" : "is not rapid upload";
        std::cout << __FUNCTION__ << " first time call ResumableFileUpload success, file id: " << fileID << ", " << isRapidUpload << std::endl;
        return fileID;
    }

    ProgressControl progressControlCallback = { ProgressControlCallback , this };
    uploadRequest.setProgressControl(progressControlCallback);
    uploadOutcome = client->ResumableFileUpload(uploadRequest);
    if (!uploadOutcome.isSuccess()) {
        PrintError(__FUNCTION__, uploadOutcome.error());
        return fileID;
    } else {
        fileID = uploadOutcome.result().FileID();
        std::string isRapidUpload = uploadOutcome.result().RapidUpload() ? "is rapid upload" : "is not rapid upload";
        std::cout << __FUNCTION__ << " second time call ResumableFileUpload success, file id: " << fileID << ", " << isRapidUpload << std::endl;
    }

    return fileID;
}

std::string ResumableSample::ResumableFileUploadCancel()
{
    ResetProgressControlCallTimes();

    std::string fileID;
    FileUploadRequest uploadRequest(Config::DriveID, Config::RootParentID, "test_resumable_file_cancel", "", "refuse", Config::FileToUpload, "checkpoint_dir");
    TransferProgress progressCallback = { ProgressCallback , this };
    uploadRequest.setTransferProgress(progressCallback);

    ProgressControl progressControlCancelCallback = { ProgressControlCancelCallback , this };
    uploadRequest.setProgressControl(progressControlCancelCallback);
    auto uploadOutcome = client->ResumableFileUpload(uploadRequest);
    if (!uploadOutcome.isSuccess()) {
        PrintError(__FUNCTION__, uploadOutcome.error());
        return fileID;
    } else {
        fileID = uploadOutcome.result().FileID();
        std::string isRapidUpload = uploadOutcome.result().RapidUpload() ? "is rapid upload" : "is not rapid upload";
        std::cout << __FUNCTION__ << " call ResumableFileUpload success, file id: " << fileID << ", " << isRapidUpload << std::endl;
    }
    return fileID;
}

void ResumableSample::ResumableFileDownload(const std::string& fileID)
{
    FileDownloadRequest downloadRequest(Config::DriveID, "", fileID, Config::FileDownloadTo, "checkpoint_dir");
    TransferProgress progressCallback = { ProgressCallback , this };
    downloadRequest.setTransferProgress(progressCallback);
    auto getOutcome = client->ResumableFileDownload(downloadRequest);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
    }
    else {
        std::cout << __FUNCTION__ << " call ResumableFileDownload success" << std::endl;
    }
}

void ResumableSample::ResumableFileDownloadStopOnce(const std::string& fileID)
{
    ResetProgressControlCallTimes();

    FileDownloadRequest downloadRequest(Config::DriveID, "", fileID, Config::FileDownloadTo, "checkpoint_dir");
    TransferProgress progressCallback = { ProgressCallback , this };
    downloadRequest.setTransferProgress(progressCallback);

    ProgressControl progressControlStopCallback = { ProgressControlStopCallback , this };
    downloadRequest.setProgressControl(progressControlStopCallback);
    auto getOutcome = client->ResumableFileDownload(downloadRequest);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
    }
    else {
        std::cout << __FUNCTION__ << " first time call ResumableFileDownload success" << std::endl;
        return;
    }

    ProgressControl progressControlCallback = { ProgressControlCallback , this };
    downloadRequest.setProgressControl(progressControlCallback);
    getOutcome = client->ResumableFileDownload(downloadRequest);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
    }
    else {
        std::cout << __FUNCTION__ << " second time call ResumableFileDownload success" << std::endl;
    }
}

void ResumableSample::ResumableFileDownloadCancel(const std::string& fileID)
{
    ResetProgressControlCallTimes();

    FileDownloadRequest downloadRequest(Config::DriveID, "", fileID, Config::FileDownloadTo, "checkpoint_dir");
    TransferProgress progressCallback = { ProgressCallback , this };
    downloadRequest.setTransferProgress(progressCallback);

    ProgressControl progressControlCancelCallback = { ProgressControlCancelCallback , this };
    downloadRequest.setProgressControl(progressControlCancelCallback);
    auto getOutcome = client->ResumableFileDownload(downloadRequest);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
    }
    else {
        std::cout << __FUNCTION__ << " call ResumableFileDownload success" << std::endl;
    }
}