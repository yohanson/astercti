#include <stdexcept>
#include <iostream>
#include "observer.h"

IObserver::IObserver()
    : observer_descr("")
{
}

IObserver::~IObserver()
{
    for (auto observable : m_observables)
    {
        observable->no_broadcast(*this, false);
    }
}

void IObserver::subscribe(IObservable &o)
{
    m_observables.push_back(&o);
}

void IObserver::unsubscribe(IObservable &o)
{
    m_observables.remove(&o);
}

//----

void IObservable::broadcast(IObserver &observer)
{
    _observers.push_back(&observer);
    observer.subscribe(*this);
}

void IObservable::no_broadcast(IObserver &observer, bool both_ends)
{
    _observers.remove(&observer);
    if (both_ends)
        observer.unsubscribe(*this);
}

void IObservable::Notify(const AmiMessage &message)
{
    for (auto iter : _observers)
    {
        iter->handleEvent(message);
    }
}

IObservable::~IObservable()
{
    for (auto observer : _observers)
    {
        observer->unsubscribe(*this);
    }
}
