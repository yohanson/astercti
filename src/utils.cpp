#include "utils.h"

wxString wxToString(const wxPoint& p)
{
    wxString result;
    result << p.x << 'x' << p.y;
    return result;
}

bool wxFromString(wxString& s, wxPoint *&p)
{
    long x,y;
    if (!s.BeforeFirst('x').ToLong(&x)) return false;
    if (!s.AfterLast('x').ToLong(&y)) return false;
    p->x = x;
    p->y = y;
    return true;
}

wxString wxToString(const wxSize& s)
{
    wxString result;
    result << s.GetWidth() << 'x' << s.GetHeight();
    return result;
}

bool wxFromString(wxString& s, wxSize *&size)
{
    long w,h;
    if (!s.BeforeFirst('x').ToLong(&w)) return false;
    if (!s.AfterLast('x').ToLong(&h)) return false;
    size->SetWidth(w);
    size->SetHeight(h);
    return true;
}

