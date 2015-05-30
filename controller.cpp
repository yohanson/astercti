#include <wx/taskbar.h>
#include "asterisk.h"
#include "controller.h"

AsteriskController::AsteriskController(Asterisk *asterisk)
{
	m_mainFrame = NULL;
	m_asterisk = asterisk;
}
AsteriskController::~AsteriskController()
{
	std::cout << "Controller destruct" << std::endl;
	delete m_asterisk;
}
void AsteriskController::ImGonnaDie(ControllerUser *almostdead)
{
	std::cout << "Controller: remove " << almostdead << " (" << almostdead->toString() << ")" << std::endl;
	m_clients.remove(almostdead);
}
void AsteriskController::add(ControllerUser *client)
{
	std::cout << "Controller: add " << client << std::endl;
	m_clients.push_back(client);
	client->SetController(this);
}
void AsteriskController::CloseMainFrame(wxCommandEvent& event) {
	if (m_mainFrame) {
		m_mainFrame->Close();
		m_mainFrame = NULL;
	}
};
void AsteriskController::SetMainFrame(wxWindow *frame) {
	m_mainFrame = frame;
}

void AsteriskController::Originate(std::string exten)
{
	m_asterisk->Originate("SIP/100", "from-sip", "123");
}

void AsteriskController::HangupChannel(std::string &channel)
{
	m_asterisk->HangupChannel(channel);
}

// ControllerUser

ControllerUser::ControllerUser(){
	m_controller = NULL;
};

ControllerUser::~ControllerUser(){
//	if (m_controller)
//		m_controller->ImGonnaDie(this);
}

void ControllerUser::SetController(AsteriskController *c){
	std::cout << "set controller" << std::endl;
	m_controller = c; 
};
