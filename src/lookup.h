#ifndef _LOOKUP_H_
#define _LOOKUP_H_

class CallerInfoLookuper
{
public:
    CallerInfoLookuper(const std::string &src);
    bool ShouldLookup(const std::string &callerid);
    virtual wxString Lookup(const std::string &) = 0;
        
protected:
    wxString ParseJson(const wxString &, const wxString &);
    wxString m_src;
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

class CallerInfoLookuperURL : public CallerInfoLookuper
{
public:
    CallerInfoLookuperURL(const std::string &src);
    virtual wxString Lookup(const std::string &);
};

#endif
