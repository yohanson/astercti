
#include <map>
#include <list>
#include <wx/wx.h>
#include <wx/socket.h>
#include <sstream>
#include "observer.h"
#include "asterisk.h"


void Asterisk::Notify(AmiMessage &message)
{
    if (message.has("Response"))
    {
        if (message.has("Ping") && message["Ping"] == "Pong")
        {
            m_pingTimer.Stop();
            m_ping_timer_active = false;
            m_pingTimer.StartOnce(5000);
            AmiMessage m;
            m["InternalMessage"] = "ConnectionStatus";
            m["Status"] = "Connected";
            Notify(m);
        }
    }
    IObservable::Notify(message);
}

void Asterisk::OnSocketEvent(wxSocketEvent &event)
{
    std::string login;
    AmiMessage m;
    switch ( event.GetSocketEvent() )
    {
        case wxSOCKET_INPUT:
            OnInputAvailable();
            break;

        case wxSOCKET_OUTPUT:
            break;

        case wxSOCKET_LOST:
            m["InternalMessage"] = "ConnectionStatus";
            m["Status"] = "Lost";
            Notify(m);
            AmiConnect();
            break;

        case wxSOCKET_CONNECTION:
            login = "Action: login\nUsername: "+m_ami_username+"\nSecret: "+m_ami_secret+"\n\n";
            m_socket->Write(login.c_str(), login.length());
            AmiPing();
            break;

        default:
            std::cout << "Unknown socket event! (" << event.GetSocketEvent() << ")" << std::endl;
            break;
    }
}

void Asterisk::OnInputAvailable()
{
    m_socket->Read(m_recv_buff, RECV_BUFF-1);
    int read_bytes = m_socket->LastReadCount();
    m_recv_buff[read_bytes] = 0;
    std::string raw_messages = m_recv_buff;
    std::string delim = "\r\n";
    std::string line, key, value;
    size_t start = 0, colon;
    size_t end = raw_messages.find(delim);
    static AmiMessage am;
    while (end != std::string::npos)
    {
        line = raw_messages.substr(start, end-start);
        if ( ! line.length() )
        {
            Notify(am);
            am.clear();
        }
        else
        {
            colon = line.find(':');
            key = line.substr(0, colon);
            if (line.length() == colon+1)
                value = "";
            else
                value = line.substr(colon+2);
            am[key] = value;
            if (key == "Channel")
            {
                am["ChannelID"] = value.substr(0, value.find_last_of('-'));
            }
            else if (key == "DestinationChannel")
            {
                am["DestinationChannelID"] = value.substr(0, value.find_last_of('-'));
            }
            else if (key == "Destination")
            {
                am["DestinationChannelID"] = value.substr(0, value.find_last_of('-'));
            }
        }
        start = end + delim.length();
        end = raw_messages.find(delim, start);
    }
}

void Asterisk::AmiPing()
{
    m_ping_timer_active = true;
    if (!m_pingTimer.StartOnce(5000))
    {
        std::cerr << "Timer could not be started!" << std::endl;
    }
    std::string action = "Action: ping\n\n";
    m_socket->Write(action.c_str(), action.length());
}

void Asterisk::AmiRequestStatus()
{
    std::string cmd = "Action: status\n\n";
    m_socket->Write(cmd.c_str(), cmd.length());
}

void Asterisk::OnPingTimeout(wxTimerEvent& event)
{
    if (m_ping_timer_active)
    {
        std::cerr << wxDateTime::Now().FormatISOCombined() << " Connection lost" << std::endl;
        AmiMessage m;
        m["InternalMessage"] = "ConnectionStatus";
        m["Status"] = "Lost";
        Notify(m);
        m_socket->Close();
        AmiConnect();
    }
    else
        AmiPing();
}

void Asterisk::AmiConnect()
{
    wxIPV4address addr;
    addr.Hostname(m_ami_host);
    addr.Service(m_ami_port);
    m_socket->Connect(addr, false);
}

Asterisk::Asterisk(std::string host, int port, std::string username, std::string secret)
{
    m_ami_host = host;
    m_ami_port = port;
    m_ami_username = username;
    m_ami_secret = secret;
    Bind(wxEVT_SOCKET, &Asterisk::OnSocketEvent, this);
    m_socket = new wxSocketClient;
    m_socket->SetEventHandler(*this);
    m_socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG | wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
    m_socket->Notify(true);
    m_pingTimer.SetOwner(this);
    Bind(wxEVT_TIMER, &Asterisk::OnPingTimeout, this);
    AmiConnect();
}

Asterisk::~Asterisk()
{
    m_socket->Close();
    delete m_socket;
}

void Asterisk::Originate(std::string mychan, std::string context, std::string exten, std::string myexten, int priority)
{
    std::ostringstream actionstream;
    char id[32];
    time_t tmp_time = time(0);
    strftime(id, 32, "%H%M%S", localtime(&tmp_time));
    actionstream <<
           "Action: originate"
        << "\nChannel: " << mychan
        << "\nContext: " << context
        << "\nExten: " << exten
        << "\nPriority: " << priority
        << "\nAsync: yes"
        << "\nCallerID: \"" << exten << "\" <" << myexten << ">"
        << "\nActionID: " << mychan << "-" << id
        << "\n\n";
    std::string action = actionstream.str();
    m_socket->Write(action.c_str(), action.length());
}


void Asterisk::HangupChannel(const std::string &channel)
{
    std::string action = "Action: hangup\nChannel: "+channel+"\n\n";
    m_socket->Write(action.c_str(), action.length());
}

