#include <wx/wx.h>
#include <wx/taskbar.h>
#include "taskbaricon.h"

MyTaskBarIcon::MyTaskBarIcon(wxString defaultIconFile, wxString missedIconFile, wxString tooltip = wxEmptyString)
{
    Init();
    m_defaultIcon.LoadFile(defaultIconFile, wxBITMAP_TYPE_PNG);
    m_missedIcon.LoadFile(missedIconFile, wxBITMAP_TYPE_PNG);
    tooltip_base = tooltip;
    SetIcon(m_defaultIcon, tooltip_base);
}

MyTaskBarIcon::MyTaskBarIcon(wxIcon defaultIcon, wxIcon missedIcon, wxString tooltip = wxEmptyString)
{
    Init();
    m_defaultIcon = defaultIcon;
    m_missedIcon = missedIcon;
    tooltip_base = tooltip;
    SetIcon(m_defaultIcon, tooltip_base);
}

void MyTaskBarIcon::SetMissedCalls(int missed = 0)
{
    if (missed)
    {
        SetIcon(m_missedIcon, tooltip_base + "\n" + wxString::Format(wxPLURAL("Missed call", "%d missed calls", missed), missed));
    }
    else
    {
        SetIcon(m_defaultIcon, tooltip_base);
    }
}

void MyTaskBarIcon::Init()
{
    m_mainFrame = NULL;
    Bind(wxEVT_TASKBAR_LEFT_DOWN, &MyTaskBarIcon::OnClick, this);
}

void MyTaskBarIcon::SetMainFrame(wxFrame *frame)
{
    m_mainFrame = frame;
}

void MyTaskBarIcon::OnExit(wxCommandEvent& event)
{
    RemoveIcon();
    if (m_mainFrame)
        m_mainFrame->Close(true);
}

void MyTaskBarIcon::OnClick(wxTaskBarIconEvent&)
{
    if (m_mainFrame)
    {
        if (m_mainFrame->IsShown())
            m_mainFrame->Show(false);
        else
        {
            m_mainFrame->Show(true);
            m_mainFrame->Raise();
        }
    }
}

wxMenu* MyTaskBarIcon::CreatePopupMenu()
{
    wxMenu *menu = new wxMenu("AsterCTI");
    menu->Append(wxID_EXIT);
    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MyTaskBarIcon::OnExit, this);
    return menu;
}
