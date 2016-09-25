#include "chanstatus.h"

void ChannelStatusPool::handleEvent(const AmiMessage &m)
{
    bool changed = false;
    if (m["Event"] == "Newchannel")
    {
        changed = true;
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
        changed = true;
        DEBUG_MSG(m["Event"] << " " << m["Channel"] << std::endl);
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
        changed = true;
        if (m["SubEvent"] == "Begin")
        {
            DEBUG_MSG("Dial " << m["Channel"] << " → " << m["Destination"] << std::endl);
            DEBUG_MSG("------------------------------" << std::endl);
            for (auto i : m)
            {
                DEBUG_MSG(i.first << ": " << i.second << std::endl);
            }
            DEBUG_MSG("------------------------------" << std::endl);
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
            if (dst_chan->getID() == m_mychannel)
            {
                DEBUG_MSG("We have a call from " << (std::string)src_chan->m_channel << std::endl);
                DEBUG_MSG("ID: " << src_chan->getID() << std::endl);
                MetaChannel *mc = findMetaChannel(src_chan->m_channel.getID());
                DEBUG_MSG("MetaChannel: " << mc << std::endl);
                if (mc)
                {
                    for (auto caller_chan : mc->m_ownChannels)
                    {
                        if (caller_chan->m_bridgedTo && caller_chan->m_bridgedTo != dst_chan)
                        {
                            Channel *call_on_hold = caller_chan->m_bridgedTo;
                            std::cout << "Incoming transfer! " << (std::string)call_on_hold->m_channel << ": "
                                << call_on_hold->m_callerIDNum << "(" << call_on_hold->m_callerIDName << ")" << std::endl;
                        }
                    }
                }
            }
        }
    }
    else if (m["Event"] == "Bridge")
    {
        changed = true;
        Channel *channel1 = findChannel(m["Channel1"]);
        Channel *channel2 = findChannel(m["Channel2"]);
        if ( !(channel1 && channel2) )
            return;
        DEBUG_MSG("Bridge between " << m["Channel1"] << " and " << m["Channel2"] << " - " << m["Bridgestate"] << std::endl);
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
    if (changed)
    {
        DEBUG_MSG("Channels:" << std::endl);
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it)
        {
            DEBUG_MSG('\t' << it->second->m_channelID);
            if (it->second->m_ownChannels.size())
            {
                DEBUG_MSG(": ");
                for (auto chan = it->second->m_ownChannels.begin(); chan != it->second->m_ownChannels.end(); ++chan)
                {
                    DEBUG_MSG((std::string)(*chan)->m_channel << "(" << (*chan)->m_callerIDNum << " " << (*chan)->m_callerIDName << ")");
                    if ((*chan)->m_bridgedTo)
                        DEBUG_MSG("[" << (std::string)(*chan)->m_bridgedTo->m_channel << "]");
                    if (chan != std::prev(it->second->m_ownChannels.end()))
                        DEBUG_MSG(", ");
                }
            }
            DEBUG_MSG(std::endl);
        }
        DEBUG_MSG(std::endl);
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

Channel* MetaChannel::findChannel(ChannelName name)
{
    for (auto it = m_ownChannels.begin(); it != m_ownChannels.end(); ++it)
    {
        if ((*it)->m_channel == name)
            return *it;
    }
    return NULL;
}

Channel* ChannelStatusPool::findChannel(ChannelName chan)
{
    MetaChannel *mc = findMetaChannel(chan.getID());
    if (mc)
    {
        return mc->findChannel(chan);
    }
    return NULL;
}

std::map<std::string, MetaChannel *>::iterator ChannelStatusPool::findMetaChannel_iter(ChannelName chan)
{
    return m_channels.find(chan.getID());
}

MetaChannel *ChannelStatusPool::findMetaChannel(const std::string &chan)
{
    auto it = m_channels.find(chan);
    DEBUG_MSG("findMetaChannel(" << (chan) << ") = " << (unsigned long)it->second);
    if (it != m_channels.end())
    {
        DEBUG_MSG(" " << it->second->m_channelID << std::endl);
        return it->second;
    }
    DEBUG_MSG(std::endl);
    return NULL;
}

ChannelStatusPool::ChannelStatusPool(const std::string &mychannel)
{
    m_mychannel = mychannel;
}

std::list<Channel *> ChannelStatusPool::getBridgedChannelsOf(const ChannelName &channelname)
{
    std::list<Channel *> peers;
    MetaChannel *metachan = findMetaChannel(channelname.getID());
    DEBUG_MSG("getting Bridged Channels of " << (std::string)channelname.getID() << " (got " << (void *)metachan << ")" << std::endl);
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

