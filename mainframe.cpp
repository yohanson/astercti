#include <stdexcept>
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/stdpaths.h>

#include "observer.h"
#include "asterisk.h"
#include "myapp.h"
#include "mainframe.h"
#include "version.h"
#include "call.h"
#include "chanstatus.h"

wxDECLARE_APP(MyApp);

enum CALLSTATUS {
    INCOMING_ANSWERED,
    INCOMING_UNANSWERED,
    OUTBOUND_ANSWERED,
    OUTBOUND_UNANSWERED,
    INCOMING_ANSWERED_ELSEWHERE
};

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size, ChannelStatusPool *pool)
        : wxFrame(NULL, wxID_ANY, title, pos, size), ChannelStatusPooler(pool)
{
    edescr = "mainframe";
    m_taskbaricon = NULL;

    wxImage::AddHandler(new wxPNGHandler);
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuHelp, "&Help" );
    SetMenuBar( menuBar );

    wxString datadir = wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator();
    wxImageList *imagelist = new wxImageList(24, 24, true);
    imagelist->Add(wxBitmap(wxImage(datadir + "incoming_answered.png")));
    imagelist->Add(wxBitmap(wxImage(datadir + "incoming_unanswered.png")));
    imagelist->Add(wxBitmap(wxImage(datadir + "outbound_answered.png")));
    imagelist->Add(wxBitmap(wxImage(datadir + "outbound_unanswered.png")));
    imagelist->Add(wxBitmap(wxImage(datadir + "incoming_answered_elsewhere.png")));

    wxSplitterWindow *TopMostVerticalSplitter = new wxSplitterWindow(this);
    TopMostVerticalSplitter->SetMinSize(wxSize(100,100));
    TopMostVerticalSplitter->SetMinimumPaneSize(100);
    TopMostVerticalSplitter->SetSashGravity(0);

    wxBoxSizer *RightSizer = new wxBoxSizer(wxVERTICAL);
    wxPanel *RightPanel = new wxPanel(TopMostVerticalSplitter);
    RightPanel->SetSizer(RightSizer);
    wxBoxSizer *DialSizer = new wxBoxSizer(wxHORIZONTAL);
    m_DialNumber = new wxTextCtrl(RightPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    wxFont numberFont(wxSize(0,24), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_DialNumber->SetFont(numberFont);
    m_DialButton = new wxBitmapButton(RightPanel, wxID_ANY, wxBitmap(wxImage(datadir + "dial.png")), wxDefaultPosition, wxSize(36,36), wxBU_AUTODRAW);
    DialSizer->Add(m_DialNumber, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    DialSizer->Add(m_DialButton, 0, wxALL|         wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StatusText = new wxTextCtrl(RightPanel, ID_TextCtlNumber, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    m_CallInfo = new wxStaticText(RightPanel, wxID_ANY, "\n\n\n\n");
    m_callList = new wxListCtrl(TopMostVerticalSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
    m_callList->AssignImageList(imagelist, wxIMAGE_LIST_SMALL);
    m_callList->InsertColumn(0, "");
    RightSizer->Add(DialSizer, 0, wxEXPAND);
    RightSizer->Add(m_CallInfo, 0, wxEXPAND);
    RightSizer->Add(StatusText, 1, wxEXPAND);
    TopMostVerticalSplitter->SplitVertically(m_callList, RightPanel);

    Connect(ID_Hello, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnHello));
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnExit));
    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnAbout));
    Bind(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MyFrame::OnClose), this);
    m_DialNumber->Bind(wxEVT_TEXT_ENTER, &MyFrame::OnDialPressEnter, this);
    m_DialButton->Bind(wxEVT_BUTTON, &MyFrame::OnDialPressEnter, this);
    m_callList->Bind(wxEVT_SIZE, &MyFrame::OnListResize, this);
    m_callList->Bind(wxEVT_LIST_ITEM_SELECTED, &MyFrame::OnListItemSelect, this);

    CreateStatusBar();
    m_DialNumber->SetFocus();
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxString version_message;
       version_message << "AsterCTI v"  VERSION  "\n"
		"Git commit: " << gitcommit << " " << gitcommitdate << "\n"
		"Built: " << builddate << "\n"
		"https://github.com/yohanson/astercti";
	wxLogMessage(version_message);
}
void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        Show(false);
        event.Veto();
        return;
    }
    m_taskbaricon->Destroy();
	Destroy();
}

void MyFrame::OnDialPressEnter(wxCommandEvent &event)
{
    if (m_last_channel_state == AST_STATE_DOWN)
    {
        if (!m_DialNumber->GetValue().IsEmpty())
            m_controller->Originate(m_DialNumber->GetValue().ToStdString());
    }
    else
    {
        m_controller->HangupChannel(m_current_channel);
    }
}

void MyFrame::UpdateDialButtonImage()
{
    if (m_last_channel_state == AST_STATE_DOWN)
        m_DialButton->SetBitmap(wxBitmap(wxImage(wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator() + "dial.png")));
    else
        m_DialButton->SetBitmap(wxBitmap(wxImage(wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator() + "hangup.png")));
}

