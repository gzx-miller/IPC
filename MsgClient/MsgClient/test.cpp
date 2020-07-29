#include "MsgClient.h"
#include <conio.h>

using namespace std;

static void WriteToDebug(const char* buffer, size_t size) {
    static HANDLE hFile = INVALID_HANDLE_VALUE;
    if (hFile == INVALID_HANDLE_VALUE) {
        hFile = CreateFileA("./debug.log", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return;
        // SetFilePointer(hFile, 0, NULL, FILE_END);
    }
    DWORD dwBytesWritten;
    if (!WriteFile(hFile, buffer, size, &dwBytesWritten, NULL)) return;
    FlushFileBuffers(hFile);
}

void sprintLog(const char *format, ...) {
    char strBuf[512] = { 0 };
    va_list ap;
    va_start(ap, format);
    _vsnprintf_s(strBuf, sizeof(strBuf) - 1, format, ap);
    va_end(ap);
    WriteToDebug(strBuf, strlen(strBuf));
    OutputDebugStringA(strBuf);
}

bool onRcvMsg(MsgStruct & msg) {
    sprintLog("MsgClient onRcvMsg: %d, %d \r\n", msg.type, msg.val);
    return true;
}

int _tmain()
{
    MsgClient msgClient(sizeof(MsgStruct), onRcvMsg);
    msgClient.Connect(string("my_shared_memory"));
    for (int i = 100; i < 200; ++i) {
        MsgStruct msg;
        msg.type = msg_svr;
        msg.val = i;
        msgClient.PostMsg(msg);
        msgClient.WaitMsg();
    }
    return 0;
}