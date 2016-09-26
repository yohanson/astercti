#ifndef _LOOKUP_H_
#define _LOOKUP_H_

#include <json/value.h>

class CallerInfoLookuper
{
public:
    CallerInfoLookuper(const std::string &src);
    bool ShouldLookup(const std::string &callerid);
    wxString GetHtml(const std::string &callerid);
    wxString GetField(const std::string &callerid, const std::string &path);
protected:
    Json::Value GetJson(const std::string &callerid);
    bool IsCached(const std::string &callerid);
    void Cache(const std::string &callerid, const Json::Value &json);
    virtual wxString Lookup(const std::string &) = 0;
    Json::Value GetCache();
    wxString m_src;
    wxString m_lastCallerid;
    Json::Value m_lastJson;
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
