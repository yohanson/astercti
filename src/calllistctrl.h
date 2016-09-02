#ifndef _CALLLISTCTRL_H_
#define _CALLLISTCTRL_H_

#include <wx/listctrl.h>
#include "call.h"

class CallListCtrl: public wxListCtrl
{
public:
    CallListCtrl(wxWindow *  parent,
                 wxWindowID  id,
                 const wxPoint &        pos = wxDefaultPosition,
                 const wxSize &         size = wxDefaultSize,
                 long                   style = wxLC_ICON,
                 const wxValidator &    validator = wxDefaultValidator,
                 const wxString &       name = wxListCtrlNameStr,
                 const wxString &       timeFormat = wxDefaultDateTimeFormat
    );
    ~CallListCtrl();
    long InsertCallItem(Call *call, long index = 0);
    void UpdateItem(long index);
    void SetTimeFormat(const wxString & timeFormat);
    void OnResize(wxSizeEvent &event);
private:
    wxString m_timeFormat;
    wxColour secondcolour;
};

#endif
