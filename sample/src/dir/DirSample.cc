#include <iostream>
#include "../Config.h"
#include "DirSample.h"
#include <alibabacloud/pds/Const.h>
#include <memory>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace AlibabaCloud::PDS;

DirSample::DirSample()
{
    ClientConfiguration conf;
    client = new PdsClient(Config::Endpoint, Config::AccessToken, conf);
}

DirSample::~DirSample() {
    delete client;
}

void DirSample::PrintError(const std::string &funcName, const PdsError &error)
{
    std::cout << funcName << " fail" <<
        ",code:" << error.Code() <<
        ",message:" << error.Message() <<
        ",request_id:" << error.RequestId() << std::endl;
}

std::string DirSample::DirCreate()
{
    DirCreateRequest request(Config::DriveID, Config::RootParentID, "test_dir", "auto_rename");
    auto outcome = client->DirCreate(request);
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
        return outcome.result().FileID();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
        return "";
    }
}

void DirSample::DirRename(const std::string& fileID)
{
    DirRenameRequest request(Config::DriveID, fileID, "test_dir2", "auto_rename");
    auto outcome = client->DirRename(request);
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

void DirSample::DirList(const std::string& parentFileID)
{
    DirListRequest request(Config::DriveID, parentFileID, "updated_at", "DESC", "*", "", 10);
    auto outcome = client->DirList(request);
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
        auto fileList = outcome.result().FileItemList();
        for (uint32_t i = 0; i < fileList.size(); i++) {
            std::cout << "Type: " << fileList[i].Type() << ", FileID: " << fileList[i].FileID() <<  ", Name: " << fileList[i].Name() << std::endl;
        }
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

void DirSample::DirTrash(const std::string& fileID)
{
    DirTrashRequest request(Config::DriveID, fileID);
    auto outcome = client->DirTrash(request);
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}

std::string DirSample::DirDelete(const std::string& fileID)
{
    DirDeleteRequest request(Config::DriveID, fileID);
    auto outcome = client->DirDelete(request);
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
        return outcome.result().AsyncTaskID();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
        return "";
    }

}

void DirSample::AsyncTaskGet(const std::string& asyncTaskID)
{
    AsyncTaskGetRequest request(asyncTaskID);
    auto outcome = client->AsyncTaskGet(request);
    if (outcome.isSuccess()) {
        outcome.result().PrintString();
    }
    else {
        PrintError(__FUNCTION__, outcome.error());
    }
}