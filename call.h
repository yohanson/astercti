#ifndef _CALL_H_
#define _CALL_H_

class Call
{
public:
	enum Direction {CALL_IN, CALL_OUT};
	enum Disposition {CALL_ANSWERED, CALL_UNANSWERED};
	Call(){m_duration=0; m_unique_id=0;};
	void		SetNumber(wxString s){m_number = s;};
	wxString	GetNumber(){return m_number;};
	void		SetName(wxString s){m_name = s;};
	wxString	GetName(){return m_name;};
	long		GetUniqueID(){return m_unique_id;};
	void		SetUniqueID(long id){m_unique_id = id;};
	void		SetSecondChannelID(wxString s){m_secondchannelid = s;};
	wxString	GetSecondChannelID(){return m_secondchannelid;};
	void		SetTime(wxDateTime t){m_time = t;};
	wxDateTime	GetTime(){return m_time;};
	void		SetDuration(int d){m_duration = d;};
	int	        GetDuration(){return m_duration;};
	void		SetDirection(Direction d){m_direction = d;};
	Direction	GetDirection(){return m_direction;};
private:
    wxString m_number;
	wxString m_name;
	wxString m_description;
    wxString m_secondchannelid;
	long m_unique_id;
	wxDateTime m_time;
	long m_duration;
	Direction m_direction;
	Disposition m_disposition;
};

#endif
