#include "NavRotatedButton.h"

#include "wx/dcbuffer.h"

wxBEGIN_EVENT_TABLE(NavRotatedButton, wxControl)
    EVT_PAINT(NavRotatedButton::OnPaint)
    EVT_LEFT_DOWN(NavRotatedButton::OnMouseLeftDown)
wxEND_EVENT_TABLE()

void NavRotatedButton::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);
    dc.Clear();

    dc.SetBrush(wxBrush(*wxBLACK));
    dc.SetPen(*wxWHITE_PEN); // border color painted by this too
    dc.SetTextForeground(*wxWHITE);
    dc.SetFont(GetFont());

    wxSize size1 = GetSize();
    wxSize size2 = dc.GetTextExtent(m_label);
    int x = (size1.x - size2.y) / 2;
    int y = (size1.y + size2.x) / 2;
    dc.DrawRectangle(GetSize());
    dc.DrawRotatedText(m_label, x, y, 90);
}

void NavRotatedButton::OnMouseLeftDown(wxMouseEvent& event) 
{
    wxCommandEvent buttonEvent(wxEVT_BUTTON, GetId());
    wxPostEvent(GetParent(), buttonEvent);
}
