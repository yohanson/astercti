#include "ipc.h"
#include "myapp.h"

IpcConnection::IpcConnection()
{
}

IpcConnection::IpcConnection(IpcServer *server)
    : m_server(server)
{
}

bool IpcConnection::OnExecute(const wxString& topic,
                              const void *data,
                              size_t size,
                              wxIPCFormat format)
    {
    const char *str = (const char *) data;
    wxString uri = wxString(str);
    wxString number;
    if (uri.StartsWith("tel:", &number))
    {
        number.Trim();
        bool badNumber = false;
        size_t pos;
        if (number.StartsWith("+"))
        { // global number
            while ((pos = number.find_first_not_of("0123456789+-.()")) != wxString::npos)
            {
                badNumber = true;
                break;
            }
        }
        else // local number
        {
            while ((pos = number.find_first_not_of("0123456789-.()abcdefABCDEF*#")) != wxString::npos)
            {
                badNumber = true;
                break;
            }
        }

        if (badNumber)
        {
            wxString msg;
            msg << "Wrong telephone number format: '" << number << "' (char[" << pos << "] == '" << number[pos] << "')";
            wxLogMessage(msg);
            std::cerr << msg << "\n";
            return false;
        }
        wxGetApp().Originate(number.ToStdString());
        return true;
    }
    else if (uri == IPC_CMD_RISE)
    {
        wxGetApp().ShowMainFrame();
    }
    return false;
}

const void *IpcConnection::OnRequest(const wxString& topic,
                        const wxString& item,
                        size_t *size,
                        wxIPCFormat format)
{
    *size = 0;


    wxString s = "123";
    *size = 3;

    // store the data pointer to which we return in a member variable to ensure
    // that the pointer remains valid even after we return
    m_requestData = s.mb_str();
    const void * const data = m_requestData;
    return data;
}

bool IpcConnection::OnDisconnect()
{
    if (m_server)
    {
        m_server->InvalidateConnection();
    }
    return wxConnection::OnDisconnect();
}

//--------------------

IpcServer::IpcServer()
    : m_connection(NULL)
{
}

IpcServer::~IpcServer()
{
    Disconnect();
}

wxConnectionBase *IpcServer::OnAcceptConnection(const wxString& topic)
{
    if ( topic == IPC_TOPIC )
    {
        m_connection = new IpcConnection(this);
        return m_connection;
    }
    else
    {
        wxLogMessage("IpcServer: Unknown topic '%s', connection refused", topic);
        return NULL;
    }
}

void IpcServer::Disconnect()
{
    if ( m_connection )
    {
        m_connection->Disconnect();
    }
}

void IpcServer::InvalidateConnection()
{
    m_connection = NULL;
}

bool
IpcServer::IsConnected()
{
   return m_connection != NULL;
}

IpcConnection *
IpcServer::GetConnection()
{
   return m_connection;
}

//------------------------

IpcClient::IpcClient() : wxClient()
{
    m_connection = NULL;
}

bool IpcClient::Connect(const wxString& sHost, const wxString& sService, const wxString& sTopic)
{
    wxLogNull nolog;

    m_connection = (IpcConnection *)MakeConnection(sHost, sService, sTopic);
    return m_connection    != NULL;
}

wxConnectionBase *IpcClient::OnMakeConnection()
{
    return new IpcConnection;
}

void IpcClient::Disconnect()
{
    if (m_connection)
    {
        m_connection->Disconnect();
    }
}

IpcClient::~IpcClient()
{
    Disconnect();
}

bool
IpcClient::IsConnected()
{
   return m_connection != NULL;
}

IpcConnection *
IpcClient::GetConnection()
{
   return m_connection;
}

