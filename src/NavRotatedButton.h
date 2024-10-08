#pragma once

#include "NavCommon.h"

class NavRotatedButton : public wxControl // TODO: derive from wxControl instead of wxButton
{
  public:
    NavRotatedButton() : wxControl() {}

    NavRotatedButton(
        wxWindow* parent,
        wxWindowID id,
        const wxString& label = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0)
        : wxControl(parent, id, pos, size, style), m_label(label)
    {
    }

  protected:
    void OnPaint(wxPaintEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);

  private:
    wxString m_label;
    wxDECLARE_EVENT_TABLE();
};