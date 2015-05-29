#include <wx/wx.h>
#include <wx/taskbar.h>
#include "controller.h"
#include "taskbaricon.h"

MyTaskBarIcon::MyTaskBarIcon()
{
	//m_parent = parent;
	//Bind(wxEVT_TASKBAR_RIGHT_DOWN, &MyTaskBarIcon::OnRightDown, this);
	Bind(wxEVT_TASKBAR_LEFT_DCLICK, &MyTaskBarIcon::OnLeftButtonDClick, this);
	wxIcon icon("/usr/share/pixmaps/astercti.png", wxBITMAP_TYPE_PNG);
	SetIcon(icon);
}

void MyTaskBarIcon::OnExit(wxCommandEvent& event)
{
	m_parent->Close();
}

void MyTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
	std::cout << "taskbar left dclick" << std::endl;
	if (!m_controller) return;
	if (m_controller->m_mainFrame && m_controller->m_mainFrame->IsShown())
		m_controller->m_mainFrame->Show(false);
	else
	{
		m_controller->m_mainFrame->Show(true);
		m_controller->m_mainFrame->Raise();
	}
}

wxMenu* MyTaskBarIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu("AsterCTI");
	menu->Append(wxID_EXIT);
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MyTaskBarIcon::OnExit, this);
	return menu;
}
