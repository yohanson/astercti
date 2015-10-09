#ifndef _MYAPP_H_
#define _MYAPP_H_

#include <wx/fileconf.h>
#include "controller.h"
#include "taskbaricon.h"

class MyApp: public wxApp
{
public:
    ~MyApp();
    virtual bool OnInit();
    AsteriskController *m_controller;
    wxFileConfig *m_config;
    wxLocale m_locale;
    MyTaskBarIcon *m_taskbaricon;
};

#endif
