#ifdef WX_PRECOMP
	#include <wx/wxprec.h>
#else
	#include <wx/wx.h>
#endif

#include <wx/string.h>
#include <locale>
#include <wx/intl.h>

#include "events.h"
#include "controller.h"
#include "asterisk.h"
#include "notificationFrame.h"
#include "mainframe.h"
#include "myapp.h"
#include "taskbaricon.h"

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
    if (wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_UNIX)
    {
        wxLog *logger = new wxLogStream(&std::cout);
        wxLog::SetActiveTarget(logger);
    }
    wxLogMessage("app init");
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
        std::ostringstream msg;
        msg << _("Error opening config file.") << std::endl
            << _("Sample config is at /usr/share/astercti/astercti.ini") << std::endl
            << _("Copy it to ") << configfile.GetFullPath() << _(" and edit.");
        wxLogMessage("%s", msg.str());
	return false;
    }

    wxLogMessage("Filename: %s", configfile.GetFullPath());

    MyFrame *frame = new MyFrame( "AsterCTI", wxDefaultPosition, wxSize(600, 400) );
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
    InternalMessageFilter *intmsgfilter = new InternalMessageFilter();
    asterisk->add(*mychanfilter);
    asterisk->add(*intmsgfilter);
    mychanfilter->add(*frame);
    intmsgfilter->add(*frame);
    notificationFrame *notifyframe = new notificationFrame(frame);
    notifyframe->SetLookupCmd(m_config->Read("lookup/lookup_cmd").ToStdString());
    EventGenerator *events = new EventGenerator;
    events->add(*frame);
    events->add(*notifyframe);
    mychanfilter->add(*events);
    intmsgfilter->add(*events);
    wxString iconfile = "/usr/share/pixmaps/astercti.png";
    wxIcon iconimage(iconfile, wxBITMAP_TYPE_PNG);
    frame->SetIcon(iconimage);
    MyTaskBarIcon *icon = new MyTaskBarIcon(iconimage);
    icon->SetMainFrame(frame);
    m_controller->add(icon);
    m_controller->add(frame);
    m_controller->add(notifyframe);
    m_controller->add(events);
    return true;
}

MyApp::~MyApp()
{
	std::cout << "app destruct" << std::endl;
}
