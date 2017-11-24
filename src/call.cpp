#include <wx/wx.h>
#include <vector>
#include <sstream>
#include "call.h"

Call::Call()
  : m_duration(0),
    m_unique_id(0),
    m_direction(CALL_IN),
    m_disposition(CALL_ANSWERED)
{
}

void Call::SetNumber(wxString s)
{
    m_number = s;
};

wxString Call::GetNumber()
{
    return m_number;
};

void Call::SetName(wxString s)
{
    m_name = s;
};

wxString Call::GetName()
{
    return m_name;
};

long Call::GetUniqueID()
{
    return m_unique_id;
};

void Call::SetUniqueID(long id)
{
    m_unique_id = id;
};

void Call::SetSecondChannelID(wxString s)
{
    m_secondchannelid = s;
};

wxString Call::GetSecondChannelID()
{
    return m_secondchannelid;
};

void Call::SetTimeStart(wxDateTime t)
{
    m_time_start = t;
};

wxDateTime Call::GetTimeStart()
{
    return m_time_start;
};

void Call::SetTimeAnswer(wxDateTime t)
{
    m_time_answer = t;
};

wxDateTime Call::GetTimeAnswer()
{
    return m_time_answer;
};

void Call::SetTimeEnd(wxDateTime t)
{
    m_time_end = t;
};

wxDateTime Call::GetTimeEnd()
{
    return m_time_end;
};

void Call::SetDuration(int d)
{
    m_duration = d;
};

int Call::GetDuration()
{
    return m_duration;
};

void Call::SetDirection(Direction d)
{
    m_direction = d;
};

Call::Direction Call::GetDirection()
{
    return m_direction;
};

void Call::SetDisposition(Disposition d)
{
    m_disposition = d;
};

Call::Disposition Call::GetDisposition()
{
    return m_disposition;
};

void Call::SetDescription(wxString s)
{
    m_description = s;
};

wxString Call::GetDescription()
{
    return m_description;
};


wxString Call::Serialize()
{
    wxString s;
    s   << (m_time_start.IsValid() ? m_time_start.GetTicks() : -1) << SERIAL_DELIM
        << (m_time_answer.IsValid() ? m_time_answer.GetTicks() : -1) << SERIAL_DELIM
        << (m_time_end.IsValid() ? m_time_end.GetTicks() : -1) << SERIAL_DELIM
        << m_number << SERIAL_DELIM
        << m_name << SERIAL_DELIM
        << m_secondchannelid << SERIAL_DELIM
        << m_unique_id << SERIAL_DELIM
        << m_duration << SERIAL_DELIM
        << m_direction << SERIAL_DELIM
        << m_disposition << SERIAL_DELIM
        << m_description << SERIAL_DELIM;
    s.Replace("\n", "\\n");
    return s;
}

Call::Call(wxString serialized)
{
    serialized.Replace("\\n", "\n");
    std::stringstream s(serialized.ToStdString());
    std::string token;
    std::vector<std::string> els;
    while (std::getline(s, token, SERIAL_DELIM))
    {
        els.push_back(token);
    }
    auto it = els.begin();

    m_time_start.Set((time_t)stoi(*it++));
    m_time_answer.Set((time_t)stoi(*it++));
    m_time_end.Set((time_t)stoi(*it++));
    m_number = *it++;
    m_name = *it++;
    m_secondchannelid = *it++;
    m_unique_id = stoi(*it++);
    m_duration = stoi(*it++);
    m_direction = static_cast<Call::Direction>(stoi(*it++));
    m_disposition = static_cast<Call::Disposition>(stoi(*it++));
    m_description = *it;
}

