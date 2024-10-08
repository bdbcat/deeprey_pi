#include "NavCanvas.h"

#include "NavSidebar.h"
#include "wx/dcbuffer.h"
#include "wx/dcgraph.h"
#include "wx/graphics.h"

wxBEGIN_EVENT_TABLE(NavCanvas, wxScrolledWindow)
    EVT_PAINT(NavCanvas::OnPaint)
    EVT_MOUSE_EVENTS(NavCanvas::OnMouseEvents) EVT_SIZE(NavCanvas::OnSize)
wxEND_EVENT_TABLE()

NavCanvas::NavCanvas(
    wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : wxScrolledWindow(parent, id, pos, size, style, name), m_timer(this, wxID_ANY)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Connect(m_timer.GetId(), wxEVT_TIMER, wxTimerEventHandler(NavCanvas::OnTimer));

    if (NAV_SMOOTH_TRANSITION)
    {
        m_timer.Start(TimerSmooth::INTERVAL);
        m_now = wxDateTime::UNow();
    }
    m_sidebarTransition = 0;
    ConstructContextMenuReplace();
    wxMenuItem* add = m_contextMenuAdd.Append(wxID_ANY, wxT("Add"));
    Connect(add->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(NavCanvas::OnContextMenuAddItemSelected));
}

NavCanvas::~NavCanvas() {}

void NavCanvas::ConstructContextMenuReplace()
{
    wxMenuItem* cogText = m_contextMenuReplace.Append(wxID_ANY, wxT("COG_TEXT"), wxEmptyString, wxITEM_CHECK);
    ConnectContextMenuReplaceItemSelected(cogText);
    wxMenuItem* dptText = m_contextMenuReplace.Append(wxID_ANY, wxT("DPT_TEXT"), wxEmptyString, wxITEM_CHECK);
    ConnectContextMenuReplaceItemSelected(dptText);
    wxMenuItem* airText = m_contextMenuReplace.Append(wxID_ANY, wxT("AIR_TEXT"), wxEmptyString, wxITEM_CHECK);
    ConnectContextMenuReplaceItemSelected(airText);
    wxMenuItem* cogImage = m_contextMenuReplace.Append(wxID_ANY, wxT("COG_IMAGE"), wxEmptyString, wxITEM_CHECK);
    ConnectContextMenuReplaceItemSelected(cogImage);
    wxMenuItem* dptImage = m_contextMenuReplace.Append(wxID_ANY, wxT("DPT_IMAGE"), wxEmptyString, wxITEM_CHECK);
    ConnectContextMenuReplaceItemSelected(dptImage);
    m_contextMenuReplace.AppendSeparator();
    wxMenuItem* remove = m_contextMenuReplace.Append(wxID_ANY, wxT("Remove"));
    ConnectContextMenuReplaceItemSelected(remove);
}

void NavCanvas::OnPaint(wxPaintEvent& event)
{
    // Just use this class instead of wxPaintDC and make sure
    // wxWindow::SetBackgroundStyle() is called with wxBG_STYLE_PAINT somewhere in the class
    // initialization code, and that's all you have to do to (mostly) avoid flicker.
    wxBufferedPaintDC pdc(this);

#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC gdc;
    wxGraphicsContext* context = wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(pdc);
    context->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    gdc.SetBackground(GetBackgroundColour());
    gdc.SetGraphicsContext(context);
    wxDC& dc = static_cast<wxDC&>(gdc);
#else
    wxDC& dc = pdc;
#endif

    PrepareDC(dc);
    dc.Clear();

    for (auto it = m_floattingItems.begin(); it != m_floattingItems.end(); ++it)
    {
        if (it->get() != m_draggedItem.get())
        {
            (*it)->Draw(dc);
        }
    }

    if (m_sidebar != nullptr)
    {
        m_sidebar->Draw(dc);
    }

    if (m_draggedItem != nullptr)
    {
        m_draggedItem->Draw(dc);
    }
}

void NavCanvas::RefreshAndUpdate()
{
    wxRect rect = wxRect(GetPosition(), GetSize());
    Refresh(true); //, &rect);
    Update();
}

