#include <alibabacloud/pds/PdsClient.h>

class DirSample
{
public:
    DirSample();
    ~DirSample();

    std::string DirCreate();
    void DirRename(const std::string& fileID);
    void DirList(const std::string& parentFileID);
    void DirSearch();
    void DirTrash(const std::string& fileID);
    std::string DirDelete(const std::string& fileID);
    std::string DirCopy(const std::string& fileID, const std::string& toParentFileID);
    std::string DirMove(const std::string& fileID, const std::string& toParentFileID);
    void DirHidden(const std::string& fileID);
    void AsyncTaskGet(const std::string& fileID);

private:
    void PrintError(const std::string &funcName, const AlibabaCloud::PDS::PdsError &error);
    AlibabaCloud::PDS::PdsClient *client;
    std::string bucket_;
};
