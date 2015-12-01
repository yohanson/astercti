#ifndef _TASKBARICON_H_
#define _TASKBARICON_H_

#include <wx/taskbar.h>

class MyTaskBarIcon : public wxTaskBarIcon, public ControllerUser
{
private:
    wxFrame *m_mainFrame;
public:
    MyTaskBarIcon(wxString, wxString);
    MyTaskBarIcon(wxIcon, wxString);

    void Init();
    void SetMainFrame(wxFrame *);
    void OnClick(wxTaskBarIconEvent&);
    void OnExit(wxCommandEvent&);
    wxMenu* CreatePopupMenu();
};

#endif
