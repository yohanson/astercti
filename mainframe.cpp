#include <stdexcept>
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>

#include "observer.h"
#include "asterisk.h"
#include "myapp.h"
#include "calllistctrl.h"
#include "mainframe.h"
#include "version.h"
#include "call.h"
#include "chanstatus.h"

#define CALLS_FILE "calls.txt"

wxDECLARE_APP(MyApp);

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size, ChannelStatusPool *pool)
        : wxFrame(NULL, wxID_ANY, title, pos, size), ChannelStatusPooler(pool)
{
    edescr = "mainframe";
    m_taskbaricon = NULL;
    m_last_channel_state = AST_STATE_DOWN;
    m_missed_calls = 0;

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
    m_CallInfo = new wxStaticText(RightPanel, wxID_ANY, "\n\n\n\n\n\n");
    m_callList = new CallListCtrl(TopMostVerticalSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
    m_callList->AssignImageList(imagelist, wxIMAGE_LIST_SMALL);
    m_callList->InsertColumn(0, "");
    m_callList->InsertColumn(1, "");
    if (!LoadCalls(CALLS_FILE))
        std::cerr << _("Loading calls failed") << std::endl;
    RightSizer->Add(DialSizer, 0, wxEXPAND);
    RightSizer->Add(m_CallInfo, 0, wxEXPAND);
    RightSizer->Add(StatusText, 1, wxEXPAND);
    TopMostVerticalSplitter->SplitVertically(m_callList, RightPanel);

    Connect(ID_Hello, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnHello));
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnExit));
    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnAbout));
    Bind(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MyFrame::OnClose), this);
    Bind(wxEVT_ACTIVATE, wxActivateEventHandler(MyFrame::OnActivate), this);
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
    if (!SaveCalls(CALLS_FILE))
        std::cerr << _("Saving calls failed") << std::endl;
    m_taskbaricon->Destroy();
	Destroy();
}

void MyFrame::OnActivate(wxActivateEvent &event)
{
    if (m_missed_calls)
    {
        m_missed_calls = 0;
        m_taskbaricon->SetMissedCalls(0);
    }
}

