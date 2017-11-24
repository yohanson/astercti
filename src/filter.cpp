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

//== ShortenNumberModifier ==
ShortenNumberModifier::ShortenNumberModifier(const std::string &ShorteningRules)
{
    if (!ShorteningRules.empty())
    {
        if (!CreateShorteningRules(ShorteningRules))
        {
            std::cerr << "Syntax error in replace_number_prefix, disabling shortening" << std::endl;
            long_prefixes.clear();
            short_prefixes.clear();
        }
    }
}

bool ShortenNumberModifier::CreateShorteningRules(const std::string &rules)
{
    const static char RuleBoundary = '/';
    size_t start, pos = 0;
    std::string long_prefix, short_prefix;
    while (std::string::npos != (start = rules.find(RuleBoundary, pos)))
    {
        size_t middle, end;
        middle = rules.find(RuleBoundary, start+1);
        if (middle == std::string::npos) return false;
        end = rules.find(RuleBoundary, middle+1);
        if (end == std::string::npos) return false;
        long_prefix = rules.substr(start+1, middle-(start+1));
        short_prefix = rules.substr(middle+1, end-(middle+1));
        long_prefixes.push_back(long_prefix);
        short_prefixes.push_back(short_prefix);
        pos = end+1;
    }
    return long_prefixes.size() == short_prefixes.size();
}

void ShortenNumberModifier::shorten(std::string &number)
{
    for (int i=0; i<long_prefixes.size(); i++)
    {
        if (0 == number.compare(0, long_prefixes[i].length(), long_prefixes[i])) // if at the beginning
        {
            number.replace(0, long_prefixes[i].length(), short_prefixes[i]);
        }
    }
}

void ShortenNumberModifier::handleEvent(const AmiMessage &m)
{
    if (long_prefixes.size() && !m["CallerIDNum"].empty())
    {
        for (auto long_prefix : long_prefixes)
        {
            if (m["CallerIDNum"].find(long_prefix) != std::string::npos)
            {
                AmiMessage mangled(m);
                shorten(mangled["CallerIDNum"]);
                Notify(mangled);
                return;
            }
        }
    }
    Notify(m);
}
