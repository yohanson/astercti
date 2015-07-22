#ifndef _CHAN_EVENTS_H_
#define _CHAN_EVENTS_H_

#include "asterisk.h"
#include "observer.h"

/*class ChannelEvent
{
	enum ast_channel_state m_state;
	std::string m_ChannelID;
	std::string m_Channel;
	std::string m_CallerIDNum;
	std::string m_CallerIDName;
	std::string m_peerChannel;
};*/

// Takes AmiMessage, calls corresponding method of listeners
class EventGenerator : IObserver
{
	std::list<EventListener *> _listeners;
	void handleEvent(const AmiMessage &message);
public:
	void add(EventListener &);
	void remove(EventListener &);
	void Notify();
};

class EventListener
{
protected:
	void OnRing();
	void OnOriginate();
	void OnDial();
	void OnUp();
	void OnHangup();
	void OnCdr();

};

#endif
