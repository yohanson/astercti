#ifdef WX_PRECOMP
	#include <wx/wxprec.h>
#else
	#include <wx/wx.h>
#endif

#include <wx/string.h>
#include <locale>
#include <wx/intl.h>
#include <wx/stdpaths.h>
#include <wx/debugrpt.h>

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
#include "debugreport.h"
#include "iconmacro.h"

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
#ifndef __WXMSW__
    if (wxPlatformInfo::Get().GetOperatingSystemId() & wxOS_UNIX)
    {
        wxStandardPaths::Get().SetInstallPrefix("/usr");
    }
#endif
    wxString datadir = wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator();
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

    ChannelStatusPool *chanstatuspool = new ChannelStatusPool(m_config->Read("dialplan/channel").ToStdString());
    MyFrame *frame = new MyFrame( "AsterCTI", wxDefaultPosition, wxSize(600, 400), chanstatuspool);
    Asterisk *asterisk = new Asterisk(m_config->Read("server/address").ToStdString(),
		5038,
		m_config->Read("server/username").ToStdString(),
		m_config->Read("server/password").ToStdString());
    m_controller = new AsteriskController(asterisk, m_config);
    m_controller->SetMainFrame(frame);
    m_mychanfilter = new MyChanFilter(m_config->Read("dialplan/channel").ToStdString());
    m_intmsgfilter = new InternalMessageFilter();
    m_numbershortener = new ShortenNumberModifier(m_config->Read("lookup/replace_number_prefix").ToStdString());
    asterisk->observable_descr = "asterisk";
    m_mychanfilter->observable_descr = "mychanfilter";
    m_intmsgfilter->observable_descr = "intmsgfilter";
    asterisk->add(*m_mychanfilter);
    asterisk->add(*m_intmsgfilter);
    asterisk->add(*chanstatuspool);
    m_mychanfilter->add(*m_numbershortener);
    m_numbershortener->add(*frame);
    m_intmsgfilter->add(*frame);
    notificationFrame *notifyframe = new notificationFrame(frame, chanstatuspool);
    m_events = new EventGenerator;
    m_events->add(*frame);
    m_events->add(*notifyframe);
    m_numbershortener->add(*m_events);
    m_intmsgfilter->add(*m_events);
    wxIcon defaultIcon(ACTI_ICON("astercti"));
    wxIcon  missedIcon(ACTI_ICON("astercti-missed"));
    frame->SetIcon(defaultIcon);
    m_taskbaricon = new MyTaskBarIcon(defaultIcon, missedIcon, "AsterCTI: " + m_config->Read("dialplan/exten"));
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
        frame->Log(_("Lookup URL and Lookup command are both unconfigured.\nLookup disabled."));
    }
    else
    {
        notifyframe->SetLookupCmd(m_config->Read("lookup/lookup_cmd").ToStdString());
        notifyframe->SetLookupUrl(m_config->Read("lookup/lookup_url").ToStdString());
    }
    m_ipcServer = new IpcServer(m_controller);
    if (!m_ipcServer->Create(IPC_SERVICENAME))
        wxLogMessage("Failure creating IPC Server %s", IPC_SERVICENAME);
#ifndef __WXMSW__
    wxHandleFatalExceptions();
#endif
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
    IpcClient client;
    bool ipcClientConnected =  client.Connect("localhost", IPC_SERVICENAME,  IPC_TOPIC);
    if (parser.GetParamCount())
    {
        wxString dialnumber = parser.GetParam(0);
        if (ipcClientConnected)
        {
            client.GetConnection()->Execute(dialnumber);
            client.Disconnect();
        }
        return false;
    }
    else // no command line params
    {
        if (ipcClientConnected)
        {
            client.GetConnection()->Execute(IPC_CMD_RISE);
            client.Disconnect();
            return false;
        }
    }
    return true;
}

void MyApp::OnFatalException()
{
    wxString url = m_config->Read("gui/debugreport_url");
    wxDebugReportCompress *report;
    if (url.empty())
    {
        wxSafeShowMessage(_("debugreport_url not configured!"), _("Cannot send crash report.\nPlease, edit config to include [gui] section and 'debugreport_url=http://example.com/debugreport/' — your url of debug file receiver in it."));
        report = new wxDebugReportCompress;
        //report->SetCompressedFileDirectory(wxStandardPaths::Get().GetDataDir());
    }
    else
    {
        report = new MyDebugReport(url);
    }

    report->AddAll(wxDebugReport::Context_Exception);
    report->AddFile(m_config->GetLocalFileName("astercti.ini", wxCONFIG_USE_SUBDIR), "AsterCTI Settings");
    wxString versiontext;
    versiontext << "AsterCTI v" VERSION "\n"
        << "Commit " << gitcommit << " " << gitcommitdate << "\n"
        << "Built " << builddate << "\n";
    report->AddText(wxString("version.txt"), versiontext, _("Version information"));

    wxString result;
    if ( report->Process() )
    {
        if (!url.empty())
            result = "Report successfully uploaded.";
        else
            result = "Report saved as '" + report->GetCompressedFileName() + "'.";

    }
    else
    {
        if (!url.empty())
            result = "Report failed to upload.";
        else
            result = "Report failed to save.";

    }
    wxSafeShowMessage("AsterCTI Crash Report", result);
    delete report;
}
