#include <wx/string.h>
#include <wx/process.h>
#include <wx/txtstrm.h>
#include <json/value.h>
#include <json/reader.h>
#include <curl/curl.h>
#include <regex>
#include "lookup.h"
#include "myapp.h"
#include "version.h"


static void FindAndReplace(std::string &tmpl, std::string varname, std::string value)
{
    size_t start_pos = 0;
    size_t found_pos;
    while ((found_pos = tmpl.find(varname, start_pos)) != std::string::npos)
    {
        start_pos = found_pos + 1;
        tmpl.replace(found_pos, varname.length(), value);
    }
}


CallerInfoLookuper::CallerInfoLookuper(const std::string &src)
    : m_src(src)
{
}

CallerInfoLookuperCmd::CallerInfoLookuperCmd(const std::string &src)
    : CallerInfoLookuper(src)
{
}

CallerInfoLookuperURL::CallerInfoLookuperURL(const std::string &src)
    : CallerInfoLookuper(src)
{
}

Json::Value CallerInfoLookuper::GetJson(const std::string &callerid)
{
    if (IsCached(callerid))
    {
        return GetCache();
    }
    wxString json = Lookup(callerid);
    Json::Value root;
    Json::Reader reader;
    std::string outstring = std::string(json.mb_str());
    if (!reader.parse(outstring, root, false))
    {
        std::cerr << "Failed to parse JSON: " << std::endl
               << reader.getFormattedErrorMessages() <<std::endl
               << "----" << std::endl
               << json << std::endl;
    }
    Cache(callerid, root);
    return root;
}

bool CallerInfoLookuper::ShouldLookup(const std::string &callerid)
{
    bool should = false;
    if (!callerid.empty()
        && !m_src.empty()
        && callerid != "<unknown>"
        && callerid != wxGetApp().GetMyExten())
    {
        std::string regex = wxGetApp().Cfg("lookup/number_match_regex");
        if (!regex.empty())
        {
            try {
                should = std::regex_match(callerid, std::regex(regex));
                std::cerr << "Regex: '" << regex << "' matches: " << should << std::endl;
            } catch (std::regex_error)
            {
                std::cerr << "Regex only implemented in stdc++ 4.9 or later." << std::endl
                    << "Try to use number_min_length option in astercti.ini" << std::endl;
            }
        }
        else
        {
            int number_length = wxGetApp().CfgInt("lookup/number_min_length");
            if (number_length && callerid.length() >= number_length)
                should = true;
        }
    }
    return should;
}

/*
 * path example: "clients/0/name"
 */
wxString CallerInfoLookuper::GetField(const std::string &callerid, const std::string &path)
{
    Json::Value current = GetJson(callerid);
    const static char delimiter = '/';
    std::stringstream s(path);
    std::string token;
    while (std::getline(s, token, delimiter))
    {
        if (current.isObject())
        {
            current = current[token];
        }
        else if (current.isArray())
        {
            int index = atoi(token.c_str());
            current = current[index];
        }
        if (current.isNull())
        {
            std::cerr << "Cannot find field '" << token << "' in JSON.\n";
            return wxEmptyString;
        }
    }
    if (current.isConvertibleTo(Json::ValueType::stringValue))
    {
        return current.asString();
    }
    else
    {
        return wxEmptyString;
    }
}

