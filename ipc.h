#ifndef IPC_H
#define IPC_H

#include <wx/ipc.h>
#include "controller.h"

#define IPC_TOPIC "astercti.IPC"
#define IPC_SERVICENAME "/tmp/astercti-ipc.socket"

class IpcConnection : public wxConnection
{
public:
    IpcConnection();
    IpcConnection(AsteriskController *);
    virtual bool OnExecute(const wxString& topic, const void *data, size_t size, wxIPCFormat format);
	virtual const void *OnRequest(const wxString& topic, const wxString& item, size_t *size, wxIPCFormat format);

protected:
    wxCharBuffer m_requestData;
    AsteriskController *m_controller;
};


class IpcServer : public wxServer
{
public:
    IpcServer(AsteriskController *);
    virtual ~IpcServer();

    void Disconnect();
    bool IsConnected() { return m_connection != NULL; }
    IpcConnection *GetConnection() { return m_connection; }
    virtual wxConnectionBase *OnAcceptConnection(const wxString& topic);

protected:
    IpcConnection *m_connection;
    AsteriskController *m_controller;
};


class IpcClient: public wxClient
{
public:
    IpcClient();
    ~IpcClient();
    bool Connect(const wxString& sHost, const wxString& sService, const wxString& sTopic);
    void Disconnect();
    wxConnectionBase *OnMakeConnection();
    bool IsConnected() { return m_connection != NULL; };
    IpcConnection *GetConnection() { return m_connection; };

protected:
    IpcConnection     *m_connection;
};


#endif // IPC_H
