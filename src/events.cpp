#include "events.h"

EventGenerator::EventGenerator(const std::string &myexten)
    : m_myexten(myexten)
{}

void EventGenerator::broadcast(EventListener &listener)
{
	_listeners.push_back(&listener);
    listener.subscribe(*this);
}

void EventGenerator::no_broadcast(EventListener &listener)
{
	_listeners.remove(&listener);
}

void EventGenerator::NotifyOnRing(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnRing(message);
	}
}

void EventGenerator::NotifyOnOriginate(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnOriginate(message);
	}
}

void EventGenerator::NotifyOnDial(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnDial(message);
	}
}

void EventGenerator::NotifyOnDialIn(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnDialIn(message);
	}
}


void EventGenerator::NotifyOnUp(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnUp(message);
	}
}

void EventGenerator::NotifyOnHangup(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnHangup(message);
	}
}

void EventGenerator::NotifyOnCdr(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnCdr(message);
	}
}

void EventGenerator::NotifyOnLookupStart(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnLookupStart(message);
	}
}

void EventGenerator::NotifyOnLookupFinish(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnLookupFinish(message);
	}
}
void EventGenerator::NotifyOnInternalMessage(const AmiMessage &message) {
	for (auto iter : _listeners) {
		iter->OnInternalMessage(message);
	}
}
void EventGenerator::NotifyOnCallerInfoAvailable(const AmiMessage &message) {
}
void EventGenerator::handleEvent(const AmiMessage &m)
{
	if (m["Event"] == "Newstate")
	{
		switch (std::stoi(m["ChannelState"]))
		{
		case AST_STATE_RINGING:
			if (m["ConnectedLineNum"] == m_myexten)
				NotifyOnOriginate(m);
			else
				NotifyOnRing(m);
			break;

		case AST_STATE_RING:
			NotifyOnDial(m);
			break;

		case AST_STATE_UP:
			NotifyOnUp(m);
			break;
		}
	}
	else if (m["Event"] == "Hangup")
	{
		NotifyOnHangup(m);
	}
	else if (m["Event"] == "Cdr")
	{
		NotifyOnCdr(m);
	}
    else if (m["Event"] == "Dial")
    {
        NotifyOnDialIn(m);
    }
	else if (m.has("InternalMessage"))
	{
		NotifyOnInternalMessage(m);
	}
}

EventListener::EventListener()
{
}

EventListener::~EventListener()
{
    for (auto eg : m_eventgenerators)
        eg->no_broadcast(*this);
}

void EventListener::subscribe(EventGenerator &eg)
{
    m_eventgenerators.push_back(&eg);
}

void EventListener::OnDialIn(const AmiMessage &){};
void EventListener::OnRing(const AmiMessage &){
    m_last_channel_state = AST_STATE_RINGING;
};
void EventListener::OnOriginate(const AmiMessage &){};
void EventListener::OnDial(const AmiMessage &){};
void EventListener::OnUp(const AmiMessage &){};
void EventListener::OnHangup(const AmiMessage &){};
void EventListener::OnCdr(const AmiMessage &){};
void EventListener::OnLookupStart(const AmiMessage &){};
void EventListener::OnLookupFinish(const AmiMessage &){};
void EventListener::OnInternalMessage(const AmiMessage &){};
void EventListener::OnCallerInfoAvailable(const AmiMessage &){};
