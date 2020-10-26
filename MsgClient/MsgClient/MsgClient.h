#pragma once
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <sstream>

using namespace std;

enum MsgType {
    msg_client = 1,
    msg_svr = 2,
    win_handle = 3,
};
struct MsgStruct {
    MsgType type;
    int val;
};

class EvtOpt {
private:
    HANDLE _hEvent;
public:
    EvtOpt() {}
    ~EvtOpt() {
        Uninit();
    }
    bool Create(string name);                               // for event svr
    bool Connect(string name);                            // for event client
    bool Wait(DWORD time = INFINITE);
    bool Signal();
    bool Uninit();
};

typedef bool(*PFRcvMsg)(MsgStruct & msg);
class MsgClient {
private:
    HANDLE _hMapFile;
    LPVOID _pBuf;
    int _bufSize;
    bool _exited;

    EvtOpt _evtClient;                          // for signal
    EvtOpt _evtSvr;                              // for wait 

    PFRcvMsg _onRcvMsg;
public:
    MsgClient(int bufSize, PFRcvMsg onRcvMsg) : 
        _bufSize(bufSize),
        _onRcvMsg(onRcvMsg), 
        _exited(false) { }
    ~MsgClient() { Uninit(); }
    bool Connect(string name);
    bool Uninit();
    bool WaitMsg(int time = INFINITE);
    bool PostMsg(MsgStruct &msg);
};

