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
    imagelist->Add(wxBitmap(wxImage("incoming_answered.png")));
    imagelist->Add(wxBitmap(wxImage("incoming_missed.png")));
    imagelist->Add(wxBitmap(wxImage("outbound_answered.png")));

    wxSplitterWindow *TopMostVerticalSplitter = new wxSplitterWindow(this);
    TopMostVerticalSplitter->SetMinSize(wxSize(100,100));
    TopMostVerticalSplitter->SetMinimumPaneSize(100);
    TopMostVerticalSplitter->SetSashGravity(0);

    wxBoxSizer *RightSizer = new wxBoxSizer(wxVERTICAL);
    wxPanel *RightPanel = new wxPanel(TopMostVerticalSplitter);
    RightPanel->SetSizer(RightSizer);
    wxBoxSizer *DialSizer = new wxBoxSizer(wxHORIZONTAL);
    m_DialNumber = new wxTextCtrl(RightPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    wxBitmapButton *DialButton = new wxBitmapButton(RightPanel, wxID_ANY, wxBitmap(wxImage(_T("dial.png"))), wxDefaultPosition, wxSize(36,36), wxBU_AUTODRAW);
    DialSizer->Add(m_DialNumber, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    DialSizer->Add(DialButton, 0, wxALL|         wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StatusText = new wxTextCtrl(RightPanel, ID_TextCtlNumber, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRLNUMBER"));
    m_callList = new wxListCtrl(TopMostVerticalSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER);
    m_callList->AssignImageList(imagelist, wxIMAGE_LIST_SMALL);
    m_callList->InsertColumn(0, "");
    wxListItem item;
    item.SetId(0);
    item.SetColumn(0);
    item.SetText("911 abrakadabraabrakadabra");
    item.SetImage(1);
    m_callList->InsertItem(item);
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

    CreateStatusBar();
    SetStatusText( "Welcome to wxWidgets!" );
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
	std::cout << "enter!" << std::endl;
	if (!m_DialNumber->GetValue().IsEmpty())
		m_controller->Originate(m_DialNumber->GetValue().ToStdString());
}

void MyFrame::OnListResize(wxSizeEvent &event)
{
	wxSize size = event.GetSize();
	m_callList->SetColumnWidth(0, size.x);

	event.Skip();
}

void MyFrame::handleEvent(const AmiMessage &message)
{
	std::string status;
	for (auto iter : message)
	{	
		status += iter.first + ": " + iter.second + "\n";
	}
	StatusText->AppendText(status+"\n");
	std::string callerid = "";
	try {
		if (message.at("Event") == "Newstate")
		{
			if (message.at("ChannelStateDesc") == "Up"
			 || message.at("ChannelStateDesc") == "Ring"
		       	 || message.at("ChannelStateDesc") == "Ringing")
			{
				callerid = message.at("ConnectedLineNum");
			}
		}
		else if (message.at("Event") == "Hangup")
		{
			if (message.at("ConnectedLineNum") != "<unknown>")
			{
				wxListItem *item = new wxListItem;
				item->SetId(m_callList->GetItemCount());
				item->SetText(message.at("ConnectedLineNum"));
				m_callList->InsertItem(*item);
			}
		}
	}
	catch (std::out_of_range)
	{
	
	}
}

