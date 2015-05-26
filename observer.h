#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <map>
#include <list>

typedef std::map<std::string, std::string> AmiMessage;

class IObserver
{
public:
	virtual void handleEvent(const AmiMessage&) const = 0;
};

class IObservable
{
	std::list<const IObserver *> _observers;
public:
	void add(IObserver &observer)
	{
		_observers.push_back(&observer);
	}
	void remove(IObserver &observer)
	{
		_observers.remove(&observer);
	}
	void Notify(const AmiMessage &message) const
	{
		for (auto iter : _observers)
		{
			iter->handleEvent(message);
		}
	}

};

class AmiMessageFilter : public IObserver, public IObservable
{
	virtual bool filter(const AmiMessage &message) const = 0;
	void handleEvent(const AmiMessage& message) const
	{
		if (filter(message))
		{
			Notify(message);
		}
	}
};

class MyChanFilter : public AmiMessageFilter
{
	std::string m_channel_id;
public:
	MyChanFilter(std::string channel)
	{
		m_channel_id = channel;
	}
	bool filter(const AmiMessage &message) const
	{
	    try {
	    	if (message.at("ChannelID") == m_channel_id)
			if (message.at("Event") == "Newstate" || message.at("Event") == "Hangup" || message.at("Event") == "Cdr")
				return true;
	    }
	    catch (std::out_of_range)
	    {
	    	return false;
	    }
	    return false;
	}

};

#endif
