#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <wx/fileconf.h>
#include "asterisk.h"

class ControllerUser;

class AsteriskController
{
private:
	Asterisk *m_asterisk;
	std::list<ControllerUser *> m_clients;
	std::string m_mychannel;
	std::string m_myexten;
	std::string m_context;
	wxFileConfig *m_config;

public:
	wxWindow *m_mainFrame;
	AsteriskController(Asterisk *asterisk, std::string context, std::string mychannel, std::string exten);
	AsteriskController(Asterisk *asterisk, wxFileConfig *config);
	~AsteriskController();
	void ImGonnaDie(ControllerUser *almostdead);
	void add(ControllerUser *client);
	void CloseMainFrame(wxCommandEvent& event);
	void ShowMainFrame();
	void SetMainFrame(wxWindow *frame);
	void HangupChannel(std::string &channel);
	void Originate(std::string exten);
	std::string GetMyExten() const;
	std::string GetMyChannel() const;
	std::string Cfg(std::string);
	long CfgInt(std::string);
	bool CfgBool(std::string, bool);
};

class ControllerUser
{
protected:
	AsteriskController *m_controller;
	std::string descr;
public:
	ControllerUser();
	~ControllerUser();
	void SetController(AsteriskController *c);
	std::string toString()
	{
		return descr;
	}
};

#endif
