#include <alibabacloud/pds/PdsClient.h>

class ResumableSample
{
public:
    ResumableSample();
    ~ResumableSample();

    std::string ResumableFileUpload();
    void ResumableFileDownload(const std::string& fileID);

private:
    void PrintError(const std::string &funcName, const AlibabaCloud::PDS::PdsError &error);
    AlibabaCloud::PDS::PdsClient *client;
    std::string bucket_;
};
