#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <wx/listctrl.h>
#include "events.h"
#include "taskbaricon.h"
#include "call.h"
#include "chanstatus.h"

#define LOG_MAX_LINES 1000
#define LOG_DELETE_LINES 50

class MyFrame: public wxFrame, public IObserver, public ControllerUser, public EventListener, public ChannelStatusPooler
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size, ChannelStatusPool *pool);
    ~MyFrame();
    void SetTaskBarIcon(MyTaskBarIcon *);
    void handleEvent(const AmiMessage &);
    void OnOriginate(const AmiMessage &);
    void OnDialIn(const AmiMessage &);
    void OnHangup(const AmiMessage &);
    void OnCdr(const AmiMessage &);
    void OnDial(const AmiMessage &);
    void OnUp(const AmiMessage &);
    void OnInternalMessage(const AmiMessage &);
    void Log(const wxString &);

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnActivate(wxActivateEvent &event);
    void UpdateDialButtonImage();
    void OnDialPressEnter(wxCommandEvent &);
    void OnListResize(wxSizeEvent&);
    void OnListItemSelect(wxListEvent&);
    bool SaveCalls(const wxString &filename);
    bool LoadCalls(const wxString &filename);
    wxTextCtrl *StatusText;
    wxListCtrl *m_callList;
    wxTextCtrl *m_DialNumber;
    wxBitmapButton *m_DialButton;
    wxStaticText *m_CallInfo;
    MyTaskBarIcon *m_taskbaricon;
    std::string m_current_channel;
    int m_missed_calls;
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
