#include <iostream>

void ResetProgressControlCallTimes();
void ProgressCallback(size_t increment, int64_t transfered, int64_t total, void* userData);
int32_t ProgressControlCallback(void* userData);
int32_t ProgressControlStopCallback(void* userData);
int32_t ProgressControlCancelCallback(void* userData);