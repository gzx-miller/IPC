#include "MsgClient.h"
using namespace std;

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

bool MsgClient::Connect(string name)
{
    string strGlobal = "Local\\";
    string strSvr = strGlobal + name + string("_svr");
    string strClient = strGlobal + name + string("_client");

    if (!_evtClient.Connect(strSvr.c_str())) {
        _evtClient.Create(strSvr.c_str());
    }
    _evtClient.Connect(strSvr.c_str());

    if (!_evtSvr.Connect(strClient.c_str())) {
        _evtSvr.Create(strClient.c_str());
    }
    _evtSvr.Connect(strClient.c_str());
    _evtClient.Signal();

    _hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, _bufSize, (strGlobal + name).c_str());
    if (_hMapFile == NULL)  return false;
    _pBuf = (LPTSTR)MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _bufSize);
    return _pBuf != NULL;
}

bool MsgClient::Uninit()
{
    _exited = true;
    _evtClient.Signal();
    _evtSvr.Uninit();
    if (_pBuf) UnmapViewOfFile(_pBuf);
    if (_hMapFile) CloseHandle(_hMapFile);
    return true;
}

bool MsgClient::WaitMsg(int time /*= INFINITE*/)
{
    if (_exited) return false;
    _evtSvr.Wait(time);
    if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
    return true;
}

bool MsgClient::PostMsg(MsgStruct &msg)
{
    memcpy_s((void*)_pBuf, _bufSize, &msg, sizeof(msg));
    _evtClient.Signal();
    return true;
}
