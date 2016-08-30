#ifndef _DEBUGREPORT_H_
#define _DEBUGREPORT_H_

class MyDebugReport : public wxDebugReportUpload
{
public:
    MyDebugReport(const wxString& url);
protected:
    virtual bool OnServerReply(const wxArrayString& reply);
};

#endif
