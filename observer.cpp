#include <stdexcept>
#include <iostream>
#include "observer.h"

IObserver::IObserver()
{
    observer_descr="";
}

IObserver::~IObserver()
{
    for (auto observable : m_observables)
    {
        observable->remove(*this);
    }
}

void IObserver::listens_to(IObservable &o)
{
    m_observables.push_back(&o);
}

//----

void IObservable::add(IObserver &observer)
{
	_observers.push_back(&observer);
    observer.listens_to(*this);
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
    else if (message["Event"] == "Dial")
    {
        if (message["DestinationChannelID"] == m_channel_id)
            return true;
    }
    return false;
}

bool InternalMessageFilter::filter(const AmiMessage &message)
{
	if (message["InternalMessage"] != "")
		return true;
	return false;
}

