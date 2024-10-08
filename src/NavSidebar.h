#pragma once
#include "NavCommon.h"

class NavSidebarItem
{
  public:
    NavSidebarItem(const wxPoint& pos, const wxSize& size, SidebarItemType type);
    virtual ~NavSidebarItem();

  public:
    virtual void Draw(wxDC& dc) = 0;
    bool IsInside(int x, int y);

    void SetValue(const wxString& value)
    {
        m_value = value;
    }
    wxPoint GetPosition() const
    {
        return m_position;
    }
    void SetPosition(const wxPoint& position)
    {
        m_position = position;
    }
    wxSize GetSize() const
    {
        return m_size;
    }
    void SetSize(const wxSize& size)
    {
        m_size = size;
    }
    bool GetDragging() const
    {
        return m_dragging;
    }
    void SetDragging(bool dragging)
    {
        m_dragging = dragging;
    }
    SidebarItemType GetItemType()
    {
        return m_type;
    }

  protected:
    wxString m_value;
    wxPoint m_position;
    wxSize m_size;

  private:
    bool m_dragging;
    SidebarItemType m_type;
};

class NavSidebarItemText : public NavSidebarItem
{
  public:
    NavSidebarItemText(
        const wxString& title,
        const wxString& symbol,
        const wxPoint& pos,
        const wxSize& size,
        SidebarItemType type);

    ~NavSidebarItemText() {}

  public:
    virtual void Draw(wxDC& dc);

  private:
    wxString m_title;
    wxString m_symbol;
};

class NavSidebarItemImage : public NavSidebarItem
{
  public:
    NavSidebarItemImage(const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, SidebarItemType type);
    ~NavSidebarItemImage() {}

  public:
    virtual void Draw(wxDC& dc);

  private:
    wxBitmap m_bitmap;
};

struct NavSidebarItemMovement
{
    std::weak_ptr<NavSidebarItem> item;
    wxPoint targetPosition;
    int maxFrame;
    int curFrame;
};

class NavBitmapButton
{
  public:
    NavBitmapButton(
        const wxBitmap& bitmap,
        const wxSize& size = wxDefaultSize,
        const wxPoint& position = wxDefaultPosition)
        : m_size(size), m_position(position), m_bitmap(bitmap)
    {
    }

    wxSize GetSize() const
    {
        return m_size;
    }
    wxPoint GetPosition() const
    {
        return m_position;
    }
    wxBitmap GetBitmap() const
    {
        return m_bitmap;
    }
    void SetSize(const wxSize& size)
    {
        m_size = size;
    }
    void SetPosition(const wxPoint& position)
    {
        m_position = position;
    }
    bool HitTest(const wxPoint& position);

  private:
    wxSize m_size;
    wxPoint m_position;
    wxBitmap m_bitmap;
};

class NavSidebar : wxEvtHandler
{
  public:
    constexpr static int MAX_ITEMS = 10;

    NavSidebar(const wxPoint& position, const wxSize& size, int headSizeY);
    ~NavSidebar();

    void InsertItem(std::shared_ptr<NavSidebarItem> item, int index);
    void RemoveItem(const std::shared_ptr<NavSidebarItem> item);
    std::shared_ptr<NavSidebarItem> FindItem(int x, int y);
    bool IsItemInside(const NavSidebarItem* item) const;

    void Show(bool show);
    bool IsShown()
    {
        return m_shown;
    }
    void Draw(wxDC& dc);
    void Layout();
    void LayoutSmoothly();
    void OnDragItem(std::shared_ptr<NavSidebarItem> item);
    void OnTimer(wxTimerEvent& event, long span);
    int GetMoveCount()
    {
        return m_movements.size();
    }

    wxSize GetSize()
    {
        return m_size;
    }
    wxPoint GetPosition()
    {
        return m_position;
    }
    auto& GetPlus() const
    {
        return *m_plus;
    }
    auto& GetClose() const
    {
        return *m_close;
    }

    void SetSize(const wxSize& size)
    {
        m_size = size;
    }
    void SetPosition(const wxPoint& position)
    {
        m_position = position;
    }
    wxRect GetBodyRect() const;
    int FindIndex(const std::shared_ptr<NavSidebarItem> item) const;
    void SetHeadBitmaps(const wxBitmap& plus, const wxBitmap& close);

  private:
    void LayoutHead();
    int FindIndexByPosition(const std::shared_ptr<NavSidebarItem> item, int index) const;
    int GetItemsStarty() const;
    void SetItemMoveTarget(std::shared_ptr<NavSidebarItem> item, const wxPoint& position);

  private:
    bool m_shown;
    wxPoint m_position;
    wxSize m_size;
    wxSize m_headSize;

    std::unique_ptr<NavBitmapButton> m_plus;
    std::unique_ptr<NavBitmapButton> m_close;

    int m_count;
    std::array<std::shared_ptr<NavSidebarItem>, MAX_ITEMS> m_items;

    wxTimer m_timer;
    std::vector<NavSidebarItemMovement> m_movements;
};