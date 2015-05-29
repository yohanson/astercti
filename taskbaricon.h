#ifndef _TASKBARICON_H_
#define _TASKBARICON_H_

#include <wx/taskbar.h>

class MyTaskBarIcon : public wxTaskBarIcon, public ControllerUser
{
private:
    wxWindow *m_parent;
public:
    MyTaskBarIcon();

    void OnLeftButtonDClick(wxTaskBarIconEvent&);
    void OnExit(wxCommandEvent&);
    wxMenu* CreatePopupMenu();
};

#endif
