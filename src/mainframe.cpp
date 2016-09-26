#include <stdexcept>
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/listctrl.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/aboutdlg.h>

#include "observer.h"
#include "asterisk.h"
#include "myapp.h"
#include "calllistctrl.h"
#include "mainframe.h"
#include "version.h"
#include "call.h"
#include "chanstatus.h"
#include "iconmacro.h"
#include "utils.h"

#define CALLS_FILE "calls.txt"


MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size, ChannelStatusPool *pool, Asterisk *a, CallerInfoLookuper *lookuper, const std::string& lookup_field)
        : wxFrame(NULL, wxID_ANY, title, pos, size),
          ChannelStatusPooler(pool),
          TopMostVerticalSplitter(this),
          asterisk(a),
          m_lookuper(lookuper),
          m_lookup_field(lookup_field)
{
    edescr = "mainframe";
    m_taskbaricon = NULL;
    m_last_channel_state = AST_STATE_DOWN;
    m_missed_calls = 0;

    wxImage::AddHandler(new wxPNGHandler);
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );
    menuBar->Append( menuHelp, "&Help" );
    SetMenuBar( menuBar );

    wxString datadir = wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator();
    wxImageList *imagelist = new wxImageList(24, 24, true);
    imagelist->Add(ACTI_ICON("incoming_answered"));
    imagelist->Add(ACTI_ICON("incoming_unanswered"));
    imagelist->Add(ACTI_ICON("outbound_answered"));
    imagelist->Add(ACTI_ICON("outbound_unanswered"));
    imagelist->Add(ACTI_ICON("incoming_answered_elsewhere"));
    m_dialIcon.CopyFromIcon(ACTI_ICON_SIZED("dial", 24));
    m_hangupIcon.CopyFromIcon(ACTI_ICON_SIZED("hangup", 24));

    TopMostVerticalSplitter.SetMinSize(wxSize(100,100));
    TopMostVerticalSplitter.SetMinimumPaneSize(100);
    TopMostVerticalSplitter.SetSashGravity(0);
    long splitter_pos = wxGetApp().m_config->ReadLong("autosave/splitter_position", 0);

    wxBoxSizer *RightSizer = new wxBoxSizer(wxVERTICAL);
    wxPanel *RightPanel = new wxPanel(&TopMostVerticalSplitter);
    RightPanel->SetSizer(RightSizer);
    wxBoxSizer *DialSizer = new wxBoxSizer(wxHORIZONTAL);
    m_DialNumber = new wxTextCtrl(RightPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    wxFont numberFont(wxSize(0,24), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_DialNumber->SetFont(numberFont);
    m_DialButton = new wxBitmapButton(RightPanel, wxID_ANY, m_dialIcon, wxDefaultPosition, wxSize(36,36), wxBU_AUTODRAW);
    DialSizer->Add(m_DialNumber, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    DialSizer->Add(m_DialButton, 0, wxALL|         wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StatusText = new wxTextCtrl(RightPanel, ID_TextCtlNumber, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    m_CallInfo = new wxHtmlWindow(RightPanel, wxID_ANY, wxDefaultPosition, wxSize(400,300), wxHW_NO_SELECTION);
    m_CallInfo->SetBackgroundColour(wxNullColour);
    m_callList = new CallListCtrl(&TopMostVerticalSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
    m_callList->SetTimeFormat(wxGetApp().m_config->Read("gui/call_list_time_format", "%a %d %b %H:%M"));
    m_callList->AssignImageList(imagelist, wxIMAGE_LIST_SMALL);
    if (!LoadCalls(CALLS_FILE))
        std::cerr << _("Loading calls failed") << std::endl;
    RightSizer->Add(DialSizer, 0, wxEXPAND);
    RightSizer->Add(m_CallInfo, 0, wxEXPAND);
    RightSizer->Add(StatusText, 1, wxEXPAND);
    TopMostVerticalSplitter.SplitVertically(m_callList, RightPanel, splitter_pos);

    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnExit));
    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::OnAbout));
    Bind(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MyFrame::OnClose), this);
    Bind(wxEVT_ACTIVATE, wxActivateEventHandler(MyFrame::OnActivate), this);
    Bind(wxEVT_HTML_LINK_CLICKED, &MyFrame::OnLinkClicked, this);
    m_DialNumber->Bind(wxEVT_TEXT_ENTER, &MyFrame::OnDialPressEnter, this);
    m_DialNumber->Bind(wxEVT_TEXT, &MyFrame::OnDialTextChange, this);
    m_DialButton->Bind(wxEVT_BUTTON, &MyFrame::OnDialPressEnter, this);
    m_callList->Bind(wxEVT_LIST_ITEM_SELECTED, &MyFrame::OnListItemSelect, this);

    CreateStatusBar();
    m_DialNumber->SetFocus();
}

