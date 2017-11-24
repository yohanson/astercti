#include "chanstatus.h"

void ChannelStatusPool::handleEvent(const AmiMessage &m)
{
    if (m["Event"] == "Newchannel")
    {
        if (m["Channel"] == "")
        {
            std::cerr << "Empty channel" << std::endl;
            return;
        }
        Channel *chan = new Channel(m["Channel"]);
        chan->m_callerIDNum = m["CallerIDNum"];
        chan->m_callerIDName = m["CallerIDName"];
        auto metachan_iter = m_channels.find(chan->getID());
        MetaChannel *metachan;
        if (metachan_iter == m_channels.end())
        {
            metachan = new MetaChannel(chan->getID());
            m_channels.insert(std::pair<std::string, MetaChannel *>(metachan->m_channelID, metachan));
        }
        else metachan = metachan_iter->second;
        metachan->m_ownChannels.push_back(chan);
    }
    else if (m["Event"] == "Hangup")
    {
        std::string channel = m["Channel"];
        size_t zombie = channel.find("<ZOMBIE>");
        if (zombie != std::string::npos)
        channel = channel.substr(0, zombie);
        auto mchan_it = findMetaChannel_iter(channel);
        if (mchan_it == m_channels.end())
            return;
        Channel *chan = mchan_it->second->findChannel(channel);
        if (!chan)
            return;
        if (chan->m_bridgedTo && chan->m_bridgedTo->m_bridgedTo)
            chan->m_bridgedTo->m_bridgedTo = NULL;
        mchan_it->second->m_ownChannels.remove(chan);
        if (mchan_it->second->m_ownChannels.empty())
        {
            m_channels.erase(mchan_it);
        }
    }
    else if (m["Event"] == "Dial")
    {
        if (m["SubEvent"] == "Begin")
        {
            auto src_mchan_it = findMetaChannel_iter(m["Channel"]);
            if (src_mchan_it == m_channels.end())
                return;
            Channel *src_chan = src_mchan_it->second->findChannel(m["Channel"]);
            if (!src_chan)
                return;

            auto dst_mchan_it = findMetaChannel_iter(m["Destination"]);
            if (dst_mchan_it == m_channels.end())
                return;
            Channel *dst_chan = dst_mchan_it->second->findChannel(m["Destination"]);
            if (!dst_chan)
                return;
#ifdef DEBUG
            if (dst_chan->getID() == m_mychannel)
            {
                MetaChannel *mc = findMetaChannel(src_chan->m_channel.getID());
                if (mc)
                {
                    for (auto caller_chan : mc->m_ownChannels)
                    {
                        if (caller_chan->m_bridgedTo && caller_chan->m_bridgedTo != dst_chan)
                        {
                            Channel *call_on_hold = caller_chan->m_bridgedTo;
                            DEBUG_MSG("Incoming transfer! " << (std::string)call_on_hold->m_channel << ": "
                                << call_on_hold->m_callerIDNum << "(" << call_on_hold->m_callerIDName << ")" << std::endl);
                        }
                    }
                }
            }
#endif //DEBUG
        }
    }
    else if (m["Event"] == "Bridge")
    {
        Channel *channel1 = findChannel(m["Channel1"]);
        Channel *channel2 = findChannel(m["Channel2"]);
        if ( !(channel1 && channel2) )
            return;
        if (m["Bridgestate"] == "Link")
        {
            channel1->m_bridgedTo = channel2;
            channel2->m_bridgedTo = channel1;
        }
        else if (m["Bridgestate"] == "Unlink")
        {
            channel1->m_bridgedTo = NULL;
            channel2->m_bridgedTo = NULL;
        }
    }
}

const std::string ChannelName::getID() const
{
    size_t dash = s.find('-');
    if (dash != std::string::npos)
    {
        return s.substr(0, dash);
    }
    return "";
}

const std::string Channel::getID() const
{
    return m_channel.getID();
}

Channel* MetaChannel::findChannel(const ChannelName &name)
{
    for (auto it = m_ownChannels.begin(); it != m_ownChannels.end(); ++it)
    {
        if ((*it)->m_channel == name)
            return *it;
    }
    return NULL;
}

Channel* ChannelStatusPool::findChannel(const ChannelName &chan)
{
    MetaChannel *mc = findMetaChannel(chan.getID());
    if (mc)
    {
        return mc->findChannel(chan);
    }
    return NULL;
}

std::map<std::string, MetaChannel *>::iterator ChannelStatusPool::findMetaChannel_iter(const ChannelName &chan)
{
    return m_channels.find(chan.getID());
}

MetaChannel *ChannelStatusPool::findMetaChannel(const std::string &chan)
{
    auto it = m_channels.find(chan);
    if (it != m_channels.end())
    {
        return it->second;
    }
    return NULL;
}

ChannelStatusPool::ChannelStatusPool(const std::string &mychannel)
    : m_mychannel(mychannel)
{
}

std::list<Channel *> ChannelStatusPool::getBridgedChannelsOf(const ChannelName &channelname)
{
    std::list<Channel *> peers;
    MetaChannel *metachan = findMetaChannel(channelname.getID());
    if (!metachan)
        return peers;
    for (auto chan : metachan->m_ownChannels)
    {
        if (chan->m_bridgedTo && chan->m_bridgedTo->getID() != m_mychannel)
        {
            peers.push_back(chan);
        }
    }
    return peers;
}

ChannelStatusPooler::ChannelStatusPooler(ChannelStatusPool *pool)
{
    m_channelstatuspool = pool;
}

Channel::Channel(const std::string& chan)
    : m_state(AST_STATE_DOWN)
{
    if (chan == "")
    {
        const char *emptychan = "Empty channel name";
        std::cerr << emptychan;
        throw std::invalid_argument(emptychan);
    }
    m_channel = chan;
    m_bridgedTo = NULL;
};

