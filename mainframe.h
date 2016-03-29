#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <wx/listctrl.h>
#include "events.h"
#include "taskbaricon.h"
#include "call.h"

class MyFrame: public wxFrame, public IObserver, public ControllerUser, public EventListener
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MyFrame();
    void SetTaskBarIcon(MyTaskBarIcon *);
    void handleEvent(const AmiMessage &);
    void OnOriginate(const AmiMessage &);
    void OnDialIn(const AmiMessage &);
    void OnRing(const AmiMessage &);
    void OnHangup(const AmiMessage &);
    void OnCdr(const AmiMessage &);
    void OnDial(const AmiMessage &);
    void OnUp(const AmiMessage &);
    void OnInternalMessage(const AmiMessage &);

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnDialPressEnter(wxCommandEvent &);
    void OnListResize(wxSizeEvent&);
    void OnListItemSelect(wxListEvent&);
    wxTextCtrl *StatusText;
    wxListCtrl *m_callList;
    wxTextCtrl *m_DialNumber;
    wxStaticText *m_CallInfo;
    MyTaskBarIcon *m_taskbaricon;
};

class CallListItem : public wxListItem, public Call
{

};

enum
{
    ID_Hello = 1,
    ID_TextCtlNumber
};

extern const char *gitcommit;
extern const char *gitcommitdate;
extern const char *builddate;

#endif
