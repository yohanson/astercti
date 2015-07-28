#include <stdexcept>
#include <iostream>
#include "observer.h"

void IObservable::add(IObserver &observer)
{
	_observers.push_back(&observer);
}

void IObservable::remove(IObserver &observer)
{
	_observers.remove(&observer);
}

void IObservable::Notify(const AmiMessage &message)
{
	for (auto iter : _observers)
	{
		iter->handleEvent(message);
	}
}

//-----

void AmiMessageFilter::handleEvent(const AmiMessage& message)
{
	if (filter(message))
	{
		Notify(message);
	}
}

//-----

MyChanFilter::MyChanFilter(std::string channel)
{
	std::cout << "New filter: " << channel << std::endl;
	m_channel_id = channel;
}

bool MyChanFilter::filter(const AmiMessage &message)
{
	if (message["Event"].empty())
	       return false;

	if (message["Event"] == "Cdr")
	{
		if (message["ChannelID"] == m_channel_id
		 || message["DestinationChannelID"] == m_channel_id)
			return true;
	}
	else if (message["Event"] == "Newstate" || message["Event"] == "Hangup")
	{
		if (message["ChannelID"] == m_channel_id)
			return true;
	}
    return false;
}


