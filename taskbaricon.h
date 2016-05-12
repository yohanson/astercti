#ifndef _TASKBARICON_H_
#define _TASKBARICON_H_

#include <wx/taskbar.h>

class MyTaskBarIcon : public wxTaskBarIcon, public ControllerUser
{
private:
    wxFrame *m_mainFrame;
    wxIcon* m_defaultIcon;
    wxIcon* m_missedIcon;
    wxString tooltip_base;
public:
    MyTaskBarIcon(wxString, wxString, wxString);
    MyTaskBarIcon(wxIcon, wxIcon, wxString);

    void Init();
    void SetMainFrame(wxFrame *);
    void SetMissedCalls(int);
    void OnClick(wxTaskBarIconEvent&);
    void OnExit(wxCommandEvent&);
    wxMenu* CreatePopupMenu();
};

#endif