wxString CallerInfoLookuper::GetHtml(const std::string &callerid)
{
    Json::Value root = GetJson(callerid);
    const Json::Value clients = root["clients"];
    wxString html;
    std::string url_template = wxGetApp().Cfg("templates/client_url");
    for ( int i = 0; i < clients.size() && i < 3; ++i )
    {
        std::string url = url_template;
        Json::Value::Members client_attrs = clients[i].getMemberNames();
        for (unsigned int attr_index = 0; attr_index < client_attrs.size(); ++attr_index)
        {
            std::string variable_name = client_attrs[attr_index];
            std::string template_name = "${" + variable_name + "}";
            if (clients[i][variable_name].isConvertibleTo(Json::ValueType::stringValue))
                FindAndReplace(url, template_name, clients[i][variable_name].asString());
        }
        html += "<a href='"+ url +"'>" +clients[i]["name"].asString() + "</a><br />";
        if (clients[i]["services"].size())
        {
            html += "<ul>";
            for (int srv = 0; srv < clients[i]["services"].size(); ++srv)
            {
                html += "<li>" + clients[i]["services"][srv].asString() + "</li>";
            }
            html += "</ul>";
        }
        html += "<br />";
        if (i+1 < clients.size())
            html += "<br />";
    }
    if (clients.size() > 3)
    {
        html += "...<br/>";
    }
    if (clients.size() > 1)
    {
        wxString url_tpl = wxGetApp().Cfg("lookup/search_url");
        wxString url;
        url.Printf(url_tpl, callerid);
        html << "<hr size=1 noshade /><a href='" << url << "'>" << _("See all found") << " (" << clients.size() << ")</a>";
    }
    else if (clients.size() == 0)
    {
        html << _("Unknown number.");
    }

    return html;
}

bool CallerInfoLookuper::IsCached(const std::string &callerid)
{
    if (m_cachedAt.IsValid())
    {
        return (wxDateTime::Now() - m_cachedAt).GetSeconds() < 60
            && callerid == m_lastCallerid;
    }
    return false;
}

void CallerInfoLookuper::Cache(const std::string &callerid, const Json::Value &json)
{
    m_cachedAt = wxDateTime::Now();
    m_lastCallerid = callerid;
    m_lastJson = json;
}

Json::Value CallerInfoLookuper::GetCache()
{
    return m_lastJson;
}

// CallerInfoLookuperURL: ====================================

// Definitions for CURL: -------------------------
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}
//------------------------------------------------

wxString CallerInfoLookuperURL::Lookup(const std::string &callerid)
{
    wxString out;
    wxArrayString output;

    if (!m_src.empty())
    {
        CURL *curl;
        CURLcode res;
        struct MemoryStruct chunk;
        chunk.memory = NULL;
        chunk.size = 0;
        curl = curl_easy_init();
        if (curl)
        {
            char *url = new char[ m_src.length() + callerid.length() + 1 ];
            sprintf(url, m_src.c_str(), callerid.c_str());
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "astercti/" VERSION);
            if (wxGetApp().CfgBool("lookup/curl_insecure", false))
            {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
            }
#ifdef __WXMSW__
            else
            {
                curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
            }
#endif
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                wxLogError("curl failed: %s", curl_easy_strerror(res));
            }
            else
            {
                out = wxString::FromUTF8((const char *)chunk.memory);
            }
            curl_easy_cleanup(curl);
            free(chunk.memory);
            delete[] url;
            return out;
        }
        return "Curl Error";
    }
    return wxEmptyString;
}
//================================================
wxString CallerInfoLookuperCmd::Lookup(const std::string &callerid)
{
    wxString cmd;
    wxArrayString output;
    wxString out;
    if (!m_src.empty())
    {
        cmd.Printf(wxString(m_src), callerid);
        ExecCommand(cmd, output);
        for (auto iter : output)
        {
            out += iter;
        }
        return out;
    }
    return wxEmptyString;
}
void CallerInfoLookuperCmd::ExecCommand(wxString &cmd, wxArrayString &output)
{
    wxProcess p;
    p.Redirect();
    wxExecute(cmd, wxEXEC_SYNC, &p);

    wxInputStream *i = p.GetInputStream();
    if(i)
    {
        wxTextInputStream t(*i, " \t", wxConvUTF8);

        while(!i->Eof())
        {
            output.Add(t.ReadLine());
        }
    }
    if (p.IsErrorAvailable())
    {
        wxInputStream *e = p.GetErrorStream();
        wxString error_string;
        while (!e->Eof())
        {
            wxTextInputStream t(*e, " \t", wxConvUTF8);
            error_string = t.ReadLine();
            if (!error_string.empty())
            {
                std::cerr << "error: '" << error_string << "'" << std::endl;
            }
        }
    }
}