MyFrame::~MyFrame()
{
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxString buildinfo;
    buildinfo << "Git commit: " << gitcommit << "\n"
        << "Built: " << builddate << "\n";
    wxAboutDialogInfo info;
    info.SetName("AsterCTI");
    info.SetVersion(FULLVERSION);
    info.SetDescription(_("Computer-Telephone Integration app for Asterisk.") + "\n\n" + buildinfo);
    info.SetCopyright(wxT("(C) 2015-2016"));
    info.AddDeveloper("Michael Mikhailov <yohanson@ngs.ru>");
    info.SetWebSite("https://github.com/yohanson/astercti");
    wxAboutBox(info);
}
void MyFrame::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        Show(false);
        event.Veto();
        return;
    }
    if (!SaveCalls(CALLS_FILE))
        std::cerr << _("Saving calls failed") << std::endl;
    SavePosition();
    m_taskbaricon->Destroy();
    Destroy();
}

void MyFrame::OnActivate(wxActivateEvent &event)
{
    if (m_missed_calls)
    {
        m_missed_calls = 0;
        m_taskbaricon->SetMissedCalls(0);
    }
}

void MyFrame::OnLinkClicked(wxHtmlLinkEvent& event)
{
    wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
}

void MyFrame::SavePosition()
{
    wxPoint p = GetPosition();
    wxSize s = GetSize();
    long splitter_pos = TopMostVerticalSplitter.GetSashPosition();
    wxFileConfig *cfg = wxGetApp().m_config;
    cfg->Write("autosave/maximized", IsMaximized());
    if (!IsMaximized())
    {
        cfg->Write("autosave/position", p);
        cfg->Write("autosave/size", s);
    }
    cfg->Write("autosave/splitter_position", splitter_pos);
}

void MyFrame::OnDialPressEnter(wxCommandEvent &event)
{
    if (m_last_channel_state == AST_STATE_DOWN)
    {
        if (!m_DialNumber->GetValue().IsEmpty())
        {
            std::string raw = m_DialNumber->GetValue().ToStdString();
            std::string digits;
            for (auto it = raw.begin(); it != raw.end(); ++it)
            {
                if (*it >= '0' && *it <= '9')
                {
                    digits += *it;
                }
            }
            wxGetApp().Originate(digits);
        }
    }
    else
    {
        asterisk->HangupChannel(m_current_channel);
    }
}

void MyFrame::OnDialTextChange(wxCommandEvent &event)
{
    if (m_CallInfo->ToText().Length())
    {
        m_CallInfo->SetPage("");
        m_CallInfo->SetBackgroundColour(wxNullColour);
    }
}

void MyFrame::UpdateDialButtonImage()
{
    if (m_last_channel_state == AST_STATE_DOWN)
        m_DialButton->SetBitmap(m_dialIcon);
    else
        m_DialButton->SetBitmap(m_hangupIcon);
}

