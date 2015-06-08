#include <iostream>
#include <stdexcept>
#include <wx/event.h>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/display.h>
#include <wx/app.h>
#include <wx/process.h>
#include <wx/txtstrm.h>
#include <wx/intl.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>

#include "notificationFrame.h"
#include "controller.h"

const long notificationFrame::ID_HTMLWINDOW1 = wxNewId();
const long notificationFrame::ID_BUTTON1 = wxNewId();

void ExecCommand(wxString &cmd, wxArrayString &output)
{
	wxProcess p;
	p.Redirect();
	wxExecute(cmd, wxEXEC_SYNC, &p);

	wxInputStream *i = p.GetInputStream();
	if(i)
	{ 
		wxTextInputStream t(*i, " \t", wxConvUTF8);

		while(!i->Eof())
		{
			output.Add(t.ReadLine());
		} 
	}
}


notificationFrame::notificationFrame(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	descr = "notify frame";
	wxBoxSizer* BoxSizerButtons;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP|wxFRAME_NO_TASKBAR|wxFRAME_TOOL_WINDOW|wxNO_BORDER);
	SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	HtmlWindow1 = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(400,30), wxHW_SCROLLBAR_NEVER|wxHW_NO_SELECTION);
	HtmlWindow1->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
	HtmlWindow1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
	BoxSizer1->Add(HtmlWindow1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BoxSizerButtons = new wxBoxSizer(wxHORIZONTAL);
	Button1 = new wxButton(this, ID_BUTTON1, _("No"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	BoxSizerButtons->Add(Button1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BoxSizer1->Add(BoxSizerButtons, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	buttonsHeight = BoxSizerButtons->GetSize().GetHeight();
	wxButton *hidebutton = new wxButton(HtmlWindow1, wxID_ANY, wxT("×"), wxPoint(380,0), wxSize(20,20), wxBORDER_NONE);
	m_hidetimer = new wxTimer(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&notificationFrame::OnButton1Click);
	Connect(wxEVT_PAINT,(wxObjectEventFunction)&notificationFrame::OnPaint);
	Bind(wxEVT_HTML_LINK_CLICKED, &notificationFrame::OnLinkClicked, this);
	hidebutton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &notificationFrame::OnHideButtonClick, this);
	Bind(wxEVT_TIMER, &notificationFrame::OnHideTimer, this);
	
}

notificationFrame::~notificationFrame()
{
	delete m_hidetimer;
}

void notificationFrame::SetLookupCmd(std::string cmd) {
	std::cout << "Lookup cmd: " << cmd << std::endl;
	m_lookup_cmd = cmd;
}

void notificationFrame::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	wxColor fg,bg,middle;
	fg = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT);
	bg = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK);
	middle = wxColour((fg.Red()+bg.Red()*2)/3, (fg.Green()+bg.Green()*2)/3, (fg.Blue()+bg.Blue()*2)/3);
	dc.SetPen(middle);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	wxSize sz(GetSize());
	dc.DrawRectangle(0, 0, sz.x, sz.y);
	event.Skip();
}

void notificationFrame::OnButton1Click(wxCommandEvent& event)
{
	m_controller->HangupChannel(m_current_channel);
}

void notificationFrame::OnHideButtonClick(wxCommandEvent& event)
{
	Hide();
}

void notificationFrame::OnHideTimer(wxTimerEvent &event)
{
	Hide();
}

void notificationFrame::OnLinkClicked(wxHtmlLinkEvent& event)
{
	wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
}

void notificationFrame::SetHtml(const wxString &s)
{
	wxString bgcolor = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK).GetAsString();
	HtmlWindow1->SetPage("<body bgcolor='"+bgcolor+"'>"+s+"</body>");
	int height = HtmlWindow1->GetInternalRepresentation()->GetHeight();
	int width = HtmlWindow1->GetInternalRepresentation()->GetWidth();
	BoxSizer1->SetItemMinSize(HtmlWindow1, width, height);
	HtmlWindow1->Fit();
	Fit();
	wxDisplay display;
    	wxRect rect = display.GetClientArea();
    	wxSize size = GetSize();
    	Move(rect.GetBottomRight()-size-wxSize(0,4));
 
}

void notificationFrame::UpdateSize()
{
}

void notificationFrame::handleEvent(const AmiMessage &message)
{
	bool is_channel_up = false;
	bool is_channel_ringing = false;
	wxString html;
	std::string callerid = "";
	try {
		if (message.at("Event") == "Newstate" && message.at("ChannelID") == m_controller->GetMyChannel())
		{
			if (message.at("ChannelStateDesc") == "Up")
			{
				is_channel_up = true;
				callerid = message.at("ConnectedLineNum");
				long timer = m_controller->CfgInt("gui/pickup_notify_hide_timeout");
				if (!timer){
				       Hide();
				       return;
				}
				else if (timer != -1)
					m_hidetimer->StartOnce(timer);
			}
			else if (message.at("ChannelStateDesc") == "Ring"
		       	 || message.at("ChannelStateDesc") == "Ringing")
			{
				is_channel_up = true;
				is_channel_ringing = true;
				callerid = message.at("ConnectedLineNum");
				if (callerid == m_controller->GetMyExten() && message.at("ChannelStateDesc") == "Ringing")
				{
					html = _("Pickup the handset to dial") + " <b>" + message.at("ConnectedLineName") + "</b>";
					SetHtml(html);
				}
				else
				{
					html = "Channel: " + message.at("Channel") + "<br>CallerID: " + message.at("ConnectedLineNum") + " (" + message.at("ConnectedLineName") + ")";
				}
				m_current_channel = message.at("Channel");
			}
		}
		else if (message.at("Event") == "Hangup")
		{
			is_channel_up = false;
		}
	}
	catch (std::out_of_range)
	{
	
	}
	if (is_channel_up)
	{
	       	if (is_channel_ringing && !callerid.empty() && callerid != "<unknown>" && !m_lookup_cmd.empty())
		{
			SetHtml(html + "<br><img src='/usr/share/astercti/wait.gif'>");
			Show();
			wxString out = Lookup(callerid);
			SetHtml(html+"<br />" + out);
			//std::cout << out << std::endl;
		}
		else if (!html.empty())
		{
			SetHtml(html);
		}
		Show();
	}
	else {
		Hide();
	}
}

wxString notificationFrame::Lookup(std::string callerid)
{
	wxString out;
	if (m_lookup_cmd.substr(0,4) == "http")
	{
		std::cout << "http cmd" << std::endl;
		wxHTTP get;
		//get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
		get.SetTimeout(5); // 5 seconds of timeout instead of 10 minutes ...

		while (!get.Connect(_T("infin.risp.ru")))
			    wxSleep(1);

		wxApp::IsMainLoopRunning();

		wxInputStream *httpStream = get.GetInputStream(_T("/cgi-bin/inner/searchbynumber.cgi?Str=9139424977"));

		if (get.GetError() == wxPROTO_NOERR)
		{
			wxStringOutputStream out_stream(&out);
			httpStream->Read(out_stream);
			std::cout << "Lookup got: " << out << std::endl;
		}
		else
		{
			std::cout << _("Unable to connect!") << endl;
		}

		wxDELETE(httpStream);
		get.Close();
	}
	else
	{
		wxArrayString output;
		wxString cmd;
		cmd.Printf(wxString(m_lookup_cmd), callerid);
		ExecCommand(cmd, output);
		for (auto iter : output)
		{
			out += iter;
		}
	}
	return out;
}
