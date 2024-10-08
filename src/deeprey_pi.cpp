/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Deeprey Plugin
 * Author:   Dave Register
 *
 ***************************************************************************
 *   Copyright (C) 2024 by Dave Register                                   *
 *   $EMAIL$   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <typeinfo>
#include "deeprey_pi.h"
#include "icons.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new deeprey_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}




//---------------------------------------------------------------------------------------------------------
//
//    Deeprey PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

deeprey_pi::deeprey_pi(void *ppimgr)
      :opencpn_plugin_117(ppimgr), wxTimer(this)
{
      // Create the PlugIn icons
      initialize_images();

	  wxFileName fn;

    auto path = GetPluginDataDir("deeprey_pi");
    fn.SetPath(path);
    fn.AppendDir("data");
    fn.SetFullName("deeprey_panel_icon.png");

    path = fn.GetFullPath();

    wxInitAllImageHandlers();

    wxLogDebug(wxString("Using icon path: ") + path);
    if (!wxImage::CanRead(path)) {
        wxLogDebug("Initiating image handlers.");
        wxInitAllImageHandlers();
    }
    wxImage panelIcon(path);
    if (panelIcon.IsOk())
        m_panelBitmap = wxBitmap(panelIcon);
    else
        wxLogWarning("DEEPREY panel icon has NOT been loaded");

}

int deeprey_pi::Init(void)
{
      AddLocaleCatalog( _T("opencpn-deeprey_pi") );

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();
      m_pauimgr = GetFrameAuiManager();
      m_pvdrcontrol = NULL;

      //    And load the configuration items
      LoadConfig();

      m_tb_item_id_play = InsertPlugInToolSVG("Deeprey",
            _svg_deeprey_play, _svg_deeprey_toggled, _svg_deeprey_toggled,
            wxITEM_CHECK, "Deeprey", "", NULL,
            DEEPREY_TOOL_POSITION, 0, this);

       m_recording = false;



      return (
           WANTS_TOOLBAR_CALLBACK    |
           INSTALLS_TOOLBAR_TOOL     |
           WANTS_CONFIG              |
           WANTS_NMEA_SENTENCES      |
           WANTS_AIS_SENTENCES
            );
}

bool deeprey_pi::DeInit(void)
{
      SaveConfig();
      if ( IsRunning() ) // Timer started?
      {
            Stop(); // Stop timer
            m_istream.Close();
      }

      if ( m_pvdrcontrol )
      {
            m_pauimgr->DetachPane( m_pvdrcontrol );
            m_pvdrcontrol->Close();
            m_pvdrcontrol->Destroy();
            m_pvdrcontrol = NULL;
      }

      if ( m_recording )
      {
            m_ostream.Close();
            m_recording = false;
      }

      RemovePlugInTool( m_tb_item_id_record );
      RemovePlugInTool( m_tb_item_id_play );
      return true;
}

int deeprey_pi::GetAPIVersionMajor()
{
      return atoi(API_VERSION);
}

int deeprey_pi::GetAPIVersionMinor()
{
    std::string v(API_VERSION);
    size_t dotpos = v.find('.');
    return atoi(v.substr(dotpos + 1).c_str());
}

int deeprey_pi::GetPlugInVersionMajor()
{
     return PLUGIN_VERSION_MAJOR;
}

int deeprey_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}

int GetPlugInVersionPatch()
{ 
      return PLUGIN_VERSION_PATCH; 
}

int GetPlugInVersionPost() 
{ 
      return PLUGIN_VERSION_TWEAK; 
}

const char *GetPlugInVersionPre() 
{
      return PKG_PRERELEASE; 
}

const char *GetPlugInVersionBuild()
{
       return PKG_BUILD_INFO; 
}

wxBitmap *deeprey_pi::GetPlugInBitmap()
{
       return &m_panelBitmap;
}

wxString deeprey_pi::GetCommonName()
{
      return _("VDR");
}

wxString deeprey_pi::GetShortDescription()
{
      return _("Voyage Data Recorder plugin for OpenCPN");
}

wxString deeprey_pi::GetLongDescription()
{
      return _("Voyage Data Recorder plugin for OpenCPN\n\
Provides NMEA stream save and replay.");

}

void deeprey_pi::SetNMEASentence(wxString &sentence)
{
      if (m_recording)
            m_ostream.Write(sentence);
}

void deeprey_pi::SetAISSentence(wxString &sentence)
{
      if (m_recording)
            m_ostream.Write(sentence);
}

void deeprey_pi::Notify()
{
      wxString str;
      int pos = m_istream.GetCurrentLine();

      if (m_istream.Eof() || pos==-1)
            str = m_istream.GetFirstLine();
      else
            str = m_istream.GetNextLine();

      PushNMEABuffer(str+_T("\r\n"));

      if ( m_pvdrcontrol )
            m_pvdrcontrol->SetProgress(pos);
}

void deeprey_pi::SetInterval( int interval )
{
      m_interval = interval;
      if ( IsRunning() ) // Timer started?
            Start( m_interval, wxTIMER_CONTINUOUS ); // restart timer with new interval
}

int deeprey_pi::GetToolbarToolCount(void)
{
      return 2;
}

