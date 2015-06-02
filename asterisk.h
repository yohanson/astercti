#ifndef _ASTERISK_H_
#define _ASTERISK_H_

#include <map>
#include <list>
#include <wx/wx.h>
#include <wx/socket.h>
#include <sstream>
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

	void Notify(AmiMessage &message);
	void OnSocketEvent(wxSocketEvent &event);
	void OnInputAvailable();
	
public:
	void add(IObserver& observer);
	Asterisk(std::string host, int port, std::string username, std::string secret);
	~Asterisk();
	void Originate(std::string mychan, std::string context, std::string exten, std::string myexten = "", int priority = 1);
	void HangupChannel(std::string &channel);
};

class Call
{
private:
	wxString m_number;
	wxString m_name;
	wxString m_description;
	wxDateTime m_time;
	int m_duration;
public:
	enum {CALL_IN, CALL_OUT} m_direction;
	enum {CALL_ANSWERED, CALL_UNANSWERED} m_disposition;
	
};
#endif
