#include "ResumableCallBack.h"
#include <alibabacloud/pds/Const.h>

const int32_t ProgressControlStopAfterCallTimes = 600;
const int32_t ProgressControlCancelAfterCallTimes = 100;
int32_t progressControlCallTimes = 0;

void ResetProgressControlCallTimes()
{
    progressControlCallTimes = 0;
}

void ProgressCallback(size_t increment, int64_t transfered, int64_t total, void* userData)
{
    std::cout << "ProgressCallback[" << userData << "] => " <<
                 increment <<" ," << transfered << "," << total << std::endl;
}

// const int32_t ProgressControlContinue = 0;
// const int32_t ProgressControlStop = 1;
// const int32_t ProgressControlCancel = 2;
int32_t ProgressControlCallback(void* userData)
{
    int32_t controlFlag = 0;
    progressControlCallTimes = progressControlCallTimes + 1;
    std::cout << "ProgressControlCallback[" << userData << "] => " << controlFlag  << " ," << progressControlCallTimes << std::endl;
    return controlFlag;
}

int32_t ProgressControlStopCallback(void* userData)
{
    int32_t controlFlag = 0;
    progressControlCallTimes = progressControlCallTimes + 1;
    if (progressControlCallTimes > ProgressControlStopAfterCallTimes) {
        controlFlag = 1;
    }
    std::cout << "ProgressControlStopCallback[" << userData << "] => " << controlFlag << " ," << progressControlCallTimes << std::endl;
    return controlFlag;
}

int32_t ProgressControlCancelCallback(void* userData)
{
    int32_t controlFlag = 0;
    progressControlCallTimes = progressControlCallTimes + 1;
    if (progressControlCallTimes > ProgressControlCancelAfterCallTimes) {
        controlFlag = 2;
    }
    std::cout << "ProgressControlCancelCallback[" << userData << "] => " << controlFlag << " ," << progressControlCallTimes << std::endl;
    return controlFlag;
}
