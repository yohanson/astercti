#ifndef NOTIFICATIONFRAME_H
#define NOTIFICATIONFRAME_H

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/html/htmlwin.h>
#include <wx/timer.h>

#include "observer.h"
#include "events.h"
#include "call.h"
#include "chanstatus.h"
#include "lookup.h"

class notificationFrame: public wxFrame, public EventListener, public ChannelStatusPooler
{
    public:
        notificationFrame(wxWindow* parent, ChannelStatusPool *pool, Asterisk *a, CallerInfoLookuper *lookuper = NULL);
        virtual ~notificationFrame();
        bool AcceptsFocus() { return false; }
        bool AcceptsFocusFromKeyboard() { return false; }
        bool AcceptsFocusRecursively() { return false; }
        void SetText(wxString const&s){SetHtml(s);}
        void SetHtml(const wxString &s);
        void UpdateSize();
        void SetLookupCmd(std::string);
        void SetLookupUrl(std::string);
        wxString Lookup(std::string);
        wxButton* Button1;
        wxHtmlWindow* HtmlWindow1;
        wxBoxSizer* BoxSizer1;

        void OnOriginate(const AmiMessage &);
        void OnDialIn(const AmiMessage &);
        void OnHangup(const AmiMessage &);
        void OnDial(const AmiMessage &);
        void OnUp(const AmiMessage &);

    protected:
        static const long ID_HTMLWINDOW1;
        static const long ID_BUTTON1;

    private:
        void OnPaint(wxPaintEvent& event);
        void OnButton1Click(wxCommandEvent& event);
        void OnTimer1Trigger(wxTimerEvent& event);
        void OnLinkClicked(wxHtmlLinkEvent& event);
        void OnHideButtonClick(wxCommandEvent& event);
        void OnHideTimer(wxTimerEvent& event);
        wxTimer *m_hidetimer;
        std::string m_current_channel;
        CallerInfoLookuper *m_lookuper;
        Asterisk *asterisk;

        Call m_call;
};

#endif