void MyFrame::OnListResize(wxSizeEvent &event)
{
	wxSize csize = m_callList->GetClientSize();
	wxSize vsize = m_callList->GetVirtualSize();
	int width = csize.x;
	if (vsize.y > csize.y)
		width = csize.x - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
	m_callList->SetColumnWidth(0, width);

	event.Skip();
}

void MyFrame::OnListItemSelect(wxListEvent &event)
{
	Call *call = reinterpret_cast<Call *>(event.GetData());
	m_DialNumber->SetValue(call->GetNumber());
	wxString label;
	wxDateTime duration((time_t)call->GetDuration());
	wxString timeformat;
	if (call->GetDuration() >= 3600)
		timeformat = "%H:%M:%S";
	else timeformat = "%M:%S";
	label << _("Number: ") << call->GetNumber() << '\n' << _("Name: ")
	       << call->GetName() << '\n' << _("Time: ") << call->GetTime().FormatISOCombined(' ')
	       << '\n' << _("Duration: ") << duration.Format(timeformat, wxDateTime::UTC);
	m_CallInfo->SetLabel(label);
}

void MyFrame::SetTaskBarIcon(MyTaskBarIcon *taskbaricon)
{
    m_taskbaricon = taskbaricon;
}

void MyFrame::handleEvent(const AmiMessage &message)
{
	if (message.has("InternalMessage")) return;
    if (StatusText->GetNumberOfLines() > LOG_MAX_LINES)
    {
        int chars = 0;
        for (int i = 0; i<LOG_DELETE_LINES; i++)
        {
            chars += StatusText->GetLineLength(i)+1;
        }
        StatusText->Remove(0, chars);
    }
	std::string status;
	for (auto iter : message)
	{	
		status += iter.first + ": " + iter.second + "\n";
	}
	Log(status);
}

void MyFrame::OnOriginate(const AmiMessage &m)
{
	Log("##### We are originating! #####\n");
	wxListItem *item = new wxListItem;
	item->SetId(m_callList->GetItemCount());
	Call *call = new Call;
	call->SetNumber(m["ConnectedLineName"]); // yes, it's right
	item->SetText(m["ConnectedLineName"]);
	call->SetUniqueID(std::stoi(m["Uniqueid"]));
	call->SetTime(wxDateTime::Now());
	call->SetDirection(Call::CALL_OUT);
	item->SetData(call);
	item->SetImage(2);
	m_callList->InsertItem(*item);
	m_last_channel_state = AST_STATE_RINGING;
    m_current_channel = m["Channel"];
    UpdateDialButtonImage();
}

void MyFrame::OnDialIn(const AmiMessage &m)
{
    Log("##### Somebody's going to dial us #####\n");
    std::string calleridnum = m["CallerIDNum"];
    std::string calleridname = m["CallerIDName"];
    m_current_channel = m["Destination"];
    wxListItem *item = new wxListItem;
	item->SetId(m_callList->GetItemCount());
	Call *call = new Call;
    if (!m["CallerIDName"].empty() && m["CallerIDName"] != "<unknown>")
    {
        item->SetText(m["CallerIDNum"] + " (" + m["CallerIDName"] + ")");
        call->SetName(m["CallerIDName"]);
    }
    else item->SetText(m["CallerIDNum"]);
    std::list<Channel *> peers = m_channelstatuspool->getBridgedChannelsOf(m["Channel"]);
    if (peers.size() == 1)
    {
        Channel *peer = *peers.begin();
        std::string transferred_calleridnum = peer->m_bridgedTo->m_callerIDNum;
        std::string transferred_calleridname = peer->m_bridgedTo->m_callerIDName;
        if (!transferred_calleridname.empty() && transferred_calleridname != "<unknown>")
        {
            item->SetText(transferred_calleridnum);
            call->SetName(m["CallerIDName"] + " [" + transferred_calleridname + "]");
        }
        else item->SetText(m["CallerIDNum"] + " [" + transferred_calleridnum + "]");
	    call->SetNumber(transferred_calleridnum);
    }
    else
    {
	    call->SetNumber(m["CallerIDNum"]);
    }
	call->SetUniqueID(std::stoi(m["DestUniqueID"]));
    call->SetSecondChannelID(m["ChannelID"]);
	call->SetTime(wxDateTime::Now());
	call->SetDirection(Call::CALL_IN);
	item->SetData(call);
	m_callList->InsertItem(*item);
    UpdateDialButtonImage();
}