void NavCanvas::RefreshSidebarRect()
{
    wxRect rect =
        wxRect(wxPoint(GetSize().x - m_sidebar->GetSize().x, m_sidebar->GetPosition().y), m_sidebar->GetSize());
    Refresh(true); //, &rect);
    Update();
}

void NavCanvas::OnMouseEvents(wxMouseEvent& event)
{
    if (event.LeftDown())
    {
        OnMouseLeftDown(event);
    }
    else if (event.LeftUp())
    {
        OnMouseLeftUp(event);
    }
    else if (event.Dragging())
    {
        OnMouseDragging(event);
    }
    else if (event.RightDown())
    {
        OnMouseRightDown(event);
    }
}

void NavCanvas::OnSize(wxSizeEvent& event)
{
    m_sidebar->SetSize(wxSize(m_sidebar->GetSize().x, GetSize().y));
    m_sidebar->SetPosition(wxPoint(GetSize().x - m_sidebar->GetSize().x, m_sidebar->GetPosition().y));
    m_sidebar->Layout();
    RefreshAndUpdate();
}

void NavCanvas::ShowSelectItemDialog()
{
    int DIALOG_WIDTH = 600;
    int DIALOG_HEIGHT = 400;
    NavSelectItemDialog select_item_dialog(
        this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(DIALOG_WIDTH, DIALOG_HEIGHT), wxCAPTION | wxCLOSE_BOX);

    if (!select_item_dialog.IsShown())
    {
        wxPoint pos = GetParent()->GetPosition();
        wxSize size = GetParent()->GetSize();
        select_item_dialog.SetPosition(wxPoint(
            pos.x + (size.x - select_item_dialog.GetSize().x) * 0.5,
            pos.y + (size.y - select_item_dialog.GetSize().y) * 0.5));
        select_item_dialog.ShowItemsPage();
        select_item_dialog.Show();
        select_item_dialog.FindWindow(wxID_OK)
            ->Hide(); // must hide after Show(), otherwise incorrectly positioned in sizer
        int ret = select_item_dialog.ShowModal(); // ugly code! Show() and then ShowModal()
        if (ret == wxID_OK)
        {
            m_sidebar->InsertItem(CreateSidebarItem(select_item_dialog.GetSelectedItemType()), 0);
            m_sidebar->Layout();
            RefreshAndUpdate();
        }
    }
}

void NavCanvas::SortFloatingItems()
{
    std::sort(
        m_floattingItems.begin(),
        m_floattingItems.end(),
        [](const std::shared_ptr<NavSidebarItem> a, const std::shared_ptr<NavSidebarItem> b) {
            return a->GetPosition().y + a->GetSize().y < b->GetPosition().y + b->GetSize().y;
        });
}

void NavCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
#if 0
    if (m_sidebar->IsShown())
    {
        if (m_sidebar->GetClose().HitTest(event.GetPosition()))
        {
            if (NAV_SMOOTH_TRANSITION)
            {
                m_sidebarTransition = 1;
            }
            else
            {
                ShowSidebarInternal(false);
            }
            return;
        }
        if (m_sidebar->GetPlus().HitTest(event.GetPosition()))
        {
            ShowSelectItemDialog();
            return;
        }
    }
#endif

    if (m_sidebar->IsShown())
      m_draggedItem = m_sidebar->FindItem(event.GetPosition().x, event.GetPosition().y);

    if (m_draggedItem == nullptr)
    {
        for (auto it = m_floattingItems.rbegin(); it != m_floattingItems.rend(); ++it)
        {
            if ((*it)->IsInside(event.GetPosition().x, event.GetPosition().y))
            {
                m_draggedItem = *it;
                break;
            }
        }
    }

    if (m_draggedItem != nullptr)
    {
        m_draggedItem->SetDragging(true);
    }

    CaptureMouse(); // then receive all mouse events
    m_mouseDragLastPosition = event.GetPosition();
}

