#ifndef _CHANNELSTATUS_H_
#define _CHANNELSTATUS_H_

/*
 * The scheme:
 *
 * ChannelStatusPool
 *   - MetaChannel SIP/100
 *       - Channel SIP/100-1 → (peer pointer) → IAX/102-1
 *
 *   - MetaChannel IAX/rinet
 *       - Channel IAX/102-1 → (peer pointer) → SIP/100-1
 *
 */


#include "observer.h"
#include "asterisk.h"
#include "events.h"

#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif


class ChannelName
{
private:
    std::string s;
public:
    operator std::string() const {return s;};
    const std::string getID() const;
    ChannelName(){};
    ChannelName(const std::string& str) : s(str) {};
    ChannelName(std::string& str) : s(str) {};
    ChannelName(const ChannelName& cn) : s(cn) {};
    bool operator== (const ChannelName &cn) {return s == (std::string)cn;};
};

class Channel
{
public:
    explicit Channel(const std::string& chan);
    enum ast_channel_state m_state;
    ChannelName m_channel;
    std::string m_callerIDNum;
    std::string m_callerIDName;
    Channel *m_bridgedTo;
    const std::string getID() const;
};

class MetaChannel
{
public:
    explicit MetaChannel(const std::string &id) : m_channelID(id) {};
    Channel * findChannel(const ChannelName &);
    std::string m_channelID;
    std::list<Channel *> m_ownChannels;
};

class ChannelStatusPool : public IObserver
{
private:
    std::string m_mychannel;
    std::map<std::string, MetaChannel *> m_channels;
public:
    explicit ChannelStatusPool(const std::string &);
    Channel *                                       findChannel(const ChannelName&);
    std::map<std::string, MetaChannel *>::iterator  findMetaChannel_iter(const ChannelName &);
    MetaChannel *                                   findMetaChannel(const std::string &);
    void handleEvent(const AmiMessage&);
    std::list<Channel *> getBridgedChannelsOf(const ChannelName &channelname);
};

class ChannelStatusPooler
{
protected:
    ChannelStatusPool *m_channelstatuspool;
public:
    explicit ChannelStatusPooler(ChannelStatusPool *);
};

#endif
