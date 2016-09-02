#include <wx/debugrpt.h>
#include "debugreport.h"
#include "myapp.h"

wxDECLARE_APP(MyApp);

MyDebugReport::MyDebugReport(const wxString& url) : wxDebugReportUpload
                       (
                        url,
                        wxT("reportfile"),
                        wxT("")
                       )
{
}

bool MyDebugReport::OnServerReply(const wxArrayString& reply)
{
    if ( reply.IsEmpty() )
    {
        wxLogError(wxT("Didn't receive the expected server reply."));
        return false;
    }

    wxString s(wxT("Server replied:\n"));

    const size_t count = reply.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        s << wxT('\t') << reply[n] << wxT('\n');
    }

    wxSafeShowMessage("Report upload", s.c_str());

    return true;
}
