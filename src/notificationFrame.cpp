#include <iostream>
#include <stdexcept>
#include <wx/event.h>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/display.h>
#include <wx/app.h>
#include <wx/intl.h>
#include <wx/sstream.h>
#include <wx/stdpaths.h>

#include "notificationFrame.h"
#include "version.h"
#include "chanstatus.h"
#include "iconmacro.h"
#include "myapp.h"
#include "lookup.h"

const long notificationFrame::ID_HTMLWINDOW1 = wxNewId();
const long notificationFrame::ID_BUTTON1 = wxNewId();


notificationFrame::notificationFrame(wxWindow* parent, ChannelStatusPool *pool, Asterisk *a, CallerInfoLookuper *lookuper)
    : ChannelStatusPooler(pool), asterisk(a)
{
	edescr = "notify frame";
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
    wxBitmap hangupIcon;
    hangupIcon.CopyFromIcon(ACTI_ICON_SIZED("hangup", 24));
    Button1->SetBitmap(hangupIcon);
	BoxSizerButtons->Add(Button1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BoxSizer1->Add(BoxSizerButtons, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	m_hidetimer = new wxTimer(this);
#ifdef DEBUG
    wxButton *hidebutton = new wxButton(HtmlWindow1, wxID_ANY, wxT("×"), wxPoint(380,0), wxSize(20,20), wxBORDER_NONE);
	hidebutton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &notificationFrame::OnHideButtonClick, this);
#endif

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&notificationFrame::OnButton1Click);
	Connect(wxEVT_PAINT,(wxObjectEventFunction)&notificationFrame::OnPaint);
	Bind(wxEVT_HTML_LINK_CLICKED, &notificationFrame::OnLinkClicked, this);
	Bind(wxEVT_TIMER, &notificationFrame::OnHideTimer, this);
    m_lookuper = lookuper;
}

notificationFrame::~notificationFrame()
{
    delete m_hidetimer;
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
    asterisk->HangupChannel(m_current_channel);
}

#ifdef DEBUG
void notificationFrame::OnHideButtonClick(wxCommandEvent& event)
{
	Hide();
}
#endif

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
	wxString fgcolor = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT).GetAsString();
	HtmlWindow1->SetPage("<body bgcolor='"+bgcolor+"' text='"+fgcolor+"'>"+s+"</body>");
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

void notificationFrame::OnUp(const AmiMessage &message)
{
	long timer = wxGetApp().CfgInt("gui/pickup_notify_hide_timeout");
	if (!timer){
	       Hide();
	       return;
	}
	else if (timer != -1)
		m_hidetimer->StartOnce(timer);
}

void notificationFrame::OnDialIn(const AmiMessage &message)
{
 	std::string callerid = message["CallerIDNum"];
	wxString html = "";
	html << wxT("<h5><font face='pt sans,sans,arial'>☎ ") + message["CallerIDNum"];
	if (!message["CallerIDName"].empty() && message["CallerIDName"] != "<unknown>")
    {
		html << " (" << message["CallerIDName"] << ")";
    }
	html << "</font></h5>";
	m_current_channel = message["Destination"];

    std::list<Channel *> peers = m_channelstatuspool->getBridgedChannelsOf(message["Channel"]);
    if (peers.size() == 1)
    {
        Channel *peer = *peers.begin();
        std::string transferred_calleridnum = peer->m_bridgedTo->m_callerIDNum;
        std::string transferred_calleridname = "";
        if (peer->m_bridgedTo->m_callerIDName != peer->m_bridgedTo->m_callerIDNum)
            transferred_calleridname = peer->m_bridgedTo->m_callerIDName;
        //StatusText->AppendText(m["CallerIDNum"] + " ("+ m["CallerIDName"] +") is transferring: " + transferred_calleridnum + " (" + transferred_calleridname + ")\n\n");
        html << "<h5>&#8627; " << transferred_calleridnum << " (" << transferred_calleridname << ")</h5>";
        callerid = transferred_calleridnum;
    }

	if (m_lookuper && m_lookuper->ShouldLookup(callerid))
	{
		SetHtml(html + "<br><img src='/usr/share/astercti/wait.gif'>");
		ShowWithoutActivating();
		wxString out = m_lookuper->GetHtml(callerid);
		SetHtml(html+"<br />" + out);
		//std::cout << out << std::endl;
	}
	else if (!html.empty())
	{
		SetHtml(html);
	}
	ShowWithoutActivating();
}

void notificationFrame::OnHangup(const AmiMessage &message)
{
	Hide();
}

void notificationFrame::OnDial(const AmiMessage &message)
{
	m_current_channel = message["Channel"];
}

void notificationFrame::OnOriginate(const AmiMessage &message)
{
	m_current_channel = message["Channel"];
	wxString html = _("Pickup the handset to dial") + " <b>" + message["ConnectedLineName"] + "</b>";
	SetHtml(html);
	ShowWithoutActivating();
}

