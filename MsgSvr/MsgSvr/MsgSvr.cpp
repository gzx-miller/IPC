#include "MsgSvr.h"

bool EvtOpt::Create(string name)
{
    _hEvent = CreateEvent(NULL, FALSE, FALSE, name.c_str());
    return _hEvent != NULL;
}

bool EvtOpt::Connect(string name)
{
    _hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, name.c_str());
    return _hEvent != NULL;
}

bool EvtOpt::Wait(DWORD time /*= INFINITE*/)
{
    if (_hEvent) WaitForSingleObject(_hEvent, time);
    return true;
}

bool EvtOpt::Signal()
{
    if (_hEvent) SetEvent(_hEvent);
    return true;
}

bool EvtOpt::Uninit()
{
    if (_hEvent) {
        SetEvent(_hEvent);
        CloseHandle(_hEvent);
        _hEvent = NULL;
    }
    return true;
}

bool MsgSvr::Listen(string name)
{
    string strGlobal = "Local\\";
    string strSvr = strGlobal + name + string("_svr");
    string strClient = strGlobal + name + string("_client");

    if (!_evtSvr.Connect(strSvr.c_str())) {
        _evtSvr.Create(strSvr.c_str());
    }
    _evtSvr.Connect(strSvr.c_str());

    if (!_evtClient.Connect(strClient.c_str())) {
        _evtClient.Create(strClient.c_str());
    }
    _evtClient.Connect(strClient.c_str());
    _evtSvr.Wait();

    _hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE,
        (strGlobal + name).c_str());
    if (_hMapFile == NULL) return false;
    _pBuf = (LPTSTR)MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _bufSize);
    if (!_pBuf) return false;
    if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
    return true;
}

bool MsgSvr::Uninit()
{
    _exited = true;
    _evtClient.Signal();
    _evtSvr.Uninit();
    if (_pBuf)  UnmapViewOfFile(_pBuf);
    if (_hMapFile) CloseHandle(_hMapFile);
    return true;
}

bool MsgSvr::WaitMsg(int time /*= INFINITE*/)
{
    if (_exited) return false;
    _evtSvr.Wait(time);
    if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
    return true;
}

bool MsgSvr::PostMsg(MsgStruct &msg)
{
    if (_pBuf) memcpy_s((void*)_pBuf, _bufSize, &msg, sizeof(msg));
    _evtClient.Signal();
    return true;
}
