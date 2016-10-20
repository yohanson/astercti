#ifndef _EXECUTE_H_
#define _EXECUTE_H_

#include "events.h"
#include "lookup.h"

#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif


class Executer : public EventListener
{
public:
    Executer(CallerInfoLookuper *lookuper);
    void OnDialIn(const AmiMessage &);
private:
    void ExecCommand(wxString &cmd, wxArrayString &output);
    CallerInfoLookuper *m_lookuper;
    wxString m_cmd;
};


#endif
