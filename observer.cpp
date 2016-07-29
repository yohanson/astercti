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


