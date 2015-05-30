#ifndef NOTIFICATIONFRAME_H
#define NOTIFICATIONFRAME_H

//#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/html/htmlwin.h>
#include <wx/timer.h>

#include "observer.h"
#include "controller.h"

class notificationFrame: public wxFrame, public IObserver, public ControllerUser
{
	public:

		notificationFrame(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~notificationFrame();
		bool AcceptsFocus() { return false; }
		bool AcceptsFocusFromKeyboard() { return false; }
		bool AcceptsFocusRecursively() { return false; }
		void SetText(wxString const&s){SetHtml(s);}
		void SetHtml(const wxString &s);
		void UpdateSize();
		void SetLookupCmd(std::string);

		//(*Declarations(notificationFrame)
		wxButton* Button1;
		wxHtmlWindow* HtmlWindow1;
		wxBoxSizer* BoxSizer1;
		//*)

	protected:

		//(*Identifiers(notificationFrame)
		static const long ID_HTMLWINDOW1;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(notificationFrame)
		void OnPaint(wxPaintEvent& event);
		void OnButton1Click(wxCommandEvent& event);
		void OnTimer1Trigger(wxTimerEvent& event);
		void OnLinkClicked(wxHtmlLinkEvent& event);
		//*)
		void handleEvent(const AmiMessage &message);
		std::string m_current_channel;
		std::string m_lookup_cmd;
		int buttonsHeight;
};


#endif
