#ifndef _CHANNELSTATUS_H_
#define _CHANNELSTATUS_H_

#include "observer.h"
#include "asterisk.h"

class ChannelStatus
{
public:
	enum ast_channel_state m_state;
	std::string m_ChannelID;
	std::string m_Channel;
	std::string m_CallerIDNum;
	std::string m_CallerIDName;
    std::list<std::string> m_peerChannels;
};

class ChannelStatusPool : public IObserver
{
private:
    std::map<std::string, ChannelStatus *> m_channels;
public:
    void handleEvent(const AmiMessage&);
};

#endif
