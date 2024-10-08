#pragma once
#include <array>
#include <memory>
#include <wx/graphics.h>
#include <wx/wx.h>
#include <wx/filename.h>
#include "ocpn_plugin.h"

// application constants
constexpr int DEFAULT_MAIN_WINDOW_WIDTH = 1280;
constexpr int DEFAULT_MAIN_WINDOW_HEIGHT = 720;

// rotated button constants TODO: move to UI resources
constexpr int ROTATED_BUTTON_WIDTH = 25;
constexpr int ROTATED_BUTTON_HEIGHT = 90;

// sidebar constants
constexpr int NAV_SMOOTH_TRANSITION = 1;
constexpr int NAV_SIDEBAR_HEADER_HEIGHT = 60;
constexpr int NAV_SIDEBAR_HEIGHT = 300;
constexpr int NAV_SIDEBAR_HEADER_ICON_HEIGHT = 40;
constexpr int NAV_SIDEBAR_HEADER_ICON_WIDTH = 40;
constexpr int NAV_SIDEBAR_ICON_WIDTH = 128;
constexpr int NAV_SIDEBAR_ICON_HEIGHT = 128;
constexpr int NAV_SIDEBAR_WIDGET_WIDTH = 300;
constexpr int NAV_SIDEBAR_WIDGET_HEIGHT = 200;

enum SidebarItemType
{
    SIDEBAR_ITEM_NONE,
    SIDEBAR_ITEM_COG_TEXT,
    SIDEBAR_ITEM_DPT_TEXT,
    SIDEBAR_ITEM_AIR_TEXT,
    SIDEBAR_ITEM_COG_IMAGE,
    SIDEBAR_ITEM_DPT_IMAGE
};

struct TimerSmooth
{
    static const int INTERVAL = 10;
    static const int YMOVEMENT = 20;
    static const int XMOVEMENT = 20;
    static const int XOFFSET = 2;
};

inline wxBitmap LoadSVG(const wxString& filename, int width, int height)
{
    auto bb = wxBitmapBundle::FromSVGFile(filename, wxSize(width, height));
    return bb.GetBitmap(wxSize(width, height));
}