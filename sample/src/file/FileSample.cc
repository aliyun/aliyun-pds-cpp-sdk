#include <iostream>
#include "../Config.h"
#include "FileSample.h"
#include <alibabacloud/pds/Const.h>
#include <memory>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace AlibabaCloud::PDS;

FileSample::FileSample()
{
    ClientConfiguration conf;
    client = new PdsClient(Config::Endpoint, Config::AccessToken, conf);
}

FileSample::~FileSample() {
    delete client;
}

void FileSample::PrintError(const std::string &funcName, const PdsError &error)
{
    std::cout << funcName << " fail" <<
        ",code:" << error.Code() <<
        ",message:" << error.Message() <<
        ",request_id:" << error.RequestId() << std::endl;
}

std::string FileSample::FileCreate()
{
    // PDS create
    std::string fileID;
    FileCreateRequest createRequest(Config::DriveID, Config::RootParentID, "test_file", "", "refuse", 3);
    auto createOutcome = client->FileCreate(createRequest);
    if (!createOutcome.isSuccess()) {
        PrintError(__FUNCTION__, createOutcome.error());
        return fileID;
    }
    createOutcome.result().PrintString();
    fileID = createOutcome.result().FileID();
    std::string uploadID = createOutcome.result().UploadID();

    // PDS get upload url
    PartInfoReqList partInfoReqList;
    PartInfoReq info(1, 3, 0, 0);
    partInfoReqList.push_back(info);

    FileGetUploadUrlRequest getUploadUrlRequest(Config::DriveID, fileID, uploadID, partInfoReqList);
    auto getUploadUrlOutcome = client->FileGetUploadUrl(getUploadUrlRequest);
    if (!getUploadUrlOutcome.isSuccess()) {
        PrintError(__FUNCTION__, getUploadUrlOutcome.error());
        return fileID;
    }
    getUploadUrlOutcome.result().PrintString();


    PartInfoRespList partInfoRespList = getUploadUrlOutcome.result().PartInfoRespList();
    if (partInfoRespList.size() == 0) {
        std::cout << __FUNCTION__ << " partInfoRespList empty" << std::endl;
        return fileID;
    }
    std::string uploadURl = partInfoRespList[0].UploadUrl();
    std::cout << __FUNCTION__ << " upload url" << uploadURl << std::endl;

    // upload to OSS
    std::shared_ptr<std::iostream> content = std::make_shared<std::stringstream>();
    *content << "123";
    PutObjectByUrlRequest uploadObjectRequest(uploadURl, content);
    auto uploadObjectOutcome = client->PutObjectByUrl(uploadObjectRequest);
    if (!uploadObjectOutcome.isSuccess()) {
        PrintError(__FUNCTION__, uploadObjectOutcome.error());
        return fileID;
    }
    std::cout << __FUNCTION__ << "upload object success" << std::endl;

    // PDS complete
    FileCompleteRequest completeRequest(Config::DriveID, fileID, uploadID);
    auto completeOutcome = client->FileComplete(completeRequest);
    if (!completeOutcome.isSuccess()) {
        PrintError(__FUNCTION__, completeOutcome.error());
        return fileID;
    }
    completeOutcome.result().PrintString();
    return fileID;
}

void FileSample::FileDownload(const std::string& fileID)
{
    FileGetRequest request(Config::DriveID, fileID);
    auto getOutcome = client->FileGet(request);
    if (getOutcome.isSuccess()) {
        getOutcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, getOutcome.error());
    }
    getOutcome.result().PrintString();

    std::string downloadURl = getOutcome.result().DownloadUrl();
    int64_t size = getOutcome.result().Size();
    std::cout << __FUNCTION__ << " download url:" << downloadURl << ", size:" << size << std::endl;

    // download from OSS
    GetObjectByUrlRequest downloadObjectRequest(downloadURl);
    auto downloadObjectOutcome = client->GetObjectByUrl(downloadObjectRequest);
    if (!downloadObjectOutcome.isSuccess()) {
        PrintError(__FUNCTION__, downloadObjectOutcome.error());
    }

    auto content = downloadObjectOutcome.result().Content()->rdbuf();
    std::cout << __FUNCTION__ << " download object success" << std::endl;
    std::cout << __FUNCTION__ << " " << content << std::endl;
}

void FileSample::FileRename(const std::string& fileID)
{
    FileRenameRequest request(Config::DriveID, fileID, "test_file", "auto_rename");
    auto outcome = client->FileRename(request);
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

void FileSample::FileTrash(const std::string& fileID)
{
    FileTrashRequest request(Config::DriveID, fileID);
    auto outcome = client->FileTrash(request);
    outcome.result().PrintString();
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

void FileSample::FileDelete(const std::string& fileID)
{
    FileDeleteRequest request(Config::DriveID, fileID);
    auto outcome = client->FileDelete(request);
    outcome.result().PrintString();
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

