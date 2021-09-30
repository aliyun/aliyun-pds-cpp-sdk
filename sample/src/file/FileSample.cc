#include <iostream>
#include "../Config.h"
#include "../utils/MyCredentialsProvider.h"
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
    std::shared_ptr<MyCredentialsProvider> cProvider = std::make_shared<MyCredentialsProvider>(Config::AccessToken);
    client = new PdsClient(Config::Endpoint, cProvider, conf);
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
    FileCreateRequest createRequest(Config::DriveID, Config::RootParentID, "test_file", "", "auto_rename", 3);
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
    std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(Config::FileToUpload, std::ios::in | std::ios::binary);
    DataPutByUrlRequest uploadDataRequest(uploadURl, content);
    auto uploadDataOutcome = client->DataPutByUrl(uploadDataRequest);
    if (!uploadDataOutcome.isSuccess()) {
        PrintError(__FUNCTION__, uploadDataOutcome.error());
        return fileID;
    }
    std::cout << __FUNCTION__ << "upload data success" << std::endl;

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
    FileGetRequest request(Config::DriveID, "", fileID);
    auto getOutcome = client->FileGet(request);
    if (getOutcome.isSuccess()) {
        getOutcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, getOutcome.error());
        return;
    }
    getOutcome.result().PrintString();

    std::string downloadURl = getOutcome.result().DownloadUrl();
    int64_t size = getOutcome.result().Size();
    std::cout << __FUNCTION__ << " download url:" << downloadURl << ", size:" << size << std::endl;

    // download from OSS
    DataGetByUrlRequest downloadDataRequest(downloadURl);
    downloadDataRequest.setResponseStreamFactory([=]() {return std::make_shared<std::fstream>(Config::FileDownloadTo,
        std::ios_base::out | std::ios_base::in | std::ios_base::trunc| std::ios_base::binary); });
    auto downloadDataOutcome = client->DataGetByUrl(downloadDataRequest);
    if (!downloadDataOutcome.isSuccess()) {
        PrintError(__FUNCTION__, downloadDataOutcome.error());
        return;
    }

    auto content = downloadDataOutcome.result().Content()->rdbuf();
    std::cout << __FUNCTION__ << " download data success" << std::endl;
    std::cout << __FUNCTION__ << " " << content << std::endl;
}

void FileSample::FileGet(const std::string& fileID)
{
    FileGetRequest request(Config::DriveID, "", fileID);
    auto outcome = client->FileGet(request);
    if (outcome.isSuccess()) {
        std::cout << __FUNCTION__ << " FileGet success" << std::endl;
        outcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

void FileSample::FileRename(const std::string& fileID)
{
    FileRenameRequest request(Config::DriveID, fileID, "test_file", "auto_rename");
    auto outcome = client->FileRename(request);
    if (outcome.isSuccess()) {
        std::cout << __FUNCTION__ << " FileRename success" << std::endl;
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
        std::cout << __FUNCTION__ << " FileTrash success" << std::endl;
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
        std::cout << __FUNCTION__ << " FileDelete success" << std::endl;
        outcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

void FileSample::UserTagsPut(const std::string& fileID)
{

    UserTagList tags;
    tags.push_back(UserTag("key1", "value1"));
    tags.push_back(UserTag("key2", "value2"));
    MetaUserTagsPutRequest request(Config::DriveID, fileID, tags);
    auto outcome = client->MetaUserTagsPut(request);
    if (outcome.isSuccess()) {
        std::cout << __FUNCTION__ << " UserTagsPut success, fileID:" << outcome.result().FileID() << std::endl;
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

void FileSample::UserTagsDelete(const std::string& fileID)
{

    std::vector<std::string> keys;
    keys.push_back("key1");
    keys.push_back("key2");
    MetaUserTagsDeleteRequest request(Config::DriveID, fileID, keys);
    auto outcome = client->MetaUserTagsDelete(request);
    if (outcome.isSuccess()) {
        std::cout << __FUNCTION__ << " UserTagsDelete success" << std::endl;
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}