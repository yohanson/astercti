#ifndef _MYAPP_H_
#define _MYAPP_H_

#include <wx/fileconf.h>
#include <wx/cmdline.h>
#include "controller.h"
#include "taskbaricon.h"

class MyApp: public wxApp
{
public:
    ~MyApp();
    virtual bool OnInit();
    virtual bool ParseCmdLine();
    AsteriskController *m_controller;
    wxFileConfig *m_config;
    wxLocale m_locale;
    MyTaskBarIcon *m_taskbaricon;
private:
    bool start_iconified;
};

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
     { wxCMD_LINE_SWITCH,"h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
     { wxCMD_LINE_SWITCH, "i", "iconified", _("do not show the window upon startup") },
     { wxCMD_LINE_PARAM,  NULL, NULL, _("phone to call"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
     { wxCMD_LINE_NONE }
};

#endif