void MyFrame::OnListItemSelect(wxListEvent &event)
{
    Call *call = reinterpret_cast<Call *>(event.GetData());
    m_DialNumber->SetValue(call->GetNumber());
    wxString label;
    wxDateTime duration((time_t)call->GetDuration());
    wxString timeformat;
    if (call->GetDuration() >= 3600)
        timeformat = "%H:%M:%S";
    else timeformat = "%M:%S";
    label << _("Number: ") << call->GetNumber() << "<br>\n";
    if (!call->GetName().empty())
    {
        label << _("Name: ") << call->GetName() << "<br>\n";
    }
    label << _("Dial: ") << call->GetTimeStart().FormatISOCombined(' ') << "<br>\n";
    if (call->GetTimeAnswer().IsValid())
    {
        label << _("Answer: ") << call->GetTimeAnswer().FormatISOCombined(' ') << "<br>\n";
    }
    if (call->GetTimeEnd().IsValid())
    {
        label << _("End: ") << call->GetTimeEnd().FormatISOCombined(' ') << "<br>\n";
    }
    if (call->GetDisposition() == Call::CALL_ANSWERED)
    {
        label << _("Duration: ") << duration.Format(timeformat, wxDateTime::UTC) << "<br>\n";
    }
    label << call->GetDescription();
    wxString fgcolor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT).GetAsString();
    m_CallInfo->SetPage("<body text='"+fgcolor+"'>"+label+"</body>");
    m_CallInfo->SetBackgroundColour(wxNullColour);
}


void MyFrame::SetTaskBarIcon(MyTaskBarIcon *taskbaricon)
{
    m_taskbaricon = taskbaricon;
}

void MyFrame::handleEvent(const AmiMessage &message)
{
    if (message.has("InternalMessage")) return;
    if (message.has("Response")) return;
    if (StatusText->GetNumberOfLines() > LOG_MAX_LINES)
    {
        int chars = 0;
        for (int i = 0; i<LOG_DELETE_LINES; i++)
        {
            chars += StatusText->GetLineLength(i)+1;
        }
        StatusText->Remove(0, chars);
    }
    std::string status;
    for (auto iter : message)
    {
        status += iter.first + ": " + iter.second + "\n";
    }
    Log(status);
}

void MyFrame::OnOriginate(const AmiMessage &m)
{
    Log("##### We are originating! #####\n");
    Call *call = new Call;
    call->SetNumber(m["ConnectedLineName"]); // yes, it's right
    call->SetUniqueID(std::stoi(m["Uniqueid"]));
    call->SetTimeStart(wxDateTime::Now());
    call->SetDirection(Call::CALL_OUT);
    m_callList->InsertCallItem(call);
    m_last_channel_state = AST_STATE_RINGING;
    m_current_channel = m["Channel"];
    UpdateDialButtonImage();
}

void MyFrame::OnDialIn(const AmiMessage &m)
{
    Log("##### Somebody's going to dial us #####\n");
    std::string calleridnum = m["CallerIDNum"];
    std::string calleridname = m["CallerIDName"];
    m_current_channel = m["Destination"];
    Call *call = new Call;
    if (!m["CallerIDName"].empty() && m["CallerIDName"] != "<unknown>" && m["CallerIDName"] != m["CallerIDNum"])
    {
        call->SetName(m["CallerIDName"]);
    }
    std::list<Channel *> peers = m_channelstatuspool->getBridgedChannelsOf(m["Channel"]);
    if (peers.size() == 1)
    {
        Channel *peer = *peers.begin();
        std::string transferred_calleridnum = peer->m_bridgedTo->m_callerIDNum;
        std::string transferred_calleridname = peer->m_bridgedTo->m_callerIDName;
        if (!transferred_calleridname.empty() && transferred_calleridname != "<unknown>")
        {
            call->SetName(m["CallerIDName"] + " [" + transferred_calleridname + "]");
        }
        call->SetNumber(transferred_calleridnum);
    }
    else
    {
        call->SetNumber(m["CallerIDNum"]);
    }
    call->SetUniqueID(std::stoi(m["DestUniqueID"]));
    call->SetSecondChannelID(m["ChannelID"]);
    call->SetTimeStart(wxDateTime::Now());
    call->SetDirection(Call::CALL_IN);
    if (m_lookuper && m_lookuper->ShouldLookup(m["CallerIDNum"]))
    {
        DEBUG_MSG("lookup field: " << m_lookup_field << std::endl);
        if (!m_lookup_field.empty())
        {
            wxString name = m_lookuper->GetField(m["CallerIDNum"], m_lookup_field);
            if (!name.empty())
                call->SetName(name);
        }
        call->SetDescription(m_lookuper->GetHtml(m["CallerIDNum"]));
    }
    m_callList->InsertCallItem(call);
    UpdateDialButtonImage();
}

