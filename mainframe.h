#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <wx/listctrl.h>

class MyFrame: public wxFrame, public IObserver, public ControllerUser
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MyFrame();
    void handleEvent(const AmiMessage &message);

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnDialPressEnter(wxCommandEvent &);
    wxTextCtrl *StatusText;
    wxListCtrl *m_callList;
    wxTextCtrl *m_DialNumber;
};

enum
{
    ID_Hello = 1,
    ID_TextCtlNumber
};

#endif
