#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <map>
#include <list>
#include "amimessage.h"

class IObservable;
class IObserver
{
private:
    std::list<IObservable *> m_observables;
public:
    std::string observer_descr;
    IObserver();
    ~IObserver();
    void listens_to(IObservable &);
    virtual void handleEvent(const AmiMessage&) = 0;
};

class IObservable
{
	std::list<IObserver *> _observers;
public:
    std::string observable_descr;
	void add(IObserver &observer);
	void remove(IObserver &observer);
	void Notify(const AmiMessage &message);
};

#endif
