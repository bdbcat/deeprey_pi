#pragma once
#include "NavCommon.h"

class NavBitmapLabelButton : public wxControl
{
  public:
    NavBitmapLabelButton(
        wxWindow* parent,
        wxWindowID id,
        SidebarItemType type,
        const wxBitmap& bitmap,
        const wxString& label,
        const wxPoint& position = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        int style = 0)
        : wxControl(parent, id, position, size, style), m_itemType(type), m_label(label), m_bitmap(bitmap)
    {
    }

  public:
    SidebarItemType GetItemType() const
    {
        return m_itemType;
    }
    void OnPaint(wxPaintEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);

  private:
    wxString m_label;
    wxBitmap m_bitmap;
    SidebarItemType m_itemType;

  private:
    wxDECLARE_EVENT_TABLE();
};

class NavPageSizer;

class NavSelectItemDialog : public wxDialog
{
  public:
    NavSelectItemDialog(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Add NavData"),
        const wxPoint& position = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE);

  public:
    SidebarItemType GetSelectedItemType() const
    {
        return m_selectedItemType;
    }

    void ShowItemsPage();
    void OnItemSelected(NavBitmapLabelButton* item);
    NavBitmapLabelButton*
    CreateConfigItem(wxWindow* parent, SidebarItemType type, const wxString& title, const wxString& png);

  private:
    wxScrolledWindow* m_pageItems = nullptr;
    wxPanel* m_pageDetail = nullptr;
    wxPanel* m_currentPage = nullptr;
    NavPageSizer* m_pageSizer = nullptr;
    wxBoxSizer* m_bottomSizer = nullptr;
    SidebarItemType m_selectedItemType;

    friend class NavPageSizer;
};

class NavPageSizer : public wxSizer
{
  public:
    NavPageSizer(NavSelectItemDialog& dialog) : m_dialog(dialog) {}

    virtual void RepositionChildren(const wxSize& minSize) wxOVERRIDE
    {
        m_dialog.m_currentPage->SetSize(wxRect(m_position, m_size));
    }

    virtual wxSize CalcMin() wxOVERRIDE
    {
        return m_dialog.m_pageItems->GetMinSize();
    }

  private:
    NavSelectItemDialog& m_dialog;
};