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
#include "version.h"
#include "chanstatus.h"

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
    wxString datadir = wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator();
#ifndef __WXMSW__
    if (wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_UNIX)
    {
        wxStandardPaths::Get().SetInstallPrefix("/usr");
        datadir = wxStandardPaths::Get().GetInstallPrefix() + "/share/pixmaps";
    }
#endif
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
        wxLogError("%s", msg.str());
        return false;
    }

    MyFrame *frame = new MyFrame( "AsterCTI", wxDefaultPosition, wxSize(600, 400) );
    Asterisk *asterisk = new Asterisk(m_config->Read("server/address").ToStdString(),
		5038,
		m_config->Read("server/username").ToStdString(),
		m_config->Read("server/password").ToStdString());
    m_controller = new AsteriskController(asterisk, m_config);
    m_controller->SetMainFrame(frame);
    m_mychanfilter = new MyChanFilter(m_config->Read("dialplan/channel").ToStdString());
    m_intmsgfilter = new InternalMessageFilter();
    ChannelStatusPool *chanstatuspool = new ChannelStatusPool();
    asterisk->observable_descr = "asterisk";
    m_mychanfilter->observable_descr = "mychanfilter";
    m_intmsgfilter->observable_descr = "intmsgfilter";
    asterisk->add(*m_mychanfilter);
    asterisk->add(*m_intmsgfilter);
    asterisk->add(*chanstatuspool);
    m_mychanfilter->add(*frame);
    m_intmsgfilter->add(*frame);
    notificationFrame *notifyframe = new notificationFrame(frame);
    m_events = new EventGenerator;
    m_events->add(*frame);
    m_events->add(*notifyframe);
    m_mychanfilter->add(*m_events);
    m_intmsgfilter->add(*m_events);
    wxString iconfile = datadir + wxFileName::GetPathSeparator() + "astercti.png";
    wxIcon iconimage(iconfile, wxBITMAP_TYPE_PNG);
    frame->SetIcon(iconimage);
    m_taskbaricon = new MyTaskBarIcon(iconimage, "AsterCTI: " + m_config->Read("dialplan/exten"));
    m_taskbaricon->SetMainFrame(frame);
    frame->SetTaskBarIcon(m_taskbaricon);
    m_controller->add(m_taskbaricon);
    m_controller->add(frame);
    m_controller->add(notifyframe);
    m_controller->add(m_events);
    frame->Show(!start_iconified);
    SetTopWindow(frame);
    SetExitOnFrameDelete(true);
    if (!m_config->Read("lookup/lookup_cmd") && !m_config->Read("lookup/lookup_url"))
    {
        wxLogWarning(_("Lookup URL and Lookup command are both unconfigured.\nLookup disabled."));
    }
    else
    {
        notifyframe->SetLookupCmd(m_config->Read("lookup/lookup_cmd").ToStdString());
        notifyframe->SetLookupUrl(m_config->Read("lookup/lookup_url").ToStdString());
    }
    m_ipcServer = new IpcServer(m_controller);
    m_ipcServer->Create(IPC_SERVICENAME);
    return true;
}

MyApp::~MyApp()
{
}

int MyApp::OnExit()
{
    delete m_events;
    delete m_mychanfilter;
    delete m_intmsgfilter;
    delete m_config;
    delete m_ipcServer;
    delete m_controller;
    return 0;
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

    if (parser.Found("v"))
    {
        std::cout << "AsterCTI v" << VERSION << std::endl;
        std::cout << "Commit " << gitcommit << " " << gitcommitdate << std::endl;
        std::cout << "Built " << builddate << std::endl;
        std::cout << "https://github.com/yohanson/astercti" << std::endl;
        return false;
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
