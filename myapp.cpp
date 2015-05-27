// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#ifdef WX_PRECOMP
	#include <wx/wxprec.h>
#else
	#include <wx/wx.h>
#endif

#include <wx/string.h>

#include "controller.h"
#include "asterisk.h"
#include "notificationFrame.h"
#include "myapp.h"
#include "mainframe.h"

wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
    SetExitOnFrameDelete(true);
    frame->Show( true );
    SetTopWindow(frame);
    Asterisk *asterisk = new Asterisk("212.20.30.8", 5038, "themaster", "MyS3cretPasswordManager");
    m_controller = new AsteriskController(asterisk);
    MyChanFilter *mychanfilter = new MyChanFilter("SIP/it1");
    asterisk->add(*mychanfilter);
    mychanfilter->add(*frame);
    notificationFrame *notifyframe = new notificationFrame(frame);
    notifyframe->SetController(m_controller);
    mychanfilter->add(*notifyframe);
    std::cout << "ExitOnFrameDelete: " << GetExitOnFrameDelete() << std::endl;
    return true;
}

