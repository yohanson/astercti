#include "chanstatus.h"

void ChannelStatusPool::handleEvent(const AmiMessage &m)
{
    bool changed = false;
    if (m["Event"] == "Newchannel")
    {
        changed = true;
        ChannelStatus *chan = new ChannelStatus;
        chan->m_Channel = m["Channel"];
        m_channels.insert( std::pair<std::string, ChannelStatus *>(m["Channel"], chan));
    }
    else if (m["Event"] == "Hangup")
    {
        changed = true;
        std::cout << m["Event"] << " " << m["Channel"] << std::endl;
        std::string channel = m["Channel"];
        size_t zombie = channel.find("<ZOMBIE>");
        if (zombie != std::string::npos)
		channel = channel.substr(0, zombie);
        auto it = m_channels.find(channel);
        if (it == m_channels.end())
            return;
        m_channels.erase(it);
        for (auto chan = m_channels.begin(); chan != m_channels.end(); ++chan)
        {
            for (auto peerChan = chan->second->m_peerChannels.begin(); peerChan != chan->second->m_peerChannels.end(); ++peerChan)
            {
                if (*peerChan == channel)
                {
                    chan->second->m_peerChannels.erase(peerChan);
                    break;
                }
            }
        }
    }
    else if (m["Event"] == "Dial")
    {
        changed = true;
        if (m["SubEvent"] == "Begin")
        {
            std::cout << "Dial " << m["Channel"] << " → " << m["Destination"] << std::endl;
            m_channels[m["Channel"]]->m_peerChannels.push_back(m["Destination"]);
        }
    }
    if (changed)
    {
        std::cout << "Channels: ";
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it)
        {
            std::cout << it->second->m_Channel;
            if (it->second->m_peerChannels.size())
            {
                std::cout << "(";
                for (auto peerChan = it->second->m_peerChannels.begin(); peerChan != it->second->m_peerChannels.end(); ++peerChan)
                {
                    std::cout << *peerChan;
                    if (peerChan != std::prev(it->second->m_peerChannels.end()))
                        std::cout << ", ";
                }
                std::cout << ")";
            }
            if (it != std::prev(m_channels.end()))
                std::cout << ", ";
        }
        std::cout << std::endl;
    }
}
