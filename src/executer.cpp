#include <wx/process.h>
#include <wx/txtstrm.h>

#include "executer.h"
#include "myapp.h"

Executer::Executer(CallerInfoLookuper *lookuper)
    : m_lookuper(lookuper)
{
    m_cmd = wxGetApp().m_config->Read("hooks/exec_ondialin");
    DEBUG_MSG("Got cmd: " << m_cmd << std::endl);
}

void Executer::OnDialIn(const AmiMessage &m)
{
    std::cerr << "Executer OnDialIn!" << std::endl;
    wxString email = m_lookuper->GetField(m["CallerIDNum"], "clients/0/emails/0");
    wxString name = m_lookuper->GetField(m["CallerIDNum"], "clients/0/name");
    DEBUG_MSG("Got email: " << email << std::endl);
    DEBUG_MSG("Got name: " << name << std::endl);
    wxString cmd;

    cmd.Printf(m_cmd, email);
    DEBUG_MSG("Cmd: " << cmd << std::endl);

    wxExecute(cmd);
}

