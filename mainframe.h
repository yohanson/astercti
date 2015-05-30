#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

class MyFrame: public wxFrame, public IObserver, public ControllerUser
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
    ~MyFrame();
    void SetController(AsteriskController *ctrl){m_controller = ctrl;}
    void handleEvent(const AmiMessage &message)
    {
	std::string status;
	for (auto iter : message)
	{	
		status += iter.first + ": " + iter.second + "\n";
	}
	StatusText->AppendText(status+"\n");
    }

private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    wxTextCtrl *StatusText;
    std::string m_channel_id;
    AsteriskController *m_controller;
};

enum
{
    ID_Hello = 1,
    ID_TextCtlNumber
};

#endif