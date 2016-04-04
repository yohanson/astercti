#ifndef _CHANNELSTATUS_H_
#define _CHANNELSTATUS_H_

#include "observer.h"
#include "asterisk.h"
#include "events.h"

class ChannelName
{
private:
    std::string s;
public:
    operator std::string() const {return s;};
    std::string getID();
    ChannelName(){};
    ChannelName(const std::string& str) {s = str;};
    ChannelName(std::string& str) {s = str;};
    ChannelName(const ChannelName& cn) {s = cn;};
    bool operator== (const ChannelName &cn) {return s == (std::string)cn;};
    //const std::string& operator= (const ChannelName &cn) {return cn.s;};
    //const ChannelName& operator= (const std::string &str) {ChannelName cn(str); return cn;};
};

class Channel
{
public:
    Channel(const std::string& chan){m_channel = chan; m_bridgedTo = NULL;};
    Channel(const ChannelName& chan){m_channel = chan; m_bridgedTo = NULL;};
    enum ast_channel_state m_state;
	ChannelName m_channel;
	std::string m_callerIDNum;
	std::string m_callerIDName;
    Channel *m_bridgedTo;
    std::string getID();
};

class MetaChannel
{
public:
    MetaChannel(const std::string id){m_channelID = id;};
    Channel * findChannel(ChannelName);
    std::string m_channelID;
    std::list<Channel *> m_ownChannels;
};

class ChannelStatusPool : public EventGenerator
{
private:
    std::string m_mychannel;
    std::map<std::string, MetaChannel *> m_channels;
    void NotifyOnCallerInfoAvailable(const AmiMessage&);
public:
    ChannelStatusPool(const std::string &);
    Channel *                                       findChannel(ChannelName);
    std::map<std::string, MetaChannel *>::iterator  findMetaChannel_iter(ChannelName);
    MetaChannel *                                   findMetaChannel(ChannelName);
    void handleEvent(const AmiMessage&);
};

#endif
