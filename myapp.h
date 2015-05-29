#ifndef _MYAPP_H_
#define _MYAPP_H_

#include <wx/fileconf.h>
#include "controller.h"

class MyApp: public wxApp
{
public:
    virtual bool OnInit();
    AsteriskController *m_controller;
    wxFileConfig *m_config;
};

#endif
