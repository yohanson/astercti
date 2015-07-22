#ifdef WX_PRECOMP
	#include <wx/wxprec.h>
#else
	#include <wx/wx.h>
#endif

#include <wx/string.h>
#include <locale>
#include <wx/intl.h>

#include "chan_events.h"
#include "controller.h"
#include "asterisk.h"
#include "notificationFrame.h"
#include "mainframe.h"
#include "myapp.h"
#include "taskbaricon.h"

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
    std::cout << "hello" << std::endl;
    if (!setlocale(LC_CTYPE, ""))
    {
    	fprintf(stderr, "Can't set the specified locale! "
	                "Check LANG, LC_CTYPE, LC_ALL.\n");
	return 1;
    }
    m_locale.Init();
    m_locale.AddCatalog("astercti");
	
    m_config = NULL;
    m_config = new wxFileConfig(wxT("astercti"),
                                wxEmptyString,
                                wxT("astercti.ini"),
                                wxEmptyString,
                                wxCONFIG_USE_SUBDIR);
    wxFileName configfile = m_config->GetLocalFile("astercti.ini", wxCONFIG_USE_SUBDIR);
    if (!configfile.IsFileReadable())
    {
	std::cerr << "Error opening config file." << std::endl
	          << "Sample config is at /usr/share/astercti/astercti.ini" << std::endl
	          << "Copy it to " << configfile.GetFullPath() << " and edit." << std::endl;
	return false;
    }

    std::cout << "Filename: " << configfile.GetFullPath() << std::endl;

    MyFrame *frame = new MyFrame( "AsterCTI", wxDefaultPosition, wxSize(450, 340) );
    SetExitOnFrameDelete(true);
    frame->Show( true );
    SetTopWindow(frame);
    std::cout << "addr: " << m_config->Read("server/address") << std::endl;
    Asterisk *asterisk = new Asterisk(m_config->Read("server/address").ToStdString(),
		5038,
		m_config->Read("server/username").ToStdString(),
		m_config->Read("server/password").ToStdString());
    m_controller = new AsteriskController(asterisk, m_config);
    m_controller->SetMainFrame(frame);
    MyChanFilter *mychanfilter = new MyChanFilter(m_config->Read("dialplan/channel").ToStdString());
    asterisk->add(*mychanfilter);
    mychanfilter->add(*frame);
    notificationFrame *notifyframe = new notificationFrame(frame);
    notifyframe->SetLookupCmd(m_config->Read("lookup/lookup_cmd").ToStdString());
    mychanfilter->add(*notifyframe);
    EventGenerator *events = new EventGenerator;
    events->add(*frame);
    mychanfilter->add(*events);
    wxString iconfile = "/usr/share/pixmaps/astercti.png";
    wxIcon iconimage(iconfile, wxBITMAP_TYPE_PNG);
    frame->SetIcon(iconimage);
    MyTaskBarIcon *icon = new MyTaskBarIcon(iconimage);
    icon->SetMainFrame(frame);
    m_controller->add(icon);
    m_controller->add(frame);
    m_controller->add(notifyframe);
    return true;
}

MyApp::~MyApp()
{
	std::cout << "bye" << std::endl;
}