void MyFrame::OnDialPressEnter(wxCommandEvent &event)
{
    if (m_last_channel_state == AST_STATE_DOWN)
    {
        if (!m_DialNumber->GetValue().IsEmpty())
        {
            std::string raw = m_DialNumber->GetValue().ToStdString();
            std::string digits;
            for (auto it = raw.begin(); it != raw.end(); ++it)
            {
                if (*it >= '0' && *it <= '9')
                {
                    digits += *it;
                }
            }
            m_controller->Originate(digits);
        }
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
    int datewidth = width / 2;
    if (width >= 300)
        datewidth = 150;
    else datewidth = 0;
	m_callList->SetColumnWidth(0, width-datewidth);
    m_callList->SetColumnWidth(1, datewidth);

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
	label << _("Number: ") << call->GetNumber() << '\n';
    if (!call->GetName().empty())
    {
        label << _("Name: ") << call->GetName() << '\n';
    }
    label << _("Dial: ") << call->GetTimeStart().FormatISOCombined(' ') << '\n';
    if (call->GetTimeAnswer().IsValid())
    {
        label << _("Answer: ") << call->GetTimeAnswer().FormatISOCombined(' ') << '\n';
    }
    if (call->GetTimeEnd().IsValid())
    {
        label << _("End: ") << call->GetTimeEnd().FormatISOCombined(' ') << '\n';
    }
    label << _("Duration: ") << duration.Format(timeformat, wxDateTime::UTC);
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
	item->SetId(0);
	Call *call = new Call;
	call->SetNumber(m["ConnectedLineName"]); // yes, it's right
	item->SetText(m["ConnectedLineName"]);
	call->SetUniqueID(std::stoi(m["Uniqueid"]));
	call->SetTimeStart(wxDateTime::Now());
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
	Call *call = new Call;
    if (!m["CallerIDName"].empty() && m["CallerIDName"] != "<unknown>")
    {
        call->SetName(m["CallerIDName"]);
    }
    std::list<Channel *> peers = m_channelstatuspool->getBridgedChannelsOf(m["Channel"]);
    if (peers.size() == 1)
    {
        Channel *peer = *peers.begin();
        std::string transferred_calleridnum = peer->m_bridgedTo->m_callerIDNum;
        std::string transferred_calleridname = peer->m_bridgedTo->m_callerIDName;
        if (!transferred_calleridname.empty() && transferred_calleridname != "<unknown>")
        {
            call->SetName(m["CallerIDName"] + " [" + transferred_calleridname + "]");
        }
	    call->SetNumber(transferred_calleridnum);
    }
    else
    {
	    call->SetNumber(m["CallerIDNum"]);
    }
	call->SetUniqueID(std::stoi(m["DestUniqueID"]));
    call->SetSecondChannelID(m["ChannelID"]);
	call->SetTimeStart(wxDateTime::Now());
	call->SetDirection(Call::CALL_IN);
	m_callList->InsertCallItem(call);
    UpdateDialButtonImage();
}

void MyFrame::OnUp(const AmiMessage &m)
{
	Log("\n#####\nAlready talking\n#####\n");
	wxListItem *item = NULL;
	long lastItem = 0;
	if (m_callList->GetItemCount())
	{
		Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
		if (call->GetUniqueID() == std::stoi(m["Uniqueid"])) // updating existing call
		{
            call->SetTimeAnswer(wxDateTime::Now());
            call->SetDisposition(Call::CALL_ANSWERED);
			if (call->GetDirection() == Call::CALL_OUT)
			{
				if (m["ConnectedLineNum"] != m["CallerIDNum"])
				{
					if (m["ConnectedLineName"] != "")
					{
						call->SetName(m["ConnectedLineName"]);
					}
					call->SetNumber(m["ConnectedLineNum"]);
				}
			}
		}
	}
    m_callList->UpdateItem(lastItem);
	m_last_channel_state = AST_STATE_UP;
}

void MyFrame::OnHangup(const AmiMessage &m)
{
	Log("##### Hung up! #####\n");
	if (m_callList->GetItemCount())
	{
		int lastItem = 0;
		Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
		if (call->GetUniqueID() == std::stoi(m["UniqueID"]))
		{
            call->SetTimeEnd(wxDateTime::Now());
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
                    switch (std::stoi(m["Cause"]))
                    {
                        case AST_CAUSE_NORMAL_CLEARING:
                            call->SetDisposition(Call::CALL_UNANSWERED);
                            m_callList->SetItemImage(lastItem, INCOMING_UNANSWERED);
                            if (!IsActive())
                            {
                                m_missed_calls++;
                                m_taskbaricon->SetMissedCalls(m_missed_calls);
                            }
                            break;

                        case AST_CAUSE_ANSWERED_ELSEWHERE:
                            call->SetDisposition(Call::CALL_ANSWERED_ELSEWHERE);
                            m_callList->SetItemImage(lastItem, INCOMING_ANSWERED_ELSEWHERE);
                            break;
                    }
                }
            }
            else if (m_last_channel_state == AST_STATE_UP)
            {
                if (!call->GetDuration())
                {
                    time_t duration = ( call->GetTimeEnd() - call->GetTimeAnswer() ).GetSeconds().GetLo();
                    call->SetDuration(duration);
                }
            }
		}
        else Log("Call not found");
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
		if (m_callList->GetItemCount())
		{
			lastItem = 0;
			Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
			if (call->GetUniqueID() == std::stoi(m["UniqueID"])) // updating existing call
			{
				call->SetDuration(stoi(m["BillableSeconds"]));
			}
			else Log("UniqueID " + m["UniqueID"] + " not found.");
		}
	}
	else // not a real channel
	{
		if (m_callList->GetItemCount())
		{
			lastItem = 0;
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
	Call *call = new Call;
	if (m["ConnectedLineName"] != "")
	{
		call->SetName(m["ConnectedLineName"]);
	}
	call->SetNumber(m["ConnectedLineNum"]);
	call->SetUniqueID(std::stoi(m["Uniqueid"]));
	call->SetTimeStart(wxDateTime::Now());
	call->SetDirection(Call::CALL_OUT);
	m_callList->InsertCallItem(call);
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

bool MyFrame::SaveCalls(const wxString &filename)
{
    wxFile calls;
    wxString fullpath = wxGetApp().m_config->GetLocalFileName(filename, wxCONFIG_USE_SUBDIR);
    calls.Open(fullpath, wxFile::write);
    long maxcalls = wxGetApp().m_config->ReadLong("gui/max_calls_saved", 1000);
    if (calls.IsOpened())
    {
        for (long i=0; i<m_callList->GetItemCount() && i<maxcalls; i++)
        {
            Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(i));
            calls.Write(call->Serialize() + "\n");
        }
	    calls.Close();
        return true;
    }
    std::cerr << _("Unable to open calls file") << " " << fullpath << std::endl;
    return false;
}

bool MyFrame::LoadCalls(const wxString &filename)
{
    wxFile calls;
    wxString fullpath = wxGetApp().m_config->GetLocalFileName(filename, wxCONFIG_USE_SUBDIR);
    calls.Open(fullpath, wxFile::read);
    if (calls.IsOpened())
    {
        wxString buff;
        if (!calls.ReadAll(&buff))
        {
            std::cerr << _("Unable to read calls from file") << " " << fullpath << std::endl;
            return false;
        }
        if (buff.length() < 30)
        {
            std::cerr << _("File is too short") << " " << fullpath << std::endl;
            return false;
        }
        wxStringTokenizer calls_tokenizer(buff, "\n");
        while ( calls_tokenizer.HasMoreTokens() )
        {
            wxString serialized_call = calls_tokenizer.GetNextToken();
            Call *call = new Call(serialized_call);
            m_callList->InsertCallItem(call, m_callList->GetItemCount());
        }
	    calls.Close();
        return true;
    }
    std::cerr << _("Unable to open calls file") << " " << fullpath << std::endl;
    return false;
}
