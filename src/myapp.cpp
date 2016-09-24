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
#include "utils.h"

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

    m_start_gui = ParseCmdLine();
    if (!m_start_gui) return true;

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

    m_chanstatuspool = new ChannelStatusPool(m_config->Read("dialplan/channel").ToStdString());
    if (m_config->HasEntry("lookup/lookup_url"))
    {
        m_lookuper = new CallerInfoLookuperURL(Cfg("lookup/lookup_url"));
    }
    else if (m_config->HasEntry("lookup/lookup_cmd"))
    {
        m_lookuper = new CallerInfoLookuperCmd(Cfg("lookup/lookup_cmd"));
    }
    else
    {
        m_lookuper = NULL;
    }
    wxPoint pos = m_config->ReadObject("autosave/position", wxDefaultPosition);
    wxSize size = m_config->ReadObject("autosave/size", wxSize(600, 400));
    asterisk = new Asterisk(m_config->Read("server/address").ToStdString(),
		5038,
		m_config->Read("server/username").ToStdString(),
		m_config->Read("server/password").ToStdString());
    bool maximized = m_config->ReadBool("autosave/maximized", false);
    m_mainframe = new MyFrame( "AsterCTI", pos, size, m_chanstatuspool, asterisk, m_lookuper);
    if (maximized) m_mainframe->Maximize();
    m_mychanfilter = new MyChanFilter(m_config->Read("dialplan/channel").ToStdString());
    m_intmsgfilter = new InternalMessageFilter();
    m_numbershortener = new ShortenNumberModifier(m_config->Read("lookup/replace_number_prefix").ToStdString());
    asterisk->observable_descr = "asterisk";
    m_mychanfilter->observable_descr = "mychanfilter";
    m_intmsgfilter->observable_descr = "intmsgfilter";
    asterisk->broadcast(*m_mychanfilter);
    asterisk->broadcast(*m_intmsgfilter);
    asterisk->broadcast(*m_chanstatuspool);
    m_mychanfilter->broadcast(*m_numbershortener);
    m_numbershortener->broadcast(*m_mainframe);
    m_intmsgfilter->broadcast(*m_mainframe);
    notificationFrame *notifyframe = new notificationFrame(m_mainframe, m_chanstatuspool, asterisk, m_lookuper);
    m_events = new EventGenerator(m_config->Read("dialplan/exten").ToStdString());
    m_events->broadcast(*m_mainframe);
    m_events->broadcast(*notifyframe);
    m_numbershortener->broadcast(*m_events);
    m_intmsgfilter->broadcast(*m_events);
    wxIcon defaultIcon(ACTI_ICON("astercti"));
    wxIcon  missedIcon(ACTI_ICON("astercti-missed"));
    m_mainframe->SetIcon(defaultIcon);
    m_taskbaricon = new MyTaskBarIcon(defaultIcon, missedIcon, "AsterCTI: " + m_config->Read("dialplan/exten"));
    m_taskbaricon->SetMainFrame(m_mainframe);
    m_mainframe->SetTaskBarIcon(m_taskbaricon);
    m_mainframe->Show(!start_iconified);
    SetTopWindow(m_mainframe);
    SetExitOnFrameDelete(true);
    if (!m_lookuper)
    {
        m_mainframe->Log(_("Lookup URL and Lookup command are both unconfigured.\nLookup disabled."));
    }
    m_ipcServer = new IpcServer();
    if (!m_ipcServer->Create(IPC_SERVICENAME))
        wxLogMessage("Failure creating IPC Server %s", IPC_SERVICENAME);
#ifndef __WXMSW__
    wxHandleFatalExceptions();
#endif
    return true;
}

int MyApp::OnRun()
{
    if (m_start_gui)
    {
        m_exit_code = wxApp::OnRun();
    }
    return m_exit_code;
}

MyApp::~MyApp()
{
}

int MyApp::OnExit()
{
    delete m_events;
    delete m_mychanfilter;
    delete m_intmsgfilter;
    delete m_numbershortener;
    delete m_config;
    delete m_ipcServer;
    delete m_chanstatuspool;
    return 0;
}

bool MyApp::ParseCmdLine()
{
	wxCmdLineParser parser(g_cmdLineDesc, argc, argv);
	switch ( parser.Parse() )
    {
        case -1: return false; // The parameter -h was passed, help was given, so abort the app
	    case  0: break; // OK, so break to deal with any parameters etc
        default: m_exit_code = 1;
                 return false; // Some syntax error occurred. Abort
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
        wxSafeShowMessage(_("debugreport_url not configured!"), _("Cannot send crash report.\nPlease, edit config to include [gui] section and 'debugreport_url=http://example.com/debugreport/' - your url of debug file receiver in it."));
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

std::string MyApp::Cfg(std::string s)
{
    if (!m_config)
        return "";
    return m_config->Read(s).ToStdString();
}

long MyApp::CfgInt(std::string s)
{
    if (!m_config)
           return 0;
    long val;
    m_config->Read(s, &val);
    return val;
}

bool MyApp::CfgBool(std::string s, bool def)
{
    if (!m_config)
           return def;
    return m_config->ReadBool(s, def);
}

std::string MyApp::GetMyExten()
{
    return Cfg("dialplan/exten");
}

std::string MyApp::GetMyChannel()
{
    return Cfg("dialplan/channel");
}

void MyApp::ShowMainFrame() {
    if (m_mainframe)
    {
        m_mainframe->Show(true);
        m_mainframe->Raise();
    }
}

void MyApp::Originate(const std::string &number)
{
    asterisk->Originate(GetMyChannel(), Cfg("dialplan/context"), number, GetMyExten());
}

void MyApp::HangupChannel(const std::string &channel)
{
    asterisk->HangupChannel(channel);
}
