#ifndef _ASTERISK_H_
#define _ASTERISK_H_

#include <map>
#include <list>
#include <wx/wx.h>
#include <wx/socket.h>
#include "observer.h"

enum {ID_SOCKET = 100500};
#define RECV_BUFF 8192

typedef std::map<std::string, std::string> AmiMessage;

class Asterisk : public wxEvtHandler
{
private:
	wxSocketClient *m_socket;
	std::list<IObserver* > _observers;
	char m_recv_buff[RECV_BUFF];

	void Notify(AmiMessage &message)
	{
		for (auto iter : _observers)
		{
			iter->handleEvent(message);
		}
	}

	void OnSocketEvent(wxSocketEvent &event)
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
				//std::cout << "... socket is now connected." << std::endl;
				break;

			default:
				//std::cout << "Unknown socket event!!!" << std::endl;
				break;
		}
	}

	void OnInputAvailable()
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

public:
	void add(IObserver& observer)
	{
		_observers.push_back(&observer);
	}
	void lol()
	{	
		std::string action = "\nAction: originate\nChannel: SIP/it1\nContext: rinet\nExten: 580\nPriority: 1\nCallerID: AngryCookie\n\n";
		m_socket->Write(action.c_str(), action.length()+1);
	}

	Asterisk(wxString host, int port, wxString username, wxString secret)
	{
		Asterisk(host.ToStdString(), port, username.ToStdString(), secret.ToStdString());
	}

	Asterisk(std::string host, int port, std::string username, std::string secret)
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
		m_socket->Write(login.c_str(), login.length()+1);
		std::cout << "Socket connected." << std::endl;
	}

	~Asterisk()
	{
		m_socket->Close();
	}
	
	void HangupChannel(std::string &channel)
	{
		std::string action = "\nAction: hangup\nChannel: "+channel+"\n\n";
		m_socket->Write(action.c_str(), action.length()+1);
	}
};


#endif
