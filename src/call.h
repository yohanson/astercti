#ifndef _CALL_H_
#define _CALL_H_

enum CALLSTATUS {
    INCOMING_ANSWERED,
    INCOMING_UNANSWERED,
    OUTBOUND_ANSWERED,
    OUTBOUND_UNANSWERED,
    INCOMING_ANSWERED_ELSEWHERE
};

#define SERIAL_DELIM '|'

class Call
{
public:
    enum Direction {CALL_IN, CALL_OUT};
    enum Disposition {CALL_UNANSWERED, CALL_ANSWERED, CALL_ANSWERED_ELSEWHERE};
    Call();
    Call(wxString serialized);
    void        SetNumber(wxString s);
    wxString    GetNumber();
    void        SetName(wxString s);
    wxString    GetName();
    long        GetUniqueID();
    void        SetUniqueID(long id);
    void        SetSecondChannelID(wxString s);
    wxString    GetSecondChannelID();
    void        SetTimeStart(wxDateTime t);
    wxDateTime  GetTimeStart();
    void        SetTimeAnswer(wxDateTime t);
    wxDateTime  GetTimeAnswer();
    void        SetTimeEnd(wxDateTime t);
    wxDateTime  GetTimeEnd();
    void        SetDuration(int d);
    int         GetDuration();
    void        SetDirection(Direction d);
    Direction   GetDirection();
    void        SetDisposition(Disposition d);
    Disposition GetDisposition();
    void        SetDescription(wxString s);
    wxString    GetDescription();
    wxString    Serialize();
private:
    wxString m_number;
    wxString m_name;
    wxString m_description;
    wxString m_secondchannelid;
    long m_unique_id;
    wxDateTime m_time_start;
    wxDateTime m_time_answer;
    wxDateTime m_time_end;
    long m_duration;
    Direction m_direction;
    Disposition m_disposition;
};

#endif
