#ifndef _TASKBARICON_H_
#define _TASKBARICON_H_

#include <wx/taskbar.h>

class MyTaskBarIcon : public wxTaskBarIcon, public ControllerUser
{
private:
    wxWindow *m_mainFrame;
public:
    MyTaskBarIcon(wxString);
    MyTaskBarIcon(wxIcon);

    void Init();
    void SetMainFrame(wxWindow *);
    void OnClick(wxTaskBarIconEvent&);
    void OnExit(wxCommandEvent&);
    wxMenu* CreatePopupMenu();
};

#endif
