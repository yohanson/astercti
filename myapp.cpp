#ifdef WX_PRECOMP
	#include <wx/wxprec.h>
#else
	#include <wx/wx.h>
#endif

#include <wx/string.h>
#include <locale>
#include <wx/intl.h>
#include <wx/stdpaths.h>

#include "events.h"
#include "controller.h"
#include "asterisk.h"
#include "notificationFrame.h"
#include "mainframe.h"
#include "myapp.h"
#include "taskbaricon.h"
#include "ipc.h"

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
    wxString datadir = wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator();
    if (wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_UNIX)
    {
        wxLog *logger = new wxLogStream(&std::cout);
        wxLog::SetActiveTarget(logger);
#ifndef __WXMSW__
	wxStandardPaths::Get().SetInstallPrefix("/usr");
	datadir = wxStandardPaths::Get().GetInstallPrefix() + "/share/pixmaps";
#endif
    }
    if (!setlocale(LC_CTYPE, ""))
    {
    	fprintf(stderr, "Can't set the specified locale! "
	                "Check LANG, LC_CTYPE, LC_ALL.\n");
	return 1;
    }
    m_locale.Init();
    m_locale.AddCatalog("astercti");

    if (!ParseCmdLine())
	    return false;
	
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
            << _("Sample config is at ") << configfile.GetFullPath() << ".default" << std::endl
            << _("Rename it to astercti.ini and edit.");
        wxLogMessage("%s", msg.str());
	return false;
    }

    MyFrame *frame = new MyFrame( "AsterCTI", wxDefaultPosition, wxSize(600, 400) );
    SetExitOnFrameDelete(true);
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
    wxString iconfile = datadir + wxFileName::GetPathSeparator() + "astercti.png";
    wxIcon iconimage(iconfile, wxBITMAP_TYPE_PNG);
    frame->SetIcon(iconimage);
    MyTaskBarIcon *icon = new MyTaskBarIcon(iconimage);
    icon->SetMainFrame(frame);
    m_controller->add(icon);
    m_controller->add(frame);
    m_controller->add(notifyframe);
    m_controller->add(events);
    m_taskbaricon = icon;
    frame->Show(!start_iconified);
    IpcServer *m_ipcServer = new IpcServer(m_controller);
    m_ipcServer->Create(IPC_SERVICENAME);
    return true;
}

MyApp::~MyApp()
{
	m_taskbaricon->Destroy();
}

bool MyApp::ParseCmdLine()
{
	wxCmdLineParser parser(g_cmdLineDesc, argc, argv);
	switch ( parser.Parse() )
    {
        case -1: return false; // The parameter -h was passed, help was given, so abort the app
	    case  0: break; // OK, so break to deal with any parameters etc
		default: return false; // Some syntax error occurred. Abort
	}

    start_iconified = parser.Found(wxT("i"));
    if (parser.GetParamCount())
    {
        wxString dialnumber = parser.GetParam(0);
        IpcClient client;
        if ( client.Connect("localhost", IPC_SERVICENAME,  IPC_TOPIC) )
        {
            client.GetConnection()->Execute(dialnumber);
            client.Disconnect();
        }
        return false;
    }
    return true;
}
