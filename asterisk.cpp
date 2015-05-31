
#include <map>
#include <list>
#include <wx/wx.h>
#include <wx/socket.h>
#include <sstream>
#include "observer.h"
#include "asterisk.h"


void Asterisk::Notify(AmiMessage &message)
{
	for (auto iter : _observers)
	{
		iter->handleEvent(message);
	}
}

void Asterisk::OnSocketEvent(wxSocketEvent &event)
{
	switch ( event.GetSocketEvent() )
	{
		case wxSOCKET_INPUT:
			//std::cout << "Input available on the socket" << std::endl;
			OnInputAvailable();
			break;

		case wxSOCKET_LOST:
			//std::cout << "Socket connection was unexpectedly lost." << std::endl;
			break;

		case wxSOCKET_CONNECTION:
			std::cout << "... socket is now connected." << std::endl;
			break;

		default:
			//std::cout << "Unknown socket event!!!" << std::endl;
			break;
	}
}

void Asterisk::OnInputAvailable()
{
	m_socket->Read(m_recv_buff, RECV_BUFF);
	int read_bytes = m_socket->LastReadCount();
	m_recv_buff[read_bytes] = 0;
	std::string raw_messages = m_recv_buff;
	std::string delim = "\r\n";
	std::string line, key, value;
	size_t start = 0, colon;
	size_t end = raw_messages.find(delim);
	AmiMessage am;
	while (end != std::string::npos)
	{
		line = raw_messages.substr(start, end-start);
		//std::cout << line << std::endl;
		if ( ! line.length() )
		{
			//std::cout << "===================================\n" << message << std::endl;
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
		}
		start = end + delim.length();
		end = raw_messages.find(delim, start);
	}
}

void Asterisk::add(IObserver& observer)
{
	_observers.push_back(&observer);
}
Asterisk::Asterisk(std::string host, int port, std::string username, std::string secret)
{
	Bind(wxEVT_SOCKET, &Asterisk::OnSocketEvent, this);
	m_socket = new wxSocketClient;
	m_socket->SetEventHandler(*this);
	m_socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG | wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
	m_socket->Notify(true);
	wxIPV4address addr;
	addr.Hostname(host);
	addr.Service(port);
	m_socket->Connect(addr);
	std::string login = "Action: login\nUsername: "+username+"\nSecret: "+secret+"\n\n";
	m_socket->Write(login.c_str(), login.length());
}

Asterisk::~Asterisk()
{
	m_socket->Close();
}

void Asterisk::Originate(std::string mychan, std::string context, std::string exten, int priority)
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
		<< "\nCallerID: " << exten
		<< "\nActionID: " << mychan << "-" << id
		<< "\n\n";
	std::string action = actionstream.str();
	std::cout << "Going to originate with this: '" << action << "'" << std::endl;
	m_socket->Write(action.c_str(), action.length());
}


void Asterisk::HangupChannel(std::string &channel)
{
	std::string action = "Action: hangup\nChannel: "+channel+"\n\n";
	m_socket->Write(action.c_str(), action.length());
}

