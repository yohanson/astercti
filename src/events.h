#ifndef _CHAN_EVENTS_H_
#define _CHAN_EVENTS_H_

#include "asterisk.h"
#include "observer.h"
#include "controller.h"

/*class ChannelEvent
{
	enum ast_channel_state m_state;
	std::string m_ChannelID;
	std::string m_Channel;
	std::string m_CallerIDNum;
	std::string m_CallerIDName;
	std::string m_peerChannel;
};*/

class EventListener;

// Takes AmiMessage, calls corresponding method of listeners
class EventGenerator : public IObserver, public ControllerUser
{
protected:
    std::list<EventListener *> _listeners;
	void handleEvent(const AmiMessage &);
public:
	void broadcast(EventListener &);
	void no_broadcast(EventListener &);
	void NotifyOnDialIn(const AmiMessage &);
	void NotifyOnRing(const AmiMessage &);
	void NotifyOnOriginate(const AmiMessage &);
	void NotifyOnDial(const AmiMessage &);
	void NotifyOnUp(const AmiMessage &);
	void NotifyOnHangup(const AmiMessage &);
	void NotifyOnCdr(const AmiMessage &);
	void NotifyOnLookupStart(const AmiMessage &);
	void NotifyOnLookupFinish(const AmiMessage &);
	void NotifyOnInternalMessage(const AmiMessage &);
	virtual void NotifyOnCallerInfoAvailable(const AmiMessage &);
};

class EventListener
{
private:
    std::list<EventGenerator *> m_eventgenerators;
protected:
	ast_channel_state m_last_channel_state;
public:
    wxString edescr;
    EventListener();
    ~EventListener();
    void subscribe(EventGenerator &);
	virtual void OnDialIn(const AmiMessage &);
	virtual void OnRing(const AmiMessage &);
	virtual void OnOriginate(const AmiMessage &);
	virtual void OnDial(const AmiMessage &);
	virtual void OnUp(const AmiMessage &);
	virtual void OnHangup(const AmiMessage &);
	virtual void OnCdr(const AmiMessage &);
	virtual void OnLookupStart(const AmiMessage &);
	virtual void OnLookupFinish(const AmiMessage &);
	virtual void OnInternalMessage(const AmiMessage &);
	virtual void OnCallerInfoAvailable(const AmiMessage &);
};

#endif
