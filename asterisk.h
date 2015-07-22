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

enum ast_channel_state {
    AST_STATE_DOWN,         /*!< Channel is down and available */
    AST_STATE_RESERVED,     /*!< Channel is down, but reserved */
    AST_STATE_OFFHOOK,      /*!< Channel is off hook */
    AST_STATE_DIALING,      /*!< Digits (or equivalent) have been dialed */
    AST_STATE_RING,         /*!< Line is ringing */
    AST_STATE_RINGING,      /*!< Remote end is ringing */
    AST_STATE_UP,        /*!< Line is up */
    AST_STATE_BUSY,         /*!< Line is busy */
    AST_STATE_DIALING_OFFHOOK, /*!< Digits (or equivalent) have been dialed while offhook */
    AST_STATE_PRERING,      /*!< Channel has detected an incoming call and is waiting for ring */
    AST_STATE_MUTE = (1 << 16),   /*!< Do not transmit voice data */
};

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
protected:
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