void NavCanvas::OnMouseLeftUp(wxMouseEvent& event)
{
    if (m_draggedItem != nullptr)
    {
        m_draggedItem->SetDragging(false);

        if (!m_sidebar->IsItemInside(m_draggedItem.get()))
        {
            m_sidebar->RemoveItem(m_draggedItem);
            auto it = m_floattingItems.begin();
            for (; it != m_floattingItems.end(); ++it)
            {
                if ((*it) == m_draggedItem)
                {
                    break;
                }
            }
            if (it == m_floattingItems.end())
            {
                m_floattingItems.push_back(m_draggedItem);
            }
        }
        m_draggedItem = nullptr;

        // m_sidebar->Layout();
        m_sidebar->LayoutSmoothly();
        SortFloatingItems();
        RefreshAndUpdate();
    }

    if (HasCapture())
    {
        ReleaseMouse();
    }
}

void NavCanvas::OnMouseDragging(wxMouseEvent& event)
{
    if (m_draggedItem != nullptr)
    {
        int offset = 20;
        wxPoint position = m_draggedItem->GetPosition() + event.GetPosition() - m_mouseDragLastPosition;
        if (position.y < GetPosition().y + offset - m_draggedItem->GetSize().y)
        {
            position.y = GetPosition().y + offset - m_draggedItem->GetSize().y;
        }
        if (position.y > GetPosition().y + GetSize().y - offset)
        {
            position.y = GetPosition().y + GetSize().y - offset;
        }
        if (position.x > GetPosition().x + GetSize().x - offset)
        {
            position.x = GetPosition().x + GetSize().x - offset;
        }
        m_draggedItem->SetPosition(position);

        if (m_sidebar->IsShown())
        {
            if (m_sidebar->IsItemInside(m_draggedItem.get()))
            {
                m_sidebar->OnDragItem(m_draggedItem);
            }
            else
            {
                int j = m_sidebar->FindIndex(m_draggedItem);
                if (j >= 0)
                {
                    m_sidebar->RemoveItem(m_draggedItem);
                    m_sidebar->LayoutSmoothly();
                }
            }

            int i = m_sidebar->FindIndex(m_draggedItem);
            if (i >= 0)
            {
                for (auto it = m_floattingItems.begin(); it != m_floattingItems.end(); ++it)
                {
                    if ((*it) == m_draggedItem)
                    {
                        m_floattingItems.erase(it);
                        break;
                    }
                }
            }
        }

        RefreshAndUpdate();
    }
    m_mouseDragLastPosition = event.GetPosition();
}

void NavCanvas::ShowContextMenuAdd(wxMouseEvent& event)
{
    PopupMenu(&m_contextMenuAdd, event.GetPosition());
}

void NavCanvas::ShowContextMenuReplace(wxMouseEvent& event, SidebarItemType type)
{
    wxMenuItemList& items = m_contextMenuReplace.GetMenuItems();
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        auto pmenu_item = *it;
        if (pmenu_item->IsCheckable())
        {
            pmenu_item->Check(type == ConvertTextToItemType(pmenu_item->GetItemLabel()));
        }
    }
    PopupMenu(&m_contextMenuReplace, event.GetPosition());
}

SidebarItemType NavCanvas::ConvertTextToItemType(const wxString& text)
{
    if (text.CompareTo(wxT("COG_TEXT")) == 0)
    {
        return SIDEBAR_ITEM_COG_TEXT;
    }
    if (text.CompareTo(wxT("DPT_TEXT")) == 0)
    {
        return SIDEBAR_ITEM_DPT_TEXT;
    }
    if (text.CompareTo(wxT("AIR_TEXT")) == 0)
    {
        return SIDEBAR_ITEM_AIR_TEXT;
    }
    if (text.CompareTo(wxT("COG_IMAGE")) == 0)
    {
        return SIDEBAR_ITEM_COG_IMAGE;
    }
    if (text.CompareTo(wxT("DPT_IMAGE")) == 0)
    {
        return SIDEBAR_ITEM_DPT_IMAGE;
    }
    return SIDEBAR_ITEM_NONE;
}

