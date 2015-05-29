#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "asterisk.h"

class AsteriskController
{
private:
	Asterisk *m_asterisk;
public:
	wxWindow *m_mainFrame;
	void CloseMainFrame(wxCommandEvent& event){
		if (m_mainFrame)
		{
			m_mainFrame->Close();
			m_mainFrame = NULL;
		}
	};

	AsteriskController(Asterisk *asterisk)
	{
		m_mainFrame = NULL;
		m_asterisk = asterisk;
	}
	void HangupChannel(std::string &channel)
	{
		m_asterisk->HangupChannel(channel);
	}
};

class ControllerUser
{
protected:
	AsteriskController *m_controller;
public:
	ControllerUser(){m_controller = NULL;};
	void SetController(AsteriskController *c){
		std::cout << "set controller" << std::endl;
	       	m_controller = c; 
	};
};

#endif
