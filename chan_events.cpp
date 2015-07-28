#include "chan_events.h"

void EventGenerator::add(EventListener &listener)
{
	_listeners.push_back(&listener);
}

void EventGenerator::remove(EventListener &listener)
{
	_listeners.remove(&listener);
}

//void EventGenerator::Notify();
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

void EventGenerator::handleEvent(const AmiMessage &m)
{
	if (m["Event"] == "Newstate")
	{
		switch (std::stoi(m["ChannelState"]))
		{
		case AST_STATE_RINGING:
			if (m["ConnectedLineNum"] == "511")
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
}

void EventListener::OnRing(const AmiMessage &){};
void EventListener::OnOriginate(const AmiMessage &){};
void EventListener::OnDial(const AmiMessage &){};
void EventListener::OnUp(const AmiMessage &){};
void EventListener::OnHangup(const AmiMessage &){};
void EventListener::OnCdr(const AmiMessage &){};

