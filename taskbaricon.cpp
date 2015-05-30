#include <wx/wx.h>
#include <wx/taskbar.h>
#include "controller.h"
#include "taskbaricon.h"

MyTaskBarIcon::MyTaskBarIcon()
{
	m_mainFrame = NULL;
	descr = "taskbar icon";
	Bind(wxEVT_TASKBAR_LEFT_DCLICK, &MyTaskBarIcon::OnLeftButtonDClick, this);
	//wxIcon icon("/usr/share/pixmaps/astercti.png", wxBITMAP_TYPE_PNG);
	wxIcon icon("/home/yohanson/coding/wx/astercti/debian/astercti/usr/share/pixmaps/astercti.png", wxBITMAP_TYPE_PNG);
	SetIcon(icon);
}

void MyTaskBarIcon::SetMainFrame(wxWindow *frame)
{
	m_mainFrame = frame;
}

void MyTaskBarIcon::OnExit(wxCommandEvent& event)
{
	//if (m_controller)
	//	m_controller->Shutdown();
	//	m_controller = NULL;
	if (m_mainFrame)
		m_mainFrame->Close();
	wxTaskBarIcon::Destroy();
}

void MyTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
	std::cout << "taskbar left dclick" << std::endl;
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
