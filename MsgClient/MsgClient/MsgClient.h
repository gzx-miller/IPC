#pragma once
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <sstream>

using namespace std;

class EvtClient {
private:
    HANDLE _hEvent;
public:
    EvtClient() {}
    ~EvtClient();
    bool Connect(string name);
    bool Wait(DWORD time = INFINITE);
    bool Signal();
    bool Uninit();
};
enum MsgType {
    msg_client = 1,
    msg_svr = 2,
    win_handle = 3,
};
struct MsgStruct {
    MsgType type;
    int val;
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
    bool Connect(string name);
    bool Unint();
    bool WaitMsg(int time = INFINITE);
    bool PostMsg(MsgStruct &msg);
};