void MyFrame::OnUp(const AmiMessage &m)
{
    Log("\n#####\nAlready talking\n#####\n");
    wxListItem *item = NULL;
    long lastItem = 0;
    if (m_callList->GetItemCount())
    {
        Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
        if (call->GetUniqueID() == std::stoi(m["Uniqueid"])) // updating existing call
        {
            call->SetTimeAnswer(wxDateTime::Now());
            call->SetDisposition(Call::CALL_ANSWERED);
            if (call->GetDirection() == Call::CALL_OUT)
            {
                if (m["ConnectedLineNum"] != m["CallerIDNum"])
                {
                    if (m["ConnectedLineName"] != "")
                    {
                        call->SetName(m["ConnectedLineName"]);
                    }
                    call->SetNumber(m["ConnectedLineNum"]);
                }
            }
        }
    }
    m_callList->UpdateItem(lastItem);
    m_last_channel_state = AST_STATE_UP;
}

void MyFrame::OnHangup(const AmiMessage &m)
{
    Log("##### Hung up! #####\n");
    if (m_callList->GetItemCount())
    {
        int lastItem = 0;
        Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
        if (call->GetUniqueID() == std::stoi(m["UniqueID"]))
        {
            call->SetTimeEnd(wxDateTime::Now());
            if (m_last_channel_state == AST_STATE_RINGING)
            {
                // Originating cancelled:
                if (m["ConnectedLineNum"] == m["CallerIDNum"])
                {
                    delete call;
                    m_callList->DeleteItem(lastItem);
                }
                else
                {
                    switch (std::stoi(m["Cause"]))
                    {
                        case AST_CAUSE_NORMAL_CLEARING:
                            call->SetDisposition(Call::CALL_UNANSWERED);
                            m_callList->SetItemImage(lastItem, INCOMING_UNANSWERED);
                            if (!IsActive())
                            {
                                m_missed_calls++;
                                m_taskbaricon->SetMissedCalls(m_missed_calls);
                            }
                            break;

                        case AST_CAUSE_ANSWERED_ELSEWHERE:
                            call->SetDisposition(Call::CALL_ANSWERED_ELSEWHERE);
                            m_callList->SetItemImage(lastItem, INCOMING_ANSWERED_ELSEWHERE);
                            break;
                    }
                }
            }
            else if (m_last_channel_state == AST_STATE_UP)
            {
                if (!call->GetDuration())
                {
                    time_t duration = ( call->GetTimeEnd() - call->GetTimeAnswer() ).GetSeconds().GetLo();
                    call->SetDuration(duration);
                }
            }
        }
        else Log("Call not found");
    }
    m_last_channel_state = AST_STATE_DOWN;
    UpdateDialButtonImage();
}

void MyFrame::OnCdr(const AmiMessage &m)
{
    Log("##### Cdr data arrived! #####\n");
    long lastItem = 0;
    if (!m["DestinationChannel"].empty())
    {
        if (m_callList->GetItemCount())
        {
            lastItem = 0;
            Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
            if (call->GetUniqueID() == std::stoi(m["UniqueID"])) // updating existing call
            {
                if (m["Disposition"] == "BUSY" || m["Disposition"] == "NOANSWER")
                {
                    call->SetDisposition(Call::CALL_UNANSWERED);
                    call->SetTimeAnswer(wxInvalidDateTime);
                }
                call->SetDuration(stoi(m["BillableSeconds"]));
                if (call->GetNumber().empty() && call->GetDirection() == Call::CALL_OUT)
                {
                    call->SetNumber(m["Destination"]);
                }
                m_callList->UpdateItem(lastItem);

                if (!SaveCalls(CALLS_FILE))
                    std::cerr << _("Saving calls failed") << std::endl;
            }
            else Log("UniqueID " + m["UniqueID"] + " not found.");
        }
    }
    else // not a real channel
    {
        if (m_callList->GetItemCount())
        {
            lastItem = 0;
            Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(lastItem));
            if (call->GetUniqueID() == std::stoi(m["UniqueID"]))
            {
                delete call;
                m_callList->DeleteItem(lastItem);
            }
        }

    }

}

