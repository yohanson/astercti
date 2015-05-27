#include <wx/wx.h>
#include <wx/app.h>

#include "observer.h"
#include "asterisk.h"
#include "myapp.h"
#include "mainframe.h"

wxDECLARE_APP(MyApp);

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                     "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "Originate Biatch!");
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuHelp, "&Help" );
    SetMenuBar( menuBar );
    StatusText = new wxTextCtrl(this, ID_TextCtlNumber, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRLNUMBER"));

    Connect(ID_Hello, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnHello));
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnExit));
    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnAbout));
    Bind(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MyFrame::OnClose), this);


    CreateStatusBar();
    SetStatusText( "Welcome to wxWidgets!" );
    AmiMessage filter;
    m_channel_id = "SIP/it1";
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

