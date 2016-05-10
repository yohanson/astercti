#include "chanstatus.h"

void ChannelStatusPool::handleEvent(const AmiMessage &m)
{
    bool changed = false;
    if (m["Event"] == "Newchannel")
    {
        changed = true;
        if (m["Channel"] == "") return;
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
        std::cout << m["Event"] << " " << m["Channel"] << std::endl;
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
            std::cout << "Dial " << m["Channel"] << " → " << m["Destination"] << std::endl;
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
            src_chan->m_bridgedTo = dst_chan;
            dst_chan->m_bridgedTo = src_chan;
            if (dst_chan->getID() == m_mychannel)
            {
                std::cout << "We have a call from " << (std::string)src_chan->m_channel << std::endl;
                std::cout << "ID: " << src_chan->getID() << std::endl;
                MetaChannel *mc = findMetaChannel(src_chan->m_channel.getID());
                std::cout << "MetaChannel: " << mc << std::endl;
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
    if (changed)
    {
        std::cout << "Channels:" << std::endl;
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it)
        {
            std::cout << '\t' << it->second->m_channelID;
            if (it->second->m_ownChannels.size())
            {
                std::cout << ": ";
                for (auto chan = it->second->m_ownChannels.begin(); chan != it->second->m_ownChannels.end(); ++chan)
                {
                    std::cout << (std::string)(*chan)->m_channel << "(" << (*chan)->m_callerIDNum << " " << (*chan)->m_callerIDName << ")";
                    if ((*chan)->m_bridgedTo)
                        std::cout << "[" << (std::string)(*chan)->m_bridgedTo->m_channel << "]";
                    if (chan != std::prev(it->second->m_ownChannels.end()))
                        std::cout << ", ";
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
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
    std::cout << "findMetaChannel(" << (chan) << ") = " << (unsigned long)it->second;
    if (it != m_channels.end())
    {
        std::cout << " " << it->second->m_channelID << std::endl;
        return it->second;
    }
    std::cout << std::endl;
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
    std::cout << "getting Bridged Channels of " << (std::string)channelname.getID() << " (got " << (void *)metachan << ")" << std::endl;
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

