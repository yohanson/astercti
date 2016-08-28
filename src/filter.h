#ifndef _FILTER_H_
#define _FILTER_H_

#include <vector>
#include "observer.h"

class AmiMessageFilter : public IObserver, public IObservable
{
    virtual bool filter(const AmiMessage &message) = 0;
    void handleEvent(const AmiMessage& message);
};

class AmiMessageModifier : public IObserver, public IObservable
{
    void handleEvent(const AmiMessage& m) = 0;
};

class MyChanFilter : public AmiMessageFilter
{
    std::string m_channel_id;
public:
    explicit MyChanFilter(const std::string& channel);
    bool filter(const AmiMessage &message);
    bool CreateShorteningRules(const std::string &rules);
    void shorten(std::string &number);
};

class InternalMessageFilter : public AmiMessageFilter
{
public:
    bool filter(const AmiMessage &message);
};

#endif