void NavCanvas::OnMouseRightDown(wxMouseEvent& event)
{
    m_rightClickedItem.reset();

    if (m_draggedItem != nullptr)
    {
        return;
    }

    if (m_sidebar->IsShown() && m_sidebar->GetBodyRect().Contains(event.GetPosition()))
    {
        m_rightClickedItem = m_sidebar->FindItem(event.GetPosition().x, event.GetPosition().y);
        if (m_rightClickedItem.expired())
        {
            ShowContextMenuAdd(event);
        }
        else
        {
            ShowContextMenuReplace(event, m_rightClickedItem.lock()->GetItemType());
        }
    }
    else
    {
        for (auto it = m_floattingItems.rbegin(); it != m_floattingItems.rend(); ++it)
        {
            if ((*it)->IsInside(event.GetPosition().x, event.GetPosition().y))
            {
                m_rightClickedItem = *it;
                break;
            }
        }
        if (!m_rightClickedItem.expired())
        {
            ShowContextMenuReplace(event, m_rightClickedItem.lock()->GetItemType());
        }
    }
}

void NavCanvas::OnTimer(wxTimerEvent& event)
{
    wxDateTime now = wxDateTime::UNow();
    wxTimeSpan span = now - m_now;
    m_now = now;

    if (m_sidebarTransition != 0)
    {
        TryMoveSidebar(event, span.GetMilliseconds().ToLong());
    }
    else
    {
        if (m_sidebar->IsShown() && m_sidebar->GetMoveCount() > 0)
        {
            m_sidebar->OnTimer(event, span.GetMilliseconds().ToLong());
            RefreshAndUpdate();
        }
    }

    wxLogStatus(wxString::Format("OnTimer: %ld", span.GetMilliseconds().ToLong()));
}

void NavCanvas::ConnectContextMenuReplaceItemSelected(wxMenuItem* item)
{
    Connect(
        item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(NavCanvas::OnContextMenuReplaceItemSelected));
}

void NavCanvas::OnContextMenuAddItemSelected(wxCommandEvent& event)
{
    wxMenuItem* item = m_contextMenuAdd.FindItem(event.GetId());
    if (m_rightClickedItem.expired())
    {
        ShowSelectItemDialog();
    }
}

void NavCanvas::OnContextMenuReplaceItemSelected(wxCommandEvent& event)
{
    wxMenuItem* item = m_contextMenuReplace.FindItem(event.GetId());
    wxLogStatus(wxString::Format(wxT("Selected menu item: %s"), item->GetItemLabel()));

    if (!m_rightClickedItem.expired())
    {
        auto it = m_floattingItems.begin();
        for (; it != m_floattingItems.end(); ++it)
        {
            if ((*it) == m_rightClickedItem.lock())
            {
                break;
            }
        }

        if (it != m_floattingItems.end())
        {
            if (item->GetItemLabel().CompareTo(wxT("Remove")) == 0)
            {
                m_floattingItems.erase(it);
            }
            else
            {
                *it = CreateSidebarItemByText(item->GetItemLabel());
                (*it)->SetPosition(m_rightClickedItem.lock()->GetPosition());
            }
            SortFloatingItems();
        }
        else
        {
            if (item->GetItemLabel().CompareTo(wxT("Remove")) == 0)
            {
                m_sidebar->RemoveItem(m_rightClickedItem.lock());
            }
            else
            {
                int i = m_sidebar->FindIndex(m_rightClickedItem.lock());
                m_sidebar->RemoveItem(m_rightClickedItem.lock());
                m_sidebar->InsertItem(CreateSidebarItemByText(item->GetItemLabel()), i);
            }
            m_sidebar->Layout();
        }
        RefreshAndUpdate();
    }
}

std::unique_ptr<NavSidebarItem> NavCanvas::CreateSidebarItemByText(const wxString& text)
{
    return CreateSidebarItem(ConvertTextToItemType(text));
}

