#include <iostream>
#include "../Config.h"
#include "../utils/ResumableCallBack.h"
#include "../utils/MyCredentialsProvider.h"
#include "ShareLinkSample.h"
#include <alibabacloud/pds/Const.h>
#include <memory>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace AlibabaCloud::PDS;

ShareLinkSample::ShareLinkSample()
{
    ClientConfiguration conf;
    std::shared_ptr<MyCredentialsProvider> cProvider = std::make_shared<MyCredentialsProvider>(Config::AccessToken);
    client = new PdsClient(Config::Endpoint, cProvider, conf);
}

ShareLinkSample::~ShareLinkSample() {
    delete client;
}

void ShareLinkSample::PrintError(const std::string &funcName, const PdsError &error)
{
    std::cout << funcName << " fail" <<
        ",code:" << error.Code() <<
        ",message:" << error.Message() <<
        ",request_id:" << error.RequestId() << std::endl;
}

void ShareLinkSample::DirList(const std::string& parentFileID)
{
    DirListRequest request("", Config::ShareID, parentFileID, "updated_at", "DESC", "*", "", 10);
    request.setShareToken(Config::ShareToken);

    auto outcome = client->DirList(request);
    if (!outcome.isSuccess()) {
        PrintError(__FUNCTION__, outcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call DirList success, parent file id: " <<  parentFileID << std::endl;
    auto fileList = outcome.result().FileItemList();
    for (uint32_t i = 0; i < fileList.size(); i++) {
        std::cout << "Type: " << fileList[i].Type() << ", FileID: " << fileList[i].FileID() <<  ", Name: " << fileList[i].Name() << std::endl;
    }
}

void ShareLinkSample::FileDownload(const std::string& fileID)
{
    FileGetDownloadUrlRequest request("", Config::ShareID, fileID);
    request.setShareToken(Config::ShareToken);

    auto getOutcome = client->FileGetDownloadUrl(request);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call FileGetDownloadUrl success" << std::endl;

    std::string downloadURl = getOutcome.result().Url();
    int64_t size = getOutcome.result().Size();
    std::cout << __FUNCTION__ << " download url: " << downloadURl << ", size: " << size << std::endl;

    // download from OSS
    DataGetByUrlRequest downloadDataRequest(downloadURl);
    downloadDataRequest.setResponseStreamFactory([=]() {return std::make_shared<std::fstream>(Config::FileDownloadTo,
        std::ios_base::out | std::ios_base::in | std::ios_base::trunc| std::ios_base::binary); });
    auto downloadDataOutcome = client->DataGetByUrl(downloadDataRequest);
    if (!downloadDataOutcome.isSuccess()) {
        PrintError(__FUNCTION__, downloadDataOutcome.error());
        return;
    }

    std::cout << __FUNCTION__ << " call DataGetByUrl to download data success" << std::endl;
}


void ShareLinkSample::ResumableFileDownload(const std::string& fileID)
{
    FileDownloadRequest downloadRequest("", Config::ShareID,  fileID, Config::FileDownloadTo, Config::CheckpointDir);
    downloadRequest.setShareToken(Config::ShareToken);

    TransferProgress progressCallback = { ProgressCallback , this };
    downloadRequest.setTransferProgress(progressCallback);
    auto getOutcome = client->ResumableFileDownload(downloadRequest);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call ResumableFileDownload success" << std::endl;
}

void ShareLinkSample::ResumableFileDownloadStopOnce(const std::string& fileID)
{
    ResetProgressControlCallTimes();

    FileDownloadRequest downloadRequest("", Config::ShareID, fileID, Config::FileDownloadTo, Config::CheckpointDir);
    downloadRequest.setShareToken(Config::ShareToken);

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
        std::cout << __FUNCTION__ << "second time call ResumableFileDownload success" << std::endl;
    }
}

void ShareLinkSample::ResumableFileDownloadCancel(const std::string& fileID)
{
    ResetProgressControlCallTimes();

    FileDownloadRequest downloadRequest("", Config::ShareID, fileID, Config::FileDownloadTo, Config::CheckpointDir);
    downloadRequest.setShareToken(Config::ShareToken);

    TransferProgress progressCallback = { ProgressCallback , this };
    downloadRequest.setTransferProgress(progressCallback);

    ProgressControl progressControlCancelCallback = { ProgressControlCancelCallback , this };
    downloadRequest.setProgressControl(progressControlCancelCallback);
    auto getOutcome = client->ResumableFileDownload(downloadRequest);
    if (!getOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getOutcome.error());
        return;
    }
    std::cout << __FUNCTION__ << " call ResumableFileDownload success" << std::endl;
}
