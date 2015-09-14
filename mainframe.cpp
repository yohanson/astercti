#include <stdexcept>
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/image.h>
#include <wx/imaglist.h>

#include "observer.h"
#include "asterisk.h"
#include "myapp.h"
#include "mainframe.h"
#include "version.h"

wxDECLARE_APP(MyApp);

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    descr = "mainframe";

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

    wxImageList *imagelist = new wxImageList(24, 24, true);
    imagelist->Add(wxBitmap(wxImage("/usr/share/astercti/incoming_answered.png")));
    imagelist->Add(wxBitmap(wxImage("/usr/share/astercti/incoming_unanswered.png")));
    imagelist->Add(wxBitmap(wxImage("/usr/share/astercti/outbound_answered.png")));
    imagelist->Add(wxBitmap(wxImage("/usr/share/astercti/outbound_unanswered.png")));

    wxSplitterWindow *TopMostVerticalSplitter = new wxSplitterWindow(this);
    TopMostVerticalSplitter->SetMinSize(wxSize(100,100));
    TopMostVerticalSplitter->SetMinimumPaneSize(100);
    TopMostVerticalSplitter->SetSashGravity(0);

    wxBoxSizer *RightSizer = new wxBoxSizer(wxVERTICAL);
    wxPanel *RightPanel = new wxPanel(TopMostVerticalSplitter);
    RightPanel->SetSizer(RightSizer);
    wxBoxSizer *DialSizer = new wxBoxSizer(wxHORIZONTAL);
    m_DialNumber = new wxTextCtrl(RightPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    wxBitmapButton *DialButton = new wxBitmapButton(RightPanel, wxID_ANY, wxBitmap(wxImage("/usr/share/astercti/dial.png")), wxDefaultPosition, wxSize(36,36), wxBU_AUTODRAW);
    DialSizer->Add(m_DialNumber, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    DialSizer->Add(DialButton, 0, wxALL|         wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StatusText = new wxTextCtrl(RightPanel, ID_TextCtlNumber, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    m_CallInfo = new wxStaticText(RightPanel, wxID_ANY, _("\n\n\n\n"));
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
    DialButton->Bind(wxEVT_BUTTON, &MyFrame::OnDialPressEnter, this);
    m_callList->Bind(wxEVT_SIZE, &MyFrame::OnListResize, this);
    m_callList->Bind(wxEVT_LIST_ITEM_SELECTED, &MyFrame::OnListItemSelect, this);

    CreateStatusBar();
    SetStatusText( _("Welcome to wxWidgets!") );
    m_DialNumber->SetFocus();
}

MyFrame::~MyFrame()
{
	delete m_DialNumber;
	std::cout << "mainframe destruct" << std::endl;

}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxLogMessage("AsterCTI v"  VERSION  "\nhttps://github.com/yohanson/astercti");
}
void MyFrame::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::OnClose(wxCloseEvent& event)
{
	std::cout << "close" << std::endl;
	wxWindow::Destroy();
}

void MyFrame::OnDialPressEnter(wxCommandEvent &event)
{
	if (!m_DialNumber->GetValue().IsEmpty())
		m_controller->Originate(m_DialNumber->GetValue().ToStdString());
}

void MyFrame::OnListResize(wxSizeEvent &event)
{
	wxSize size = event.GetSize();
	m_callList->SetColumnWidth(0, size.x);

	event.Skip();
}

void MyFrame::OnListItemSelect(wxListEvent &event)
{
	Call *call = reinterpret_cast<Call *>(event.GetData());
	m_DialNumber->SetValue(call->GetNumber());
	wxString label;
	label << _("Number: ") << call->GetNumber() << '\n' << _("Name: ")
	       << call->GetName() << '\n' << _("Time: ") << call->GetTime().FormatISOCombined(' ')
	       << '\n' << _("Duration: ") << call->GetDuration();
	m_CallInfo->SetLabel(label);
}

void MyFrame::handleEvent(const AmiMessage &message)
{
	if (message.has("InternalMessage")) return;
	std::string status;
	for (auto iter : message)
	{	
		status += iter.first + ": " + iter.second + "\n";
	}
	StatusText->AppendText(status+"\n");
}

void MyFrame::OnOriginate(const AmiMessage &m)
{
	StatusText->AppendText("##### We are originating! #####\n\n");
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
}

void MyFrame::OnRing(const AmiMessage &m)
{
	StatusText->AppendText("##### Incoming call! #####\n\n");
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
	call->SetDirection(Call::CALL_IN);
	item->SetData(call);
	m_callList->InsertItem(*item);
	m_last_channel_state = AST_STATE_RINGING;
}

void MyFrame::OnUp(const AmiMessage &m)
{
	StatusText->AppendText("\n#####\nAlready talking\n#####\n\n");
	wxListItem *item = NULL;
	long lastItem = 0;
	if (m_callList->GetItemCount())
	{
		lastItem = m_callList->GetItemCount()-1;
		Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
		if (call->GetUniqueID() == std::stoi(m["Uniqueid"])) // updating existing call
		{
			if (call->GetDirection() == Call::CALL_IN)
				m_callList->SetItemImage(lastItem, 0);
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
				m_callList->SetItemImage(lastItem, 2);
			}
		}
	}
	m_last_channel_state = AST_STATE_UP;
}

void MyFrame::OnHangup(const AmiMessage &m)
{
	StatusText->AppendText("##### Hung up! #####\n\n");
	if (m_callList->GetItemCount())
	{
		int lastItem = m_callList->GetItemCount()-1;
		Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
		if (call->GetUniqueID() == std::stoi(m["UniqueID"]))
		{
			if (m["ConnectedLineNum"] == m["CallerIDNum"] && m_last_channel_state == AST_STATE_RINGING)
			{
				delete call;
				m_callList->DeleteItem(lastItem);
			}
		}
	}
	m_last_channel_state = AST_STATE_DOWN;
}

void MyFrame::OnCdr(const AmiMessage &m)
{
	StatusText->AppendText("##### Cdr data arrived! #####\n\n");
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
					StatusText->AppendText("It's Call::CALL_OUT\n");
					call->SetNumber(m["Destination"]);
					if (!call->GetName().empty())
						m_callList->SetItemText(lastItem, call->GetNumber() + " (" + call->GetName() + ")");
					else
						m_callList->SetItemText(lastItem, call->GetNumber());
				}
				else StatusText->AppendText("It's NOT Call::CALL_OUT. (?)\n");
				if (call->GetDirection() == Call::CALL_IN)
					StatusText->AppendText("It's Call::CALL_IN\n");

				if (m["Disposition"] == "ANSWERED")
					if (call->GetDirection() == Call::CALL_IN)
						m_callList->SetItemImage(lastItem, 0);
					else
						m_callList->SetItemImage(lastItem, 2);
				else // missed
				{
					call->SetDuration(0);
					if (call->GetDirection() == Call::CALL_IN)
						m_callList->SetItemImage(lastItem, 1);
					else
						m_callList->SetItemImage(lastItem, 3);
				}
			}
			else StatusText->AppendText("UniqueID " + m["UniqueID"] + " not found.\n");
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
	StatusText->AppendText("##### We are dialing out! #####\n\n");
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
}


void MyFrame::OnInternalMessage(const AmiMessage &m)
{
	if (m["InternalMessage"] == "ConnectionLost")
	{
		SetStatusText(_("Connection Lost"));
	}
	else if (m["InternalMessage"] == "Connected")
	{
		SetStatusText(_("Connected"));
	}
}

