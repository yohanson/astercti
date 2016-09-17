#ifndef _MYAPP_H_
#define _MYAPP_H_

#include <wx/fileconf.h>
#include <wx/cmdline.h>
#include "controller.h"
#include "taskbaricon.h"
#include "ipc.h"
#include "events.h"
#include "filter.h"
#include "chanstatus.h"

class MyApp: public wxApp
{
public:
    ~MyApp();
    virtual bool OnInit();
    virtual int OnExit();
    virtual int OnRun();
    virtual bool ParseCmdLine();
    virtual void OnFatalException();
    AsteriskController *m_controller;
    wxFileConfig *m_config;
    wxLocale m_locale;
    MyTaskBarIcon *m_taskbaricon;
private:
    bool m_start_gui;
    int m_exit_code;
    bool start_iconified;
    IpcServer *m_ipcServer;
    MyChanFilter *m_mychanfilter;
    InternalMessageFilter *m_intmsgfilter;
    ShortenNumberModifier *m_numbershortener;
    ChannelStatusPool   *m_chanstatuspool;
    EventGenerator *m_events;
};

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
     { wxCMD_LINE_SWITCH,"h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
     { wxCMD_LINE_SWITCH,"v", "version", "show version" },
     { wxCMD_LINE_SWITCH, "i", "iconified", _("do not show the window upon startup") },
     { wxCMD_LINE_PARAM,  NULL, NULL, _("phone to call"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
     { wxCMD_LINE_NONE }
};

extern const char *gitcommit;
extern const char *gitcommitdate;
extern const char *builddate;

#endif
