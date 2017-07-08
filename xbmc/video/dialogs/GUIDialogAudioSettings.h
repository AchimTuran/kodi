#pragma once

/*
 *      Copyright (C) 2005-2014 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <string>
#include <utility>
#include <vector>

#include "settings/dialogs/GUIDialogSettingsManualBase.h"
#include "addons/kodi-addon-dev-kit/include/kodi/addon-instance/AudioDSP.h"

class CVariant;

class CGUIDialogAudioSettings : public CGUIDialogSettingsManualBase
{
public:
  CGUIDialogAudioSettings();
  virtual ~CGUIDialogAudioSettings();

  // specialization of CGUIWindow
  virtual void FrameMove() override;
  virtual bool OnMessage(CGUIMessage &message) override;
  virtual bool OnBack(int actionID) override;

  static std::string FormatDelay(float value, float interval);
  static std::string FormatDecibel(float value);
  static std::string FormatPercentAsDecibel(float value);

protected:
  // implementations of ISettingCallback
  virtual void OnSettingChanged(std::shared_ptr<const CSetting> setting) override;
  virtual void OnSettingAction(std::shared_ptr<const CSetting> setting) override;

  // specialization of CGUIDialogSettingsBase
  virtual bool AllowResettingSettings() const override { return false; }
  virtual void Save() override;
  virtual void SetupView() override;

  // specialization of CGUIDialogSettingsManualBase
  virtual void InitializeSettings() override;

  bool SupportsAudioFeature(int feature);

  void AddAudioStreams(std::shared_ptr<CSettingGroup> group, const std::string &settingId);

  static bool IsPlayingPassthrough(const std::string &condition, const std::string &value, std::shared_ptr<const CSetting> setting, void *data);

  static void AudioStreamsOptionFiller(std::shared_ptr<const CSetting> setting, std::vector< std::pair<std::string, int> > &list, int &current, void *data);

  static std::string SettingFormatterDelay(std::shared_ptr<const CSettingControlSlider> control, const CVariant &value, const CVariant &minimum, const CVariant &step, const CVariant &maximum);
  static std::string SettingFormatterPercentAsDecibel(std::shared_ptr<const CSettingControlSlider> control, const CVariant &value, const CVariant &minimum, const CVariant &step, const CVariant &maximum);


  float m_volume;
  int m_audioStream;
  bool m_passthrough;

  typedef std::vector<int> Features;
  Features m_audioCaps; /*!<the on current playback supported audio features */

  // AudioDSP related options
  static void AudioModeOptionFiller(std::shared_ptr<const CSetting> setting, std::vector< std::pair<std::string, int> > &list, int &current, void *data);
  std::string GetSettingsLabel(CSetting *pSetting);

  void OpenMenu(const std::string &id);
  bool HasActiveMenuHooks(AUDIODSP_MENU_HOOK_CAT category);
  void GetAudioDSPMenus(std::shared_ptr<CSettingGroup> group, AUDIODSP_MENU_HOOK_CAT category);
  bool OpenAudioDSPMenu(unsigned int setupEntry);
  int FindCategoryIndex(const std::string &catId);

  typedef struct
  {
    int              addonId;
    AUDIODSP_MENU_HOOK  hook;
  } MenuHookMember;
  typedef struct
  {
    std::string      MenuName;
    int              MenuListPtr;
    std::string      CPUUsage;
  } ActiveModeData;

  unsigned int                                m_ActiveStreamId;                         /*!< The on dialog selectable stream identifier */
  AUDIODSP_ADDON_STREAMTYPE                   m_streamTypeUsed;                         /*!< The currently available stream type */
  AUDIODSP_ADDON_BASETYPE                     m_baseTypeUsed;                           /*!< The currently detected and used base type */
  int                                         m_modeTypeUsed;                           /*!< The currently selected mode type */
  std::vector<std::string>                    m_ActiveModes;                            /*!< The process modes currently active on dsp processing stream */
  std::vector<ActiveModeData>                 m_ActiveModesData;                        /*!< The process modes currently active on dsp processing stream info*/
  std::map<std::string, int>                  m_MenuPositions;                          /*!< The differnet menu selection positions */
  std::vector<int>                            m_MenuHierarchy;                          /*!< Menu selection flow hierachy */
  std::vector<MenuHookMember>                 m_Menus;                                  /*!< storage about present addon menus on currently selected submenu */
  std::vector<std::pair<std::string, int>>    m_ModeList;                               /*!< currently present modes */
  bool                                        m_GetCPUUsage;                            /*!< if true cpu usage detection is active */
  int                                         m_MenuName;                               /*!< current menu name, needed to get after the dialog was closed for addon */

                                                                                        /*! Settings control selection and information data */
  std::string                                 m_InputChannels;
  std::string                                 m_InputChannelNames;
  std::string                                 m_InputSamplerate;
  std::string                                 m_OutputChannels;
  std::string                                 m_OutputChannelNames;
  std::string                                 m_OutputSamplerate;
  std::string                                 m_CPUUsage;
};
