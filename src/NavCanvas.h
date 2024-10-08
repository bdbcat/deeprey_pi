#pragma once
#include "NavCommon.h"
#include "NavDialogs.h"

class NavCanvas;
class NavSidebar;
class NavSidebarItem;

class NavCanvas : public wxScrolledWindow
{
  public:
    NavCanvas(
        wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxASCII_STR(wxPanelNameStr));
    ~NavCanvas();

  public:
    const auto& GetSidebar() const
    {
        return m_sidebar;
    }
    void SetSidebar(std::unique_ptr<NavSidebar> sidebar)
    {
        m_sidebar = std::move(sidebar);
    }
    void ShowSidebar();
    void ShowSidebarInternal(bool show = true);
    void RefreshAndUpdate();
    std::unique_ptr<NavSidebarItem> CreateSidebarItem(SidebarItemType type);
    std::unique_ptr<NavSidebarItem> CreateSidebarItemByText(const wxString& text);

  private:
    void ConstructContextMenuReplace();
    void OnPaint(wxPaintEvent& event);
    void OnMouseEvents(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseDragging(wxMouseEvent& event);
    void OnMouseRightDown(wxMouseEvent& event);
    void OnTimer(wxTimerEvent& event);

    void SortFloatingItems();
    void ShowSelectItemDialog();

    void TryMoveSidebar(wxTimerEvent& event, long span);
    void RefreshSidebarRect();

    void ShowContextMenuAdd(wxMouseEvent& event);
    void ShowContextMenuReplace(wxMouseEvent& event, SidebarItemType type);
    void ConnectContextMenuReplaceItemSelected(wxMenuItem* item);
    void OnContextMenuReplaceItemSelected(wxCommandEvent& event);
    void OnContextMenuAddItemSelected(wxCommandEvent& event);
    SidebarItemType ConvertTextToItemType(const wxString& text);

  private:
    wxMenu m_contextMenuReplace;
    wxMenu m_contextMenuAdd;
    wxPoint m_mouseDragLastPosition;
    wxTimer m_timer;
    wxDateTime m_now;

    // wrap generic class instances with smart pointers
    std::unique_ptr<NavSidebar> m_sidebar;
    std::shared_ptr<NavSidebarItem> m_draggedItem;
    std::vector<std::shared_ptr<NavSidebarItem>> m_floattingItems;
    std::weak_ptr<NavSidebarItem> m_rightClickedItem;

    int m_sidebarTransition;

    wxDECLARE_EVENT_TABLE();
};