#pragma once
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <sstream>
// #pragma comment(lib, "user32.lib")

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

typedef bool(*PFRcvMsg)(MsgStruct& msg);
class MsgOpt {
private:
    HANDLE _hMapFile;
    LPVOID _pBuf;
    int _bufSize;
    bool _exited;

    EvtOpt _evtOpt;

    PFRcvMsg _onRcvMsg;
public:
    MsgOpt(int bufSize) :_bufSize(bufSize) {};             // used by client
    MsgOpt(int bufSize, PFRcvMsg onRcvMsg) :      // used by svr
        _bufSize(bufSize),
        _onRcvMsg(onRcvMsg),
        _exited(false) { }
    ~MsgOpt() { Uninit(); }
    bool Listen(string name);                                     // used by svr
    bool WaitMsg(int time = INFINITE);
    bool Connect(string name);                                 // used by client
    bool PostMsg(MsgStruct& msg);
    bool Uninit();
};

class MsgMgr {
private:
    MsgOpt _msgSvr;
    MsgOpt _msgClient;

public:
    MsgMgr(int bufSize, PFRcvMsg onRcvMsg) :
        _msgSvr(bufSize, onRcvMsg),
        _msgClient(bufSize) {};
    ~MsgMgr() { Destroy(); };
    bool Create(string name, bool main);
    bool WaitMsg(int time = INFINITE);
    bool PostMsg(MsgStruct& msg);
    bool Destroy();
};