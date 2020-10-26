#include <conio.h>
#include "MsgSvr.h"
// #pragma comment(lib, "user32.lib")

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
    sprintLog("MsgSvr onRcvMsg: %d, %d \r\n", msg.type, msg.val);
    return true;
}

int _tmain() {
    MsgSvr msgSvr(sizeof(MsgStruct), onRcvMsg);
    msgSvr.Listen(string("share_mem"));
    for (int i = 0; i < 101; ++i) {
        MsgStruct msg;
        msg.type = msg_client;
        msg.val = i;
        msgSvr.PostMsg(msg);
        msgSvr.WaitMsg();
    }
    return 0;
}