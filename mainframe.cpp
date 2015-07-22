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
    m_callList = new wxListCtrl(TopMostVerticalSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
    m_callList->AssignImageList(imagelist, wxIMAGE_LIST_SMALL);
    m_callList->InsertColumn(0, "");
    RightSizer->Add(DialSizer, 0, wxEXPAND);
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
	m_DialNumber->SetValue(event.GetText());
}

void MyFrame::handleEvent(const AmiMessage &message)
{
	std::string status;
	for (auto iter : message)
	{	
		status += iter.first + ": " + iter.second + "\n";
	}
	StatusText->AppendText(status+"\n");
}

void MyFrame::OnOriginate(const AmiMessage &m)
{
	StatusText->AppendText("\n#####\nWe are originating!\n#####\n\n");
}

void MyFrame::OnRing(const AmiMessage &m)
{
	StatusText->AppendText("\n#####\nIncoming call!\n#####\n\n");
}

void MyFrame::OnHangup(const AmiMessage &m)
{
	StatusText->AppendText("\n#####\nHung up!\n#####\n\n");
}

void MyFrame::OnCdr(const AmiMessage &m)
{
	StatusText->AppendText("\n#####\nCdr data arrived!\n#####\n\n");
	if (!m.at("DestinationChannel").empty())
	{
		if (m.at("ChannelID") == m_controller->GetMyChannel()) // outbound call
		{
			wxListItem *item = new wxListItem;
			item->SetId(m_callList->GetItemCount());
			item->SetText(m.at("Destination"));
			if (m.at("Disposition") == "ANSWERED")
				item->SetImage(2);
			else item->SetImage(3);
			m_callList->InsertItem(*item);
		}
		if (m.at("DestinationChannelID") == m_controller->GetMyChannel()) // incoming call
		{
			wxListItem *item = new wxListItem;
			item->SetId(m_callList->GetItemCount());
			std::string cid = m.at("CallerID");
			size_t first = cid.find_first_of("\"");
			size_t last = cid.find_last_of("\"");
			if (first != std::string::npos && last != std::string::npos && first != last)
			{
				cid = cid.substr(first+1, last-1);
				item->SetText(m.at("Source") + " (" + cid + ")");
			}
			else item->SetText(m.at("Source"));
			if (m.at("Disposition") == "ANSWERED")
				item->SetImage(0);
			else item->SetImage(1);
			m_callList->InsertItem(*item);
		}
	}
}

void MyFrame::OnDial(const AmiMessage &m)
{
	StatusText->AppendText("\n#####\nWe are dialing out!\n#####\n\n");
}

void MyFrame::OnUp(const AmiMessage &m)
{
	StatusText->AppendText("\n#####\nAlready talking.\n#####\n\n");
}


