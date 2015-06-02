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
    try {
	if (message.at("Event") == "Cdr")
	{
		if (message.at("ChannelID") == m_channel_id
		 || message.at("DestinationChannelID") == m_channel_id)
			return true;
	}
	else if (message.at("Event") == "Newstate" || message.at("Event") == "Hangup")
	{
		if (message.at("ChannelID") == m_channel_id)
			return true;
	}
    }
    catch (std::out_of_range)
    {
	return false;
    }
    return false;
}