void MyFrame::OnResponse(const AmiMessage &m)
{
    if (m["Response"] == "Error")
    {
        if (m["Message"] == "Extension does not exist.")
        {
            m_CallInfo->SetPage("<body text='red'>" + _("Extension does not exist.") + "</body>");
            m_CallInfo->SetBackgroundColour(wxNullColour);
        }
    }
}

void MyFrame::OnDial(const AmiMessage &m)
{
    Log("##### We are dialing out! #####\n");
    m_current_channel = m["Channel"];
    Call *call = new Call;
    if (m["ConnectedLineName"] != "")
    {
        call->SetName(m["ConnectedLineName"]);
    }
    call->SetNumber(m["ConnectedLineNum"]);
    call->SetUniqueID(std::stoi(m["Uniqueid"]));
    call->SetTimeStart(wxDateTime::Now());
    call->SetDirection(Call::CALL_OUT);
    m_callList->InsertCallItem(call);
    m_last_channel_state = AST_STATE_RING;
    UpdateDialButtonImage();
}


void MyFrame::OnInternalMessage(const AmiMessage &m)
{
    static bool was_connected;
    if (m["InternalMessage"] == "ConnectionStatus")
    {
        bool connected = (m["Status"] == "Connected");
        if (connected == was_connected) return;
        if (!connected)
        {
            SetStatusText(_("Connection Lost"));
            Log(wxDateTime::Now().FormatISOCombined() + " " + _("Connection Lost"));
        }
        else
        {
            SetStatusText(_("Connected"));
            Log(wxDateTime::Now().FormatISOCombined() + " " + _("Connected"));
        }
        was_connected = connected;
    }
    else if (m["InternalMessage"] == "LogMsg")
    {
        Log(m["LogMsg"]);
    }
}

void MyFrame::Log(const wxString &str)
{
    StatusText->AppendText(str + "\n");
}

bool MyFrame::SaveCalls(const wxString &filename)
{
    wxFile calls;
    wxString fullpath = wxGetApp().m_config->GetLocalFileName(filename, wxCONFIG_USE_SUBDIR);
    calls.Open(fullpath, wxFile::write);
    long maxcalls = wxGetApp().m_config->ReadLong("gui/max_calls_saved", 1000);
    if (calls.IsOpened())
    {
        for (long i=0; i<m_callList->GetItemCount() && i<maxcalls; i++)
        {
            Call *call = reinterpret_cast<Call *>(m_callList->GetItemData(i));
            calls.Write(call->Serialize() + "\n");
        }
        calls.Close();
        return true;
    }
    std::cerr << _("Unable to open calls file") << " " << fullpath << std::endl;
    return false;
}

bool MyFrame::LoadCalls(const wxString &filename)
{
    wxFile calls;
    wxString fullpath = wxGetApp().m_config->GetLocalFileName(filename, wxCONFIG_USE_SUBDIR);
    calls.Open(fullpath, wxFile::read);
    if (calls.IsOpened())
    {
        wxString buff;
        if (!calls.ReadAll(&buff))
        {
            std::cerr << _("Unable to read calls from file") << " " << fullpath << std::endl;
            return false;
        }
        if (buff.length() < 30)
        {
            std::cerr << _("File is too short") << " " << fullpath << std::endl;
            return false;
        }
        wxStringTokenizer calls_tokenizer(buff, "\n");
        while ( calls_tokenizer.HasMoreTokens() )
        {
            wxString serialized_call = calls_tokenizer.GetNextToken();
            Call *call = new Call(serialized_call);
            m_callList->InsertCallItem(call, m_callList->GetItemCount());
        }
        calls.Close();
        return true;
    }
    std::cerr << _("Unable to open calls file") << " " << fullpath << std::endl;
    return false;
}
