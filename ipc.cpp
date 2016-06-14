#include "ipc.h"
#include "mainframe.h"

IpcConnection::IpcConnection()
    : m_controller(NULL)
{
}

IpcConnection::IpcConnection(AsteriskController *controller, IpcServer *server)
    : m_controller(controller), m_server(server)
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
        if (m_controller)
        {
            m_controller->Originate(number.ToStdString());
        }
		return true;
	}
    else if (uri == IPC_CMD_RISE)
    {
       if (m_controller)
       {
            m_controller->ShowMainFrame();
       }
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

IpcServer::IpcServer(AsteriskController *controller)
    : m_connection(NULL), m_controller(controller)
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
        m_connection = new IpcConnection(m_controller, this);
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

