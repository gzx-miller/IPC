#include "MsgOpt.h"

bool EvtOpt::Create(string name) {
    _hEvent = CreateEvent(NULL, FALSE, FALSE, name.c_str());
    return _hEvent != NULL;
}

bool EvtOpt::Connect(string name) {
    _hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, name.c_str());
    return _hEvent != NULL;
}

bool EvtOpt::Wait(DWORD time /*= INFINITE*/) {
    if (_hEvent) WaitForSingleObject(_hEvent, time);
    return true;
}

bool EvtOpt::Signal() {
    if (_hEvent) SetEvent(_hEvent);
    return true;
}

bool EvtOpt::Uninit() {
    if (_hEvent) {
        SetEvent(_hEvent);
        CloseHandle(_hEvent);
        _hEvent = NULL;
    }
    return true;
}

bool MsgOpt::Listen(string name) {
    string strName = "Local\\" + name;

    if (!_evtOpt.Connect(strName.c_str())) {
        _evtOpt.Create(strName.c_str());
    }
    if (!_evtOpt.Connect(strName.c_str())) return false;
    _evtOpt.Wait();

    _hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE,
        (strName + "_map").c_str());
    if (_hMapFile == NULL) return false;
    _pBuf = (LPTSTR)MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _bufSize);
    if (!_pBuf) return false;
    if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
    return true;
}

bool MsgOpt::Connect(string name) {
    string strName = "Local\\" + name;

    if (!_evtOpt.Connect(strName.c_str())) {
        _evtOpt.Create(strName.c_str());
    }
    if (!_evtOpt.Connect(strName.c_str())) return false;

    _hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, _bufSize, (strName+"_map").c_str());
    if (_hMapFile == NULL)  return false;
    _pBuf = (LPTSTR)MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _bufSize);

    _evtOpt.Signal();
    return _pBuf != NULL;
}

bool MsgOpt::Uninit() {
    _exited = true;
    _evtOpt.Uninit();
    if (_pBuf){
        UnmapViewOfFile(_pBuf);
        _pBuf = NULL;
    }
    if (_hMapFile) {
        CloseHandle(_hMapFile);
        _hMapFile = NULL;
    }
    return true;
}

bool MsgOpt::WaitMsg(int time /*= INFINITE*/) {
    if (_exited) return false;
    _evtOpt.Wait(time);
    if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
    return true;
}

bool MsgOpt::PostMsg(MsgStruct& msg) {
    if (_pBuf) memcpy_s((void*)_pBuf, _bufSize, &msg, sizeof(msg));
    _evtOpt.Signal();
    return true;
}

bool MsgMgr::Create(string name, bool main) {
    return _msgClient.Connect(name + (main ? "" : "_main")) &&
        _msgSvr.Listen(name + (main ? "_main" : ""));
}

bool MsgMgr::WaitMsg(int time /*= INFINITE*/) {
    return _msgSvr.WaitMsg();
}

bool MsgMgr::PostMsg(MsgStruct& msg) {
    return _msgClient.PostMsg(msg);
}

bool MsgMgr::Destroy() {
    _msgClient.Uninit();
    _msgSvr.Uninit();
    return true;
}
