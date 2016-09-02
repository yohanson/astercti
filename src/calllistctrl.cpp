#include <wx/settings.h>
#include <wx/dcclient.h>
#include <wx/colour.h>
#include "calllistctrl.h"

CallListCtrl::CallListCtrl(wxWindow *  parent,
    wxWindowID  id,
    const wxPoint &        pos,
    const wxSize &         size,
    long                   style,
    const wxValidator &    validator,
    const wxString &       name,
    const wxString &       timeFormat
) : wxListCtrl(parent, id, pos, size, style, validator, name),
    m_timeFormat(timeFormat)
{
    InsertColumn(0, "");
    InsertColumn(1, "", wxLIST_FORMAT_RIGHT);
    Bind(wxEVT_SIZE, &CallListCtrl::OnResize, this);
};


long CallListCtrl::InsertCallItem(Call *call, long index)
{
    wxListItem item;
    item.SetId(index);
    item.SetData(call);
    InsertItem(item);
    wxString time = call->GetTimeStart().Format(m_timeFormat);
    SetItem(index, 1, time);
    UpdateItem(index);
}

void CallListCtrl::UpdateItem(long index)
{
    Call *call = reinterpret_cast<Call *>(GetItemData(index));
    if (!call) return;

    if (!call->GetName().empty())
    {
        SetItemText(index, call->GetNumber() + " (" + call->GetName() + ")");
    }
    else SetItemText(index, call->GetNumber());
    if (call->GetDirection() == Call::CALL_IN)
    {
        switch (call->GetDisposition())
        {
            case Call::CALL_UNANSWERED:
                SetItemImage(index, INCOMING_UNANSWERED);
                break;
            case Call::CALL_ANSWERED:
                SetItemImage(index, INCOMING_ANSWERED);
                break;
            case Call::CALL_ANSWERED_ELSEWHERE:
                SetItemImage(index, INCOMING_ANSWERED_ELSEWHERE);
                break;
         }
    }
    else // CALL_OUT
    {
        switch (call->GetDisposition())
        {
            case Call::CALL_UNANSWERED:
                SetItemImage(index, OUTBOUND_UNANSWERED);
                break;
            case Call::CALL_ANSWERED:
                SetItemImage(index, OUTBOUND_ANSWERED);
                break;
         }
    }
}

CallListCtrl::~CallListCtrl()
{
    Call *call;
    for (long i=0; i<GetItemCount(); i++)
    {
        call = reinterpret_cast<Call *>(GetItemData(i));
        delete call;
        SetItemPtrData(i, (wxUIntPtr)NULL);
    }
}

void CallListCtrl::SetTimeFormat(const wxString &timeFormat)
{
    m_timeFormat = timeFormat;
}

void CallListCtrl::OnResize(wxSizeEvent &event)
{
	wxSize csize = GetClientSize();
	wxSize vsize = GetVirtualSize();
	int width = csize.x;
	if (vsize.y > csize.y)
		width = csize.x - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
    int timewidth = wxLIST_AUTOSIZE;
    if (width < 300) timewidth = 0;
    SetColumnWidth(1, timewidth);
    if (timewidth)
    {
        timewidth = GetColumnWidth(1);
    }
    SetColumnWidth(0, width-timewidth);
	event.Skip();
}