void deeprey_pi::OnToolbarToolCallback(int id)
{
      wxSize size = GetOCPNCanvasWindow()->GetClientSize();
      wxPoint position = wxPoint(size.x * 5 / 10, 0);
      size.x /= 2;
      m_navCanvas = new NavCanvas(GetOCPNCanvasWindow(), wxID_ANY, position, size);

      // button NAVIGATION
#if 0
      NavRotatedButton* navigation = new NavRotatedButton(
          m_navCanvas,
          wxID_ANY,
          "NAVIGATION",
          wxDefaultPosition,
          wxSize(ROTATED_BUTTON_WIDTH, ROTATED_BUTTON_HEIGHT),
          wxBU_BOTTOM);
      Bind(wxEVT_BUTTON, &NavFrame::OnShowSidebar, this, navigation->GetId());
      // button PLANNING
      NavRotatedButton* planning = new NavRotatedButton(
          m_navCanvas,
          wxID_ANY,
          "PLANNING",
          wxDefaultPosition,
          wxSize(ROTATED_BUTTON_WIDTH, ROTATED_BUTTON_HEIGHT),
          wxBU_BOTTOM);
      Bind(wxEVT_BUTTON, &NavFrame::OnPlanning, this, planning->GetId());
      // right sizer
#endif

      m_navCanvas->SetSidebar(std::unique_ptr<NavSidebar>(new NavSidebar(
          wxPoint(size.x - NAV_SIDEBAR_HEADER_HEIGHT - ROTATED_BUTTON_WIDTH - 5, 0),
          wxSize(NAV_SIDEBAR_HEIGHT, size.y),
          NAV_SIDEBAR_HEADER_HEIGHT)));

      m_navCanvas->GetSidebar()->InsertItem(m_navCanvas->CreateSidebarItem(SIDEBAR_ITEM_AIR_TEXT), 0);
      m_navCanvas->GetSidebar()->InsertItem(m_navCanvas->CreateSidebarItem(SIDEBAR_ITEM_COG_TEXT), 1);
      m_navCanvas->GetSidebar()->InsertItem(m_navCanvas->CreateSidebarItem(SIDEBAR_ITEM_DPT_TEXT), 2);
      m_navCanvas->GetSidebar()->InsertItem(m_navCanvas->CreateSidebarItem(SIDEBAR_ITEM_COG_IMAGE), 3);
      m_navCanvas->GetSidebar()->InsertItem(m_navCanvas->CreateSidebarItem(SIDEBAR_ITEM_DPT_IMAGE), 4);

#if 0
      m_navCanvas->GetSidebar()->SetHeadBitmaps(
          LoadSVG(wxT("plus.svg"), NAV_SIDEBAR_HEADER_ICON_WIDTH, NAV_SIDEBAR_HEADER_ICON_HEIGHT),
          LoadSVG(wxT("close.svg"), NAV_SIDEBAR_HEADER_ICON_WIDTH, NAV_SIDEBAR_HEADER_ICON_HEIGHT)
      );
#endif

      m_navCanvas->GetSidebar()->Layout();
      m_navCanvas->GetSidebar()->Show(true);

      wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
      topSizer->AddStretchSpacer(1);

#if 0
      std::unique_ptr<wxBoxSizer> rightSizer(new wxBoxSizer(wxVERTICAL));
      rightSizer->AddSpacer(size.y * 0.2);
      rightSizer->Add(navigation, wxSizerFlags().Right());
      rightSizer->AddSpacer(size.y * 0.1);
      rightSizer->Add(planning, wxSizerFlags().Right());
      topSizer->Add(rightSizer.release());

      m_navCanvas->SetSizerAndFit(topSizer);
#endif

}

void deeprey_pi::SetColorScheme(PI_ColorScheme cs)
{
      if ( m_pvdrcontrol )
      {
            m_pvdrcontrol->SetColorScheme( cs );
      }
}

bool deeprey_pi::LoadConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath( _T("/PlugIns/Deeprey") );
            return true;
      }
      else
            return false;
}

bool deeprey_pi::SaveConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath( _T("/PlugIns/Deeprey") );
            return true;
            return true;
      }
      else
            return false;
}

//----------------------------------------------------------------
//
//    VDR replay control Implementation
//
//----------------------------------------------------------------

VDRControl::VDRControl( wxWindow *pparent, wxWindowID id, deeprey_pi *vdr, int speed, int range )
      :wxWindow( pparent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, _T("Dashboard") ), m_pvdr(vdr)
{
      wxColour cl;
      GetGlobalColor(_T("DILG1"), &cl);
      SetBackgroundColour(cl);

      wxFlexGridSizer *topsizer = new wxFlexGridSizer(2);
      topsizer->AddGrowableCol(1);

      wxStaticText *itemStaticText01 = new wxStaticText( this, wxID_ANY, _("Speed:") );
      topsizer->Add( itemStaticText01, 0, wxEXPAND|wxALL, 2 );

      m_pslider = new wxSlider( this, wxID_ANY, speed, 1, 100, wxDefaultPosition, wxSize( 200, 20) );
      topsizer->Add( m_pslider, 1, wxALL|wxEXPAND, 2 );
      m_pslider->Connect( wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(VDRControl::OnSliderUpdated), NULL, this);

      wxStaticText *itemStaticText02 = new wxStaticText( this, wxID_ANY, _("Progress:") );
      topsizer->Add( itemStaticText02, 0, wxEXPAND|wxALL, 2 );

      m_pgauge = new wxGauge( this, wxID_ANY, range, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
      topsizer->Add( m_pgauge, 1, wxALL|wxEXPAND, 2 );

      SetSizer( topsizer );
      topsizer->Fit( this );
      Layout();
}

void VDRControl::SetColorScheme( PI_ColorScheme cs )
{
      wxColour cl;
      GetGlobalColor( _T("DILG1"), &cl );
      SetBackgroundColour( cl );

      Refresh(false);
}

void VDRControl::SetProgress( int progress )
{
      m_pgauge->SetValue( progress );
}

void VDRControl::OnSliderUpdated( wxCommandEvent& event )
{
      m_pvdr->SetInterval( 1000/m_pslider->GetValue() );
}
