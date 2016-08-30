#include <wx/wx.h>
#include <vector>
#include <sstream>
#include "call.h"

Call::Call()
{
    m_duration = 0;
    m_unique_id = 0;
    m_direction = CALL_IN;
    m_disposition = CALL_ANSWERED;
}

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
    size_t pos = 0;
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

