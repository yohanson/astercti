#ifndef _UTILS_H_
#define _UTILS_H_

#include <wx/string.h>
#include <wx/gdicmn.h>

wxString wxToString(const wxPoint& p);
bool wxFromString(wxString&, wxPoint *&);

wxString wxToString(const wxSize& p);
bool wxFromString(wxString&, wxSize *&);

#endif
