// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#ifdef WX_PRECOMP
	#include <wx/wxprec.h>
#else
	#include <wx/wx.h>
#endif

#include "asterisk.h"
#include "notificationFrame.h"

class MyApp: public wxApp
{
public:
    virtual bool OnInit();
};
class MyFrame: public wxFrame, public IObserver
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    void handleEvent(const AmiMessage &message) const
    {
	std::string status;
	for (auto iter : message)
	{	
		status += iter.first + ": " + iter.second + "\n";
	}
	StatusText->SetValue(status);
    }

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    wxTextCtrl *StatusText;
    std::string m_channel_id;
};
enum
{
    ID_Hello = 1,
    ID_TextCtlNumber
};
wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
    frame->Show( true );
    notificationFrame *notifyframe = new notificationFrame(0);
    Asterisk *asterisk = new Asterisk("212.20.30.8", 5038, "themaster", "MyS3cretPasswordManager");
    MyChanFilter *mychanfilter = new MyChanFilter("SIP/it1");
    asterisk->add(*mychanfilter);
    mychanfilter->add(*frame);
    mychanfilter->add(*notifyframe);
    return true;
}
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
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
    StatusText = new wxTextCtrl(this, ID_TextCtlNumber, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRLNUMBER"));

    Connect(ID_Hello, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnHello));
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnExit));
    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnAbout));

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

