#include "calllistctrl.h"

long CallListCtrl::InsertCallItem(Call *call, long index)
{
    wxListItem *item = new wxListItem;
    item->SetId(index);
    item->SetData(call);
    InsertItem(*item);
    SetItem(item->GetId(), 1, call->GetTimeStart().FormatISOCombined(' '));
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
