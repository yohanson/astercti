#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "asterisk.h"

class AsteriskController
{
private:
	Asterisk *m_asterisk;
public:
	AsteriskController(Asterisk *asterisk)
	{
		m_asterisk = asterisk;
	}
	void HangupChannel(std::string &channel)
	{
		m_asterisk->HangupChannel(channel);
	}
};

#endif
