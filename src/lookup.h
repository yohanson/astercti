#ifndef _LOOKUP_H_
#define _LOOKUP_H_

class CallerInfoLookuper
{
public:
    CallerInfoLookuper(const std::string &src);
    bool ShouldLookup(const std::string &callerid);
    virtual wxString Lookup(const std::string &) = 0;
protected:
    bool IsCached(const std::string &callerid);
    void Cache(const std::string &callerid, const wxString &html);
    wxString GetCache();
    wxString ParseJson(const wxString &json, const wxString &callerid);
    wxString m_src;
    wxString m_lastCallerid;
    wxString m_lastHtml;
    wxDateTime m_cachedAt;
};

// =================================

class CallerInfoLookuperCmd : public CallerInfoLookuper
{
public:
    CallerInfoLookuperCmd(const std::string &src);
    virtual wxString Lookup(const std::string &);
protected:
    void ExecCommand(wxString &cmd, wxArrayString &output);
};

// ----------------------------------

class CallerInfoLookuperURL : public CallerInfoLookuper
{
public:
    CallerInfoLookuperURL(const std::string &src);
    virtual wxString Lookup(const std::string &);
};

#endif