void MyFrame::OnUp(const AmiMessage &m)
{
	Log("\n#####\nAlready talking\n#####\n");
	wxListItem *item = NULL;
	long lastItem = 0;
	if (m_callList->GetItemCount())
	{
		lastItem = m_callList->GetItemCount()-1;
		Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
		if (call->GetUniqueID() == std::stoi(m["Uniqueid"])) // updating existing call
		{
			if (call->GetDirection() == Call::CALL_IN)
				m_callList->SetItemImage(lastItem, INCOMING_ANSWERED);
			else // outbound call
			{
				if (m["ConnectedLineNum"] != m["CallerIDNum"])
				{
					if (m["ConnectedLineName"] != "")
					{
						m_callList->SetItemText(lastItem, m["ConnectedLineNum"] + " (" + m["ConnectedLineName"] + ")");
						call->SetName(m["ConnectedLineName"]);
					}
					else m_callList->SetItemText(lastItem, m["ConnectedLineNum"]);
					call->SetNumber(m["ConnectedLineNum"]);
				}
				m_callList->SetItemImage(lastItem, OUTBOUND_ANSWERED);
			}
		}
	}
	m_last_channel_state = AST_STATE_UP;
}

void MyFrame::OnHangup(const AmiMessage &m)
{
	Log("##### Hung up! #####\n");
	if (m_callList->GetItemCount())
	{
		int lastItem = m_callList->GetItemCount()-1;
		Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
		if (call->GetUniqueID() == std::stoi(m["UniqueID"]))
		{
            if (m_last_channel_state == AST_STATE_RINGING)
            {
                // Originating cancelled:
                if (m["ConnectedLineNum"] == m["CallerIDNum"])
                {
                    delete call;
                    m_callList->DeleteItem(lastItem);
                }
                else
                {
                    if (m["Cause"] == "26") // answered elsewhere
                    {
                        m_callList->SetItemImage(lastItem, INCOMING_ANSWERED_ELSEWHERE);
                    }
                }
            }
		}
	}
	m_last_channel_state = AST_STATE_DOWN;
    UpdateDialButtonImage();
}

void MyFrame::OnCdr(const AmiMessage &m)
{
	Log("##### Cdr data arrived! #####\n");
	long lastItem = 0;
	if (!m["DestinationChannel"].empty())
	{
		wxListItem *item = NULL;
		if (m_callList->GetItemCount())
		{
			lastItem = m_callList->GetItemCount()-1;
			Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
			if (call->GetUniqueID() == std::stoi(m["UniqueID"])) // updating existing call
			{
				call->SetDuration(stoi(m["BillableSeconds"]));
				if (call->GetDirection() == Call::CALL_OUT)
				{
					call->SetNumber(m["Destination"]);
					if (!call->GetName().empty())
						m_callList->SetItemText(lastItem, call->GetNumber() + " (" + call->GetName() + ")");
					else
						m_callList->SetItemText(lastItem, call->GetNumber());
				}

				if (m["Disposition"] == "ANSWERED")
					if (call->GetDirection() == Call::CALL_IN)
						m_callList->SetItemImage(lastItem, INCOMING_ANSWERED);
					else
						m_callList->SetItemImage(lastItem, OUTBOUND_ANSWERED);
				else // missed
				{
					call->SetDuration(0);
					if (call->GetDirection() == Call::CALL_IN)
						m_callList->SetItemImage(lastItem, INCOMING_UNANSWERED);
					else
						m_callList->SetItemImage(lastItem, OUTBOUND_UNANSWERED);
				}
			}
			else Log("UniqueID " + m["UniqueID"] + " not found.");
		}
	}
	else // not a real channel
	{
		if (m_callList->GetItemCount())
		{
			lastItem = m_callList->GetItemCount()-1;
			Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
			if (call->GetUniqueID() == std::stoi(m["UniqueID"]))
			{
				delete call;
				m_callList->DeleteItem(lastItem);
			}
		}

	}

}

void MyFrame::OnDial(const AmiMessage &m)
{
	Log("##### We are dialing out! #####\n");
    m_current_channel = m["Channel"];
	wxListItem *item = new wxListItem;
	item->SetId(m_callList->GetItemCount());
	Call *call = new Call;
	if (m["ConnectedLineName"] != "")
	{
		item->SetText(m["ConnectedLineNum"] + " (" + m["ConnectedLineName"] + ")");
		call->SetName(m["ConnectedLineName"]);
	}
	else item->SetText(m["ConnectedLineNum"]);
	call->SetNumber(m["ConnectedLineNum"]);
	call->SetUniqueID(std::stoi(m["Uniqueid"]));
	call->SetTime(wxDateTime::Now());
	call->SetDirection(Call::CALL_OUT);
	item->SetData(call);
	item->SetImage(2);
	m_callList->InsertItem(*item);
	m_last_channel_state = AST_STATE_RING;
    UpdateDialButtonImage();
}


void MyFrame::OnInternalMessage(const AmiMessage &m)
{
	static std::string last;
	if (m["InternalMessage"] == last) return;
	last = m["InternalMessage"];
	if (m["InternalMessage"] == "ConnectionLost")
	{
		SetStatusText(_("Connection Lost"));
		Log(wxDateTime::Now().FormatISOCombined() + " " + _("Connection Lost"));
	}
	else if (m["InternalMessage"] == "Connected")
	{
		SetStatusText(_("Connected"));
		Log(wxDateTime::Now().FormatISOCombined() + " " + _("Connected"));
	}
}

void MyFrame::Log(const wxString &str)
{
    StatusText->AppendText(str + "\n");
}
