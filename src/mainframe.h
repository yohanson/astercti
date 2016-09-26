#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <wx/html/htmlwin.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include "events.h"
#include "taskbaricon.h"
#include "calllistctrl.h"
#include "call.h"
#include "chanstatus.h"
#include "lookup.h"

#define LOG_MAX_LINES 1000
#define LOG_DELETE_LINES 50

class MyFrame: public wxFrame, public IObserver, public EventListener, public ChannelStatusPooler
{
public:
    MyFrame(const wxString& title,
            const wxPoint& pos,
            const wxSize& size,
            ChannelStatusPool *pool,
            Asterisk *a,
            CallerInfoLookuper *lookuper = NULL,
            const std::string &lookup_field = "clients/0/name");
    ~MyFrame();
    void SetTaskBarIcon(MyTaskBarIcon *);
    void handleEvent(const AmiMessage &);
    void OnOriginate(const AmiMessage &);
    void OnDialIn(const AmiMessage &);
    void OnHangup(const AmiMessage &);
    void OnCdr(const AmiMessage &);
    void OnResponse(const AmiMessage &);
    void OnDial(const AmiMessage &);
    void OnUp(const AmiMessage &);
    void OnInternalMessage(const AmiMessage &);
    void Log(const wxString &);

private:
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnActivate(wxActivateEvent &event);
    void OnLinkClicked(wxHtmlLinkEvent& event);
    void UpdateDialButtonImage();
    void OnDialPressEnter(wxCommandEvent &);
    void OnDialTextChange(wxCommandEvent &);
    void OnListResize(wxSizeEvent&);
    void OnListItemSelect(wxListEvent&);
    void SavePosition();
    bool SaveCalls(const wxString &filename);
    bool LoadCalls(const wxString &filename);
    wxTextCtrl *StatusText;
    CallListCtrl *m_callList;
    wxTextCtrl *m_DialNumber;
    wxBitmapButton *m_DialButton;
    wxHtmlWindow *m_CallInfo;
    MyTaskBarIcon *m_taskbaricon;
    wxSplitterWindow TopMostVerticalSplitter;
    std::string m_current_channel;
    wxBitmap       m_dialIcon,
                   m_hangupIcon;
    int m_missed_calls;
    Asterisk *asterisk;
    CallerInfoLookuper *m_lookuper;
    std::string m_lookup_field;
};

class CallListItem : public wxListItem, public Call
{

};

enum
{
    ID_TextCtlNumber = 1
};

extern const char *gitcommit;
extern const char *gitcommitdate;
extern const char *builddate;

#endif
