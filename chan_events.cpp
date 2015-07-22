#include "chan_events.h"

void EventGenerator::add(EventListener &);
void EventGenerator::remove(EventListener &);
void EventGenerator::Notify();

void EventGenerator::handleEvent(const AmiMessage &m)
{
	if (m["Event"] == "Newstate")
	{
		switch (atoi(m["ChannelState"]))
		{
		case AST_STATE_RINGING:
			
			break;
		}
	}
}

void EventListener::OnRing();
void EventListener::OnOriginate();
void EventListener::OnDial();
void EventListener::OnUp();
void EventListener::OnHangup();
void EventListener::OnCdr();

