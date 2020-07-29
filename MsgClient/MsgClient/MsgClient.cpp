#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <sstream>

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

enum MsgType {
    msg_client = 1,
    msg_svr = 2,
    win_handle = 3,
};
struct MsgStruct {
    MsgType type;
    int val;
};

class EvtClient {
private:
    HANDLE _hEvent;
public:
    EvtClient() {}
    ~EvtClient() {
        Uninit();
    }
    bool Connect(string name) {
        _hEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, name.c_str());
        return _hEvent != NULL;
    }
    bool Wait(DWORD time = INFINITE) {
        if (_hEvent) WaitForSingleObject(_hEvent, time);
        return true;
    }
    bool Signal() {
        if (_hEvent) SetEvent(_hEvent);
        return true;
    }
    bool Uninit() {
        if (_hEvent) CloseHandle(_hEvent);
        return true;
    }
};

typedef bool(*PFRcvMsg)(MsgStruct & msg);
class MsgClient {
private:
    HANDLE _hMapFile;
    LPVOID _pBuf;
    int _bufSize;
    EvtClient _evtClient;
    PFRcvMsg _onRcvMsg;
public:
    MsgClient(int bufSize, PFRcvMsg onRcvMsg) : _bufSize(bufSize),
        _onRcvMsg(onRcvMsg) { }
    ~MsgClient() { Unint(); }
    bool Connect(string name) {
        string strGlobal = "Global\\";
        _evtClient.Connect((strGlobal + name + string("_event")).c_str());

        _hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
            PAGE_READWRITE, 0, _bufSize, (strGlobal + name).c_str());
        if (_hMapFile == NULL)  return false;
        _pBuf = (LPTSTR)MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _bufSize);
        return _pBuf != NULL;
    }
    bool Unint() {
        _evtClient.Signal();
        if (_pBuf) UnmapViewOfFile(_pBuf);
        if (_hMapFile) CloseHandle(_hMapFile);
        return true;
    }
    bool WaitMsg(int time = INFINITE) {
        _evtClient.Wait(time);
        if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
        return true;
    }
    bool PostMsg(MsgStruct &msg) {
        memcpy_s((void*)_pBuf, _bufSize, &msg, sizeof(msg));
        _evtClient.Signal();
        return true;
    }
};

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