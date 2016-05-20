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
	void		SetNumber(wxString s){m_number = s;};
	wxString	GetNumber(){return m_number;};
	void		SetName(wxString s){m_name = s;};
	wxString	GetName(){return m_name;};
	long		GetUniqueID(){return m_unique_id;};
	void		SetUniqueID(long id){m_unique_id = id;};
	void		SetSecondChannelID(wxString s){m_secondchannelid = s;};
	wxString	GetSecondChannelID(){return m_secondchannelid;};
	void		SetTimeStart(wxDateTime t){m_time_start = t;};
	wxDateTime	GetTimeStart(){return m_time_start;};
	void		SetTimeAnswer(wxDateTime t){m_time_answer = t;};
	wxDateTime	GetTimeAnswer(){return m_time_answer;};
	void		SetTimeEnd(wxDateTime t){m_time_end = t;};
	wxDateTime	GetTimeEnd(){return m_time_end;};
	void		SetDuration(int d){m_duration = d;};
	int	        GetDuration(){return m_duration;};
	void		SetDirection(Direction d){m_direction = d;};
	Direction	GetDirection(){return m_direction;};
   	void		SetDisposition(Disposition d){m_disposition = d;};
	Disposition	GetDisposition(){return m_disposition;};
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
