// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#ifdef WX_PRECOMP
	#include <wx/wxprec.h>
#else
	#include <wx/wx.h>
#endif

#include <wx/string.h>

#include "controller.h"
#include "asterisk.h"
#include "notificationFrame.h"
#include "myapp.h"
#include "mainframe.h"

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
    m_config = NULL;
    m_config = new wxFileConfig(wxT("astercti"),
                                wxEmptyString,
                                wxT("astercti.ini"),
                                wxEmptyString,
                                wxCONFIG_USE_SUBDIR);
    if (!m_config)
    {
	std::cerr << "Error opening config file." << std::endl;
	return false;
    }

	
    MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
    SetExitOnFrameDelete(true);
    frame->Show( true );
    SetTopWindow(frame);
    std::cout << "addr: " << m_config->Read("server/address") << std::endl;
    Asterisk *asterisk = new Asterisk(m_config->Read("server/address"),
		5038,
		m_config->Read("server/username"),
		m_config->Read("server/password"));
    m_controller = new AsteriskController(asterisk);
    MyChanFilter *mychanfilter = new MyChanFilter("dialplan/channel");
    asterisk->add(*mychanfilter);
    mychanfilter->add(*frame);
    notificationFrame *notifyframe = new notificationFrame(frame);
    notifyframe->SetController(m_controller);
    mychanfilter->add(*notifyframe);
    std::cout << "ExitOnFrameDelete: " << GetExitOnFrameDelete() << std::endl;
    return true;
}

