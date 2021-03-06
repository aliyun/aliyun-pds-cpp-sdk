#include <alibabacloud/pds/PdsClient.h>

class ResumableSample
{
public:
    ResumableSample();
    ~ResumableSample();

    std::string ResumableFileUpload();
    std::string ResumableFileUploadStopOnce();
    std::string ResumableFileUploadCancel();

    void ResumableFileDownload(const std::string& fileID);
    void ResumableFileDownloadStopOnce(const std::string& fileID);
    void ResumableFileDownloadCancel(const std::string& fileID);

private:
    void PrintError(const std::string &funcName, const AlibabaCloud::PDS::PdsError &error);
    AlibabaCloud::PDS::PdsClient *client;
    std::string bucket_;
};
