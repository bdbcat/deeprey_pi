#include "NavDialogs.h"

#include "NavCommon.h"
#include "wx/wrapsizer.h"

// ==================================================== //
// ++++++++++++    NavBitmapLabelButton    ++++++++++++ //
// ==================================================== //

wxBEGIN_EVENT_TABLE(NavBitmapLabelButton, wxControl) EVT_PAINT(NavBitmapLabelButton::OnPaint)
        EVT_LEFT_DOWN(NavBitmapLabelButton::OnMouseLeftDown) wxEND_EVENT_TABLE()

                void NavBitmapLabelButton::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    PrepareDC(dc);
    dc.Clear();

    dc.SetBrush(wxBrush(*wxWHITE));
    dc.SetPen(wxPen(*wxBLACK));
    dc.DrawRoundedRectangle(wxRect(wxPoint(0, 0), GetSize()), 5);

    wxSize bitmapSize = m_bitmap.GetSize();
    int bmpx = (GetSize().x - bitmapSize.x) * 0.5;
    int bmpy = (GetSize().y - bitmapSize.y) * 0.5;
    dc.DrawBitmap(m_bitmap, wxPoint(bmpx, bmpy));

    dc.SetTextForeground(*wxBLACK);
    dc.SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    wxSize labelSize = dc.GetTextExtent(m_label);
    int LABEL_Y_OFFSET = 10;
    int labx = (GetSize().x - labelSize.x) * 0.5;
    int laby = GetSize().y - labelSize.y - LABEL_Y_OFFSET;
    dc.DrawText(m_label, wxPoint(labx, laby));
}

void NavBitmapLabelButton::OnMouseLeftDown(wxMouseEvent& event) {
    // wxLogStatus(wxString::Format("OnMouseLeftDown Dectected: %d!", event.GetId()));
    ((NavSelectItemDialog*)(GetParent()->GetParent()))->OnItemSelected(this);
}

// =================================================== //
// +++++++++++     NavSelectItemDialog     +++++++++++ //
// =================================================== //

NavSelectItemDialog::NavSelectItemDialog(wxWindow* parent, wxWindowID id, const wxString& caption,
                                         const wxPoint& position, const wxSize& size, long style)
      : wxDialog(parent, id, caption, position, size, style) {
    m_selectedItemType = SIDEBAR_ITEM_NONE;

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* title = new wxStaticText(this, wxID_ANY, wxT("Add NavData ..."));
    topSizer->Add(title, wxSizerFlags().Center());

    int BORDER = 5;
    wxWrapSizer* itemSizer = new wxWrapSizer();
    m_pageItems = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(GetSize().x, 400));
    m_pageItems->SetScrollRate(10, 10);
    itemSizer->Add(CreateConfigItem(m_pageItems, SIDEBAR_ITEM_COG_TEXT, wxT("COG"), wxT("cog.svg")),
                   0, wxALL, BORDER);
    itemSizer->Add(CreateConfigItem(m_pageItems, SIDEBAR_ITEM_COG_IMAGE, wxT("COG"),
                                    wxT("cog.svg")),
                   0, wxALL, BORDER);
    itemSizer->Add(CreateConfigItem(m_pageItems, SIDEBAR_ITEM_DPT_TEXT, wxT("DPT"), wxT("dpt.svg")),
                   0, wxALL, BORDER);
    itemSizer->Add(CreateConfigItem(m_pageItems, SIDEBAR_ITEM_DPT_IMAGE, wxT("DPT"),
                                    wxT("dpt.svg")),
                   0, wxALL, BORDER);
    m_pageItems->SetSizer(itemSizer);

    m_pageSizer = new NavPageSizer(*this);
    m_pageSizer->Add(m_pageItems);
    m_currentPage = m_pageItems;
    topSizer->Add(m_pageSizer, wxSizerFlags(1).Expand());

    m_pageDetail = new wxPanel(this);
    wxBoxSizer* detailSizer = new wxBoxSizer(wxVERTICAL);
    detailSizer->Add(new wxRadioButton(m_pageDetail, wxID_ANY, "Heave"));
    m_pageDetail->SetSizer(detailSizer);
    m_pageDetail->Hide();

    m_bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* ok = new wxButton(this, wxID_OK);
    wxButton* cancel = new wxButton(this, wxID_CANCEL);
    m_bottomSizer->Add(ok);
    m_bottomSizer->AddSpacer(10);
    m_bottomSizer->Add(cancel);
    topSizer->Add(m_bottomSizer, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(topSizer);

    // Can not Hide Here! (not calculated in sizer)
    // FindWindow(wxID_OK)->Hide();
}

NavBitmapLabelButton* NavSelectItemDialog::CreateConfigItem(wxWindow* parent, SidebarItemType type,
                                                            const wxString& title,
                                                            const wxString& svg) {

    auto bitmap = LoadSVG(svg, NAV_SIDEBAR_ICON_WIDTH, NAV_SIDEBAR_ICON_HEIGHT);
    int WIDTH = 150;
    int HEIGHT = 150;
    NavBitmapLabelButton* item =
            new NavBitmapLabelButton(parent, wxID_ANY, type, bitmap, title, wxDefaultPosition,
                                     wxSize(WIDTH, HEIGHT), wxNO_BORDER);
    return item;
}

void NavSelectItemDialog::OnItemSelected(NavBitmapLabelButton* item) {
    m_pageItems->Hide();
    m_currentPage = m_pageDetail;
    m_pageSizer->Layout();
    m_pageDetail->Show(true);
    m_pageDetail->SetFocus();

    FindWindow(wxID_OK)->Show(true);

    m_selectedItemType = item->GetItemType();
}

void NavSelectItemDialog::ShowItemsPage() {
    m_pageDetail->Hide();
    m_currentPage = m_pageItems;
    m_pageSizer->Layout();
    m_pageItems->Show(true);
    m_pageItems->SetFocus();

    m_selectedItemType = SIDEBAR_ITEM_NONE;
}