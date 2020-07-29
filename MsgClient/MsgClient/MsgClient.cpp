#include "MsgClient.h"
using namespace std;

EvtClient::~EvtClient()
{
    Uninit();
}

bool EvtClient::Connect(string name)
{
    _hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, name.c_str());
    return _hEvent != NULL;
}

bool EvtClient::Wait(DWORD time /*= INFINITE*/)
{
    if (_hEvent) WaitForSingleObject(_hEvent, time);
    return true;
}

bool EvtClient::Signal()
{
    if (_hEvent) SetEvent(_hEvent);
    return true;
}

bool EvtClient::Uninit()
{
    if (_hEvent) CloseHandle(_hEvent);
    return true;
}

bool MsgClient::Connect(string name)
{
    string strGlobal = "Local\\";
    _evtClient.Connect((strGlobal + name + string("_event")).c_str());

    _hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, _bufSize, (strGlobal + name).c_str());
    if (_hMapFile == NULL)  return false;
    _pBuf = (LPTSTR)MapViewOfFile(_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _bufSize);
    return _pBuf != NULL;
}

bool MsgClient::Unint()
{
    _evtClient.Signal();
    if (_pBuf) UnmapViewOfFile(_pBuf);
    if (_hMapFile) CloseHandle(_hMapFile);
    return true;
}

bool MsgClient::WaitMsg(int time /*= INFINITE*/)
{
    _evtClient.Wait(time);
    if (_onRcvMsg) _onRcvMsg(*((MsgStruct*)_pBuf));
    return true;
}

bool MsgClient::PostMsg(MsgStruct &msg)
{
    memcpy_s((void*)_pBuf, _bufSize, &msg, sizeof(msg));
    _evtClient.Signal();
    return true;
}
