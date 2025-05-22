#include "filter.h"
#include <iostream>

//== AmiMessageFilter ========
void AmiMessageFilter::handleEvent(const AmiMessage& message)
{
    if (filter(message))
    {
       Notify(message);
    }
}

//== MyChanFilter ===========
MyChanFilter::MyChanFilter(const std::string& channel)
    : m_channel_id(channel)
{
}

bool MyChanFilter::filter(const AmiMessage &message)
{
    if (message["Response"] == "Error")
        return true;
    if (message["Event"].empty())
        return false;

    if (message["Event"] == "Cdr")
    {
        if (message["ChannelID"] == m_channel_id
         || message["DestinationChannelID"] == m_channel_id)
            return true;
    }
    else if (message["Event"] == "Newstate" || message["Event"] == "Hangup")
    {
        if (message["ChannelID"] == m_channel_id)
            return true;
    }
    else if (message["Event"] == "Dial")
    {
        if (message["DestinationChannelID"] == m_channel_id)
            return true;
    }
    return false;
}

//== InternalMessageFilter ===
bool InternalMessageFilter::filter(const AmiMessage &message)
{
    if (message["InternalMessage"] != "")
        return true;
    return false;
}