std::unique_ptr<NavSidebarItem> NavCanvas::CreateSidebarItem(SidebarItemType type)
{
    std::unique_ptr<NavSidebarItem> item;
    switch (type) {
        case SIDEBAR_ITEM_AIR_TEXT:
            item.reset(new NavSidebarItemText(wxT("Air"), wxT(".C"),
                                              wxDefaultPosition,
                                              wxSize(200, 100), type));
            break;

        case SIDEBAR_ITEM_COG_TEXT:
            item.reset(new NavSidebarItemText(wxT("COG"), wxT(".M"),
                                              wxDefaultPosition,
                                              wxSize(200, 100), type));
            break;

        case SIDEBAR_ITEM_DPT_TEXT:
            item.reset(new NavSidebarItemText(wxT("DPT"), wxT("ft"),
                                              wxDefaultPosition,
                                              wxSize(200, 100), type));
            break;

        case SIDEBAR_ITEM_COG_IMAGE: {
            wxFileName fn;
            wxString tmp_path;
            tmp_path = GetPluginDataDir("deeprey_pi");
            fn.SetPath(tmp_path);
            fn.AppendDir(_T("data"));
            fn.SetFullName(_T("cog.svg"));

            item.reset(new NavSidebarItemImage(
                LoadSVG(fn.GetFullPath(), NAV_SIDEBAR_ICON_WIDTH,
                        NAV_SIDEBAR_ICON_HEIGHT),
                wxDefaultPosition,
                wxSize(NAV_SIDEBAR_WIDGET_WIDTH, NAV_SIDEBAR_WIDGET_HEIGHT),
                type));
            break;
        }
        case SIDEBAR_ITEM_DPT_IMAGE: {
                wxFileName fn;
                wxString tmp_path;
                tmp_path = GetPluginDataDir("deeprey_pi");
                fn.SetPath(tmp_path);
                fn.AppendDir(_T("data"));
                fn.SetFullName(_T("dpt.svg"));

                item.reset(new NavSidebarItemImage(
                    LoadSVG(fn.GetFullPath(), NAV_SIDEBAR_ICON_WIDTH,
                        NAV_SIDEBAR_ICON_HEIGHT),
                wxDefaultPosition,
                wxSize(NAV_SIDEBAR_WIDGET_WIDTH, NAV_SIDEBAR_WIDGET_HEIGHT),
                type));
            break;
        }
        default: break;
    }

    return item;
}

void NavCanvas::ShowSidebar()
{
    if (NAV_SMOOTH_TRANSITION)
    {
        m_sidebar->Show(true);
        GetSizer()->GetChildren()[1]->GetSizer()->Show(false);
        m_sidebarTransition = -1;
        m_sidebar->SetSize(wxSize(m_sidebar->GetSize().x, GetSize().y));
        m_sidebar->SetPosition(wxPoint(GetSize().x + TimerSmooth::XOFFSET, m_sidebar->GetPosition().y));
    }
    else
    {
        ShowSidebarInternal(true);
    }
}

void NavCanvas::ShowSidebarInternal(bool show)
{
    wxSizer* rightSizer = GetSizer()->GetChildren()[1]->GetSizer();
    rightSizer->Show(!show);
    m_sidebar->Show(show);
    GetSizer()->Layout();
    RefreshAndUpdate();
}

void NavCanvas::TryMoveSidebar(wxTimerEvent& event, long span)
{
    if (m_sidebarTransition > 0)
    {
        int targetx = GetSize().x + TimerSmooth::XOFFSET;
        if (m_sidebar->GetPosition().x < targetx)
        {
            m_sidebar->SetPosition(
                wxPoint(m_sidebar->GetPosition().x + TimerSmooth::XMOVEMENT, m_sidebar->GetPosition().y));
            if (m_sidebar->GetPosition().x >= targetx)
            {
                m_sidebarTransition = 0;
                m_sidebar->SetPosition(wxPoint(targetx, m_sidebar->GetPosition().y));
                m_sidebar->Layout();
                ShowSidebarInternal(false);
            }
            else
            {
                m_sidebar->Layout();
                RefreshSidebarRect();
            }
        }
    }
    else if (m_sidebarTransition < 0)
    {
        int targetx = GetSize().x - m_sidebar->GetSize().x;
        if (m_sidebar->GetPosition().x > targetx)
        {
            m_sidebar->SetPosition(
                wxPoint(m_sidebar->GetPosition().x - TimerSmooth::XMOVEMENT, m_sidebar->GetPosition().y));
            if (m_sidebar->GetPosition().x <= targetx)
            {
                m_sidebarTransition = 0;
                m_sidebar->SetPosition(wxPoint(targetx, m_sidebar->GetPosition().y));
                m_sidebar->Layout();
                ShowSidebarInternal(true);
            }
            else
            {
                m_sidebar->Layout();
                RefreshSidebarRect();
            }
        }
    }
}