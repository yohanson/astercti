#ifndef IPC_H
#define IPC_H

#include <wx/ipc.h>
#include "controller.h"

static const std::string IPC_TOPIC      ( "astercti.IPC" );
static const std::string IPC_SERVICENAME( "/tmp/astercti-ipc.socket" );
static const std::string IPC_CMD_RISE   ( "cmd:rise" );

class IpcServer;

class IpcConnection : public wxConnection
{
public:
    IpcConnection();
    IpcConnection(AsteriskController *, IpcServer *);
    virtual bool OnExecute(const wxString& topic, const void *data, size_t size, wxIPCFormat format);
	virtual const void *OnRequest(const wxString& topic, const wxString& item, size_t *size, wxIPCFormat format);
    virtual bool OnDisconnect();

protected:
    wxCharBuffer m_requestData;
    AsteriskController *m_controller;
    IpcServer *m_server;
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
    void InvalidateConnection();

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
