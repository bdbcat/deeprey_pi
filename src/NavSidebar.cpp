#include "NavSidebar.h"

#include "NavCommon.h"

bool NavBitmapButton::HitTest(const wxPoint& position)
{
    wxRect rect = wxRect(m_position, m_size);
    return rect.Contains(position);
}

NavSidebarItem::NavSidebarItem(const wxPoint& pos, const wxSize& size, SidebarItemType type)
{
    m_position = pos;
    m_size = size;
    m_type = type;
    m_dragging = false;
    m_value = wxT("---");
}

NavSidebarItem::~NavSidebarItem() {}

bool NavSidebarItem::IsInside(int x, int y)
{
    return x >= m_position.x && x < m_position.x + m_size.x && y >= m_position.y && y < m_position.y + m_size.y;
}

NavSidebarItemText::NavSidebarItemText(
    const wxString& title,
    const wxString& symbol,
    const wxPoint& pos,
    const wxSize& size,
    SidebarItemType type)
    : NavSidebarItem(pos, size, type)
{
    m_title = title;
    m_symbol = symbol;
}

void NavSidebarItemText::Draw(wxDC& dc)
{
    int alpha = GetDragging() ? 156 : 255;
    dc.SetBrush(wxBrush(wxColour(0, 0, 0, alpha)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(wxPoint(m_position.x, m_position.y + 2), wxSize(m_size.x, m_size.y - 2));

    dc.SetTextForeground(*wxWHITE);
    dc.SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    int BORDER_RIGHT = 5;
    int GAP_TITLE_SYMBOL = 5;
    wxSize sizeTitle = dc.GetTextExtent(m_title);
    int x = m_position.x + m_size.x - BORDER_RIGHT - sizeTitle.x;
    dc.DrawText(m_title, wxPoint(x, m_position.y + m_size.y * 0.5 - GAP_TITLE_SYMBOL - sizeTitle.y));
    wxSize sizeSymbol = dc.GetTextExtent(m_symbol);
    dc.DrawText(m_symbol, wxPoint(x, m_position.y + m_size.y * 0.5 + GAP_TITLE_SYMBOL));
    int GAP_VALUE_TITLE = 10;
    wxSize sizeValue = dc.GetTextExtent(m_value);
    dc.DrawText(m_value, wxPoint(x - GAP_VALUE_TITLE - sizeValue.x, m_position.y + m_size.y * 0.5 - sizeValue.y * 0.5));
    dc.SetFont(wxNullFont);
}

NavSidebarItemImage::NavSidebarItemImage(
    const wxBitmap& bitmap,
    const wxPoint& pos,
    const wxSize& size,
    SidebarItemType type)
    : NavSidebarItem(pos, size, type), m_bitmap(bitmap)
{
}

void NavSidebarItemImage::Draw(wxDC& dc)
{
    int alpha = GetDragging() ? 156 : 255;
    dc.SetBrush(wxBrush(wxColour(0, 0, 0, alpha)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(wxPoint(m_position.x, m_position.y + 2), wxSize(m_size.x, m_size.y - 2));

    wxSize sb = m_bitmap.GetSize();
    int bx = m_position.x + (m_size.x - sb.x) * 0.5;
    int by = m_position.y + (m_size.y - sb.y) * 0.5;
    dc.DrawBitmap(m_bitmap, wxPoint(bx, by));

    dc.SetTextForeground(*wxWHITE);
    dc.SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    wxSize size = dc.GetTextExtent(m_value);
    dc.DrawText(m_value, m_position.x + (m_size.x - size.x) * 0.5, m_position.y + m_size.y * 0.5 + sb.y * 0.5);
    dc.SetFont(wxNullFont);
}

// ==================================================== //
// ++++++++++++++++     NavSidebar     ++++++++++++++++ //
// ==================================================== //

NavSidebar::NavSidebar(const wxPoint& position, const wxSize& size, int headSizeY)
    : m_count(0), m_size(size), m_position(position), m_shown(false)
{
    m_headSize = wxSize(m_size.x, headSizeY);
}

NavSidebar::~NavSidebar() {}

int NavSidebar::GetItemsStarty() const
{
    return m_position.y + m_headSize.y;
}

void NavSidebar::Show(bool show)
{
    m_shown = show;
}

void NavSidebar::Draw(wxDC& dc)
{
    if (m_shown)
    {
        dc.SetBrush(wxBrush(wxColour(wxT("#606060"))));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(m_position, m_size);

        dc.SetPen(wxPen(wxColour(255, 255, 255)));
        dc.DrawLine(
            wxPoint(m_position.x, m_position.y + m_headSize.y),
            wxPoint(m_position.x + m_headSize.x, m_position.y + m_headSize.y));

        if (m_plus)
          dc.DrawBitmap(m_plus->GetBitmap(), m_plus->GetPosition());
        if (m_close)
          dc.DrawBitmap(m_close->GetBitmap(), m_close->GetPosition());

        for (int i = 0; i < m_count; ++i)
        {
            if (!m_items[i]->GetDragging())
            {
                m_items[i]->Draw(dc);
            }
        }
    }
}

void NavSidebar::Layout()
{
    LayoutHead();

    int sumy = 0;
    for (int i = 0; i < m_count; ++i)
    {
        if (!m_items[i]->GetDragging())
        {
            m_items[i]->SetPosition(wxPoint(m_position.x, GetItemsStarty() + sumy));
        }
        sumy += m_items[i]->GetSize().y;
    }
}

void NavSidebar::LayoutSmoothly()
{
    if (!NAV_SMOOTH_TRANSITION)
    {
        Layout();
        return;
    }

    LayoutHead();

    int sumy = 0;
    for (int i = 0; i < m_count; ++i)
    {
        if (!m_items[i]->GetDragging())
        {
            wxPoint target = wxPoint(m_position.x, GetItemsStarty() + sumy);
            SetItemMoveTarget(m_items[i], target);
        }
        sumy += m_items[i]->GetSize().y;
    }
}

void NavSidebar::LayoutHead()
{
    int y = m_position.y + 10;
    if (m_plus)
      m_plus->SetPosition(wxPoint(m_position.x + 200, y));
    if (m_close)
      m_close->SetPosition(wxPoint(m_position.x + 250, y));
}

void NavSidebar::InsertItem(std::shared_ptr<NavSidebarItem> item, int index)
{
    if (m_count < MAX_ITEMS && index <= m_count)
    {
        ++m_count;
        for (int i = m_count - 1; i > index; --i)
        {
            m_items[i] = m_items[i - 1];
        }
        m_items[index] = item;
    }
}

void NavSidebar::RemoveItem(const std::shared_ptr<NavSidebarItem> item)
{
    for (int i = m_count - 1; i >= 0; --i)
    {
        if (item == m_items[i])
        {
            for (int j = i; j < m_count - 1; ++j)
            {
                m_items[j] = m_items[j + 1];
            }
            --m_count;
            break;
        }
    }
}

std::shared_ptr<NavSidebarItem> NavSidebar::FindItem(int x, int y)
{
    for (int i = 0; i < m_count; ++i)
    {
        if (m_items[i]->IsInside(x, y))
        {
            return m_items[i];
        }
    }
    return nullptr;
}

bool NavSidebar::IsItemInside(const NavSidebarItem* item) const
{
    int x1 = m_position.x - item->GetSize().x;
    int x2 = m_position.x + m_size.x;
    int y1 = m_position.y - item->GetSize().y;
    int y2 = m_position.y + m_size.y;
    return item->GetPosition().x >= x1 && item->GetPosition().x <= x2 && item->GetPosition().y >= y1
        && item->GetPosition().y <= y2;
}

int NavSidebar::FindIndex(const std::shared_ptr<NavSidebarItem> item) const
{
    for (int i = 0; i < m_count; ++i)
    {
        if (m_items[i] == item)
        {
            return i;
        }
    }
    return -1;
}

int NavSidebar::FindIndexByPosition(const std::shared_ptr<NavSidebarItem> item, int refi) const
{
    int y = item->GetPosition().y + item->GetSize().y * 0.5;
    int y1 = GetItemsStarty();

    for (int i = 0; i < m_count; ++i)
    {
        int y2 = y1 + m_items[i]->GetSize().y * 0.5;
        int y3 = y1 + m_items[i]->GetSize().y;
        if (i == refi)
        {
            if (y1 <= y && y <= y3)
            {
                return i;
            }
        }
        else if (i < refi)
        {
            if (y1 <= y && y <= y2)
            {
                return i;
            }
        }
        else
        {
            if (y2 <= y && y <= y3)
            {
                return i;
            }
        }
        y1 = y3;
    }

    return -1;
}

void NavSidebar::OnTimer(wxTimerEvent& event, long span)
{
    float y = TimerSmooth::YMOVEMENT;
    for (auto it = m_movements.begin(); it != m_movements.end();)
    {
        NavSidebarItemMovement& move = *it;
        if (move.curFrame < move.maxFrame)
        {
            move.curFrame++;
            float my = (move.targetPosition.y < move.item.lock()->GetPosition().y ? -1 : 1) * y;
            move.item.lock()->SetPosition(
                wxPoint(move.item.lock()->GetPosition().x, move.item.lock()->GetPosition().y + my));
            ++it;
        }
        else
        {
            move.item.lock()->SetPosition(move.targetPosition);
            it = m_movements.erase(it);
        }
    }
}

void NavSidebar::SetItemMoveTarget(std::shared_ptr<NavSidebarItem> item, const wxPoint& position)
{
    auto it = m_movements.begin();
    for (; it != m_movements.end(); ++it)
    {
        if (it->item.lock() == item)
        {
            break;
        }
    }

    if (it == m_movements.end())
    {
        NavSidebarItemMovement move;
        move.item = item;
        move.targetPosition = position;
        move.curFrame = 0;
        move.maxFrame = (int)ceilf(abs(move.targetPosition.y - item->GetPosition().y) / TimerSmooth::YMOVEMENT);
        m_movements.push_back(move);
    }
    else
    {
        NavSidebarItemMovement& move = (*it);
        if (move.targetPosition.y != position.y)
        {
            move.targetPosition = position;
            move.curFrame = 0;
            move.maxFrame = (int)ceilf(abs(move.targetPosition.y - item->GetPosition().y) / TimerSmooth::YMOVEMENT);
        }
    }
}

void NavSidebar::OnDragItem(std::shared_ptr<NavSidebarItem> item)
{
    int i1 = FindIndex(item);
    int i2 = FindIndexByPosition(item, i1);
    if (i1 < 0)
    {
        if (i2 < 0)
        {
            i2 = m_count;
        }
        ++m_count;
        for (int i = m_count - 1; i > i2; --i)
        {
            m_items[i] = m_items[i - 1];
        }
        m_items[i2] = item;
    }
    else
    {
        if (i2 >= 0 && i1 != i2)
        {
            if (i1 < i2)
            {
                for (int i = i1; i < i2; ++i)
                {
                    m_items[i] = m_items[i + 1];
                }
                m_items[i2] = item;
            }
            else
            {
                for (int i = i1; i > i2; --i)
                {
                    m_items[i] = m_items[i - 1];
                }
                m_items[i2] = item;
            }
        }
    }

    LayoutSmoothly();
}

void NavSidebar::SetHeadBitmaps(const wxBitmap& plus, const wxBitmap& close)
{
    int y = m_position.y + 10;
    m_plus.reset(new NavBitmapButton(plus, plus.GetSize(), wxPoint(m_position.x + 200, y)));
    m_close.reset(new NavBitmapButton(close, close.GetSize(), wxPoint(m_position.x + 250, y)));
}

wxRect NavSidebar::GetBodyRect() const
{
    wxPoint position(m_position.x, m_position.y + m_headSize.y);
    wxSize size(m_size.x, m_size.y - m_headSize.y);
    return wxRect(position, size);
}