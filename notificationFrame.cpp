#include <iostream>
#include <wx/event.h>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/display.h>
#include <wx/app.h>
#include <wx/process.h>
#include <wx/txtstrm.h>

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

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP|wxFRAME_NO_TASKBAR|wxFRAME_TOOL_WINDOW|wxNO_BORDER, _T("wxID_ANY"));
	SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	HtmlWindow1 = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(400,30), wxHW_SCROLLBAR_NEVER|wxHW_NO_SELECTION, _T("ID_HTMLWINDOW1"));
	HtmlWindow1->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
	HtmlWindow1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
	BoxSizer1->Add(HtmlWindow1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BoxSizerButtons = new wxBoxSizer(wxHORIZONTAL);
	Button1 = new wxButton(this, ID_BUTTON1, _("No"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizerButtons->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BoxSizer1->Add(BoxSizerButtons, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	buttonsHeight = BoxSizerButtons->GetSize().GetHeight();

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&notificationFrame::OnButton1Click);
	Connect(wxEVT_PAINT,(wxObjectEventFunction)&notificationFrame::OnPaint);
	Bind(wxEVT_HTML_LINK_CLICKED, &notificationFrame::OnLinkClicked, this);
	//HtmlWindow1->SetBorders(0);
	//Show(true);
	
}

notificationFrame::~notificationFrame()
{
}

void notificationFrame::SetLookupCmd(std::string cmd) {
	std::cout << "Lookup cmd: " << cmd << std::endl;
	m_lookup_cmd = cmd;
}

void notificationFrame::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	//DoPrepareDC(dc);
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

void notificationFrame::OnLinkClicked(wxHtmlLinkEvent& event)
{
	wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
}

void notificationFrame::SetHtml(const wxString &s)
{
	wxString bgcolor = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK).GetAsString();
	HtmlWindow1->SetPage("<body bgcolor='"+bgcolor+"'>"+s+"</body>");
	//HtmlWindow1->Layout();
	int height = HtmlWindow1->GetInternalRepresentation()->GetHeight() + buttonsHeight;
	int width = HtmlWindow1->GetInternalRepresentation()->GetWidth();
	//HtmlWindow1->Set
	BoxSizer1->SetItemMinSize(HtmlWindow1, width, height);
	SetInitialSize();
	wxDisplay display;
    	wxRect rect = display.GetClientArea();
    	wxSize size = GetSize();
    	Move(rect.GetBottomRight()-size);
 
}

void notificationFrame::UpdateSize()
{
}

void notificationFrame::handleEvent(const AmiMessage &message)
{
	bool is_channel_up = false;
	std::string html = "";
	std::string callerid = "";
	try {
		if (message.at("Event") == "Newstate")
		{
			if (message.at("ChannelStateDesc") == "Up"
			 || message.at("ChannelStateDesc") == "Ring"
		       	 || message.at("ChannelStateDesc") == "Ringing")
			{
				is_channel_up = true;
				html = "Channel: " + message.at("Channel") + "<br>CallerID: " + message.at("ConnectedLineNum") + " (" + message.at("ConnectedLineName") + ")";
				callerid = message.at("ConnectedLineNum");
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
	      	if (!m_lookup_cmd.empty())
		{
			SetHtml(html + "<br><img src='wait.gif'>");
			Show(is_channel_up);
			wxArrayString output;
			wxString cmd;
			cmd.Printf(wxString(m_lookup_cmd), callerid);
			ExecCommand(cmd, output);
			wxString out = "";
			for (auto iter : output)
			{
				std::cout << "String: " << iter << std::endl;
				out += iter;
			}
			SetHtml(html+"<br />" + out);
			std::cout << out << std::endl;
		}
		else
		{
			SetHtml(html);
			Show(is_channel_up);
		}
	}
	Show(is_channel_up);
}

