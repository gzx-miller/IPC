#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <sstream>
#pragma comment(lib, "user32.lib")

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

class EvtSvr {
private:
    HANDLE _hEvent;
public:
    EvtSvr() {}
    ~EvtSvr() {
        Uninit();
    }
    bool Create(string name) {
        _hEvent = CreateEvent(NULL, FALSE, FALSE, name.c_str());
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
class MsgSvr {
private:
    HANDLE _hMapFile;
    LPVOID _pBuf;
    int _bufSize;
    EvtSvr _evtSvr;
    PFRcvMsg _onRcvMsg;
public:
    MsgSvr(int bufSize, PFRcvMsg onRcvMsg) : _bufSize(bufSize),
        _onRcvMsg(onRcvMsg) { }
    ~MsgSvr() { Unint(); }
    bool Listen(string name) {
        string strGlobal = "Global\\";
        _evtSvr.Create((strGlobal + name + string("_event")).c_str());
        _evtSvr.Wait();

        _hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE,
            (strGlobal + name).c_str());
        if (_hMapFile == NULL) return false;
        _pBuf = (LPTSTR)MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _bufSize);
        if (!_pBuf) return false;
        if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
        return true;
    }
    bool Unint() {
        _evtSvr.Signal();
        if (_pBuf)  UnmapViewOfFile(_pBuf);
        if (_hMapFile) CloseHandle(_hMapFile);
        return true;
    }
    bool WaitMsg(int time = INFINITE) {
        _evtSvr.Wait(time);
        if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
        return true;
    }
    bool PostMsg(MsgStruct &msg) {
        if (_pBuf) memcpy_s((void*)_pBuf, _bufSize, &msg, sizeof(msg));
        _evtSvr.Signal();
        return true;
    }
};

bool onRcvMsg(MsgStruct & msg) {
    sprintLog("MsgSvr onRcvMsg: %d, %d \r\n", msg.type, msg.val);
    return true;
}

int _tmain() {
    MsgSvr msgSvr(sizeof(MsgStruct), onRcvMsg);
    msgSvr.Listen(string("my_shared_memory"));
    for (int i = 0; i < 100; ++i) {
        MsgStruct msg;
        msg.type = msg_client;
        msg.val = i;
        msgSvr.PostMsg(msg);
        msgSvr.WaitMsg();
    }
    return 0;
}