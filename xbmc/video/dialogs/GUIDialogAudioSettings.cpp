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

#include "GUIDialogAudioSettings.h"

#include <string>
#include <vector>

#include "addons/Skin.h"
#include "Application.h"
#include "ServiceBroker.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "cores/IPlayer.h"
#include "dialogs/GUIDialogFileBrowser.h"
#include "dialogs/GUIDialogYesNo.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "FileItem.h"
#include "filesystem/File.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "GUIPassword.h"
#include "profiles/ProfilesManager.h"
#include "settings/AdvancedSettings.h"
#include "settings/lib/Setting.h"
#include "settings/lib/SettingsManager.h"
#include "settings/MediaSettings.h"
#include "settings/MediaSourceSettings.h"
#include "settings/Settings.h"
#include "URL.h"
#include "utils/LangCodeExpander.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "video/VideoDatabase.h"
#include "settings/windows/GUIControlSettings.h"
#include "settings/lib/SettingSection.h"

// AudioDSP related includes
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSP.h"

#define SETTING_AUDIO_VOLUME                   "audio.volume"
#define SETTING_AUDIO_VOLUME_AMPLIFICATION     "audio.volumeamplification"
#define SETTING_AUDIO_DELAY                    "audio.delay"
#define SETTING_AUDIO_STREAM                   "audio.stream"
#define SETTING_AUDIO_PASSTHROUGH              "audio.digitalanalog"
#define SETTING_AUDIO_DSP_MANAGER              "audio.dspmanager"

#define SETTING_AUDIO_MAKE_DEFAULT             "audio.makedefault"

#define SETTING_AUDIO_CAT_MAIN                    "audiodspmainsettings"
#define SETTING_AUDIO_CAT_MASTER                  "audiodspmastersettings"
#define SETTING_AUDIO_CAT_POST_PROCESS            "audiodsppostsettings"
#define SETTING_AUDIO_CAT_RESAMPLING              "audiodspresamplesettings"
#define SETTING_AUDIO_CAT_PRE_PROCESS             "audiodsppresettings"
#define SETTING_AUDIO_CAT_PROC_INFO               "audiodspprocinfo"

#define SETTING_AUDIO_MAIN_STREAMTYPE             "audiodsp.main.streamtype"
#define SETTING_AUDIO_MAIN_MODETYPE               "audiodsp.main.modetype"
#define SETTING_AUDIO_MAIN_BUTTON_MASTER          "audiodsp.main.menumaster"
#define SETTING_AUDIO_MAIN_BUTTON_OUTPUT          "audiodsp.main.menupostproc"
#define SETTING_AUDIO_MAIN_BUTTON_RESAMPLE        "audiodsp.main.menuresample"
#define SETTING_AUDIO_MAIN_BUTTON_PRE_PROC        "audiodsp.main.menupreproc"
#define SETTING_AUDIO_MAIN_BUTTON_INFO            "audiodsp.main.menuinfo"
#define SETTING_AUDIO_MASTER_SETTINGS_MENUS       "audiodsp.master.menu_"
#define SETTING_AUDIO_PROC_SETTINGS_MENUS         "audiodsp.proc.menu_"

#define SETTING_STREAM_INFO_INPUT_CHANNELS        "audiodsp.info.inputchannels"
#define SETTING_STREAM_INFO_INPUT_CHANNEL_NAMES   "audiodsp.info.inputchannelnames"
#define SETTING_STREAM_INFO_INPUT_SAMPLERATE      "audiodsp.info.inputsamplerate"
#define SETTING_STREAM_INFO_OUTPUT_CHANNELS       "audiodsp.info.outputchannels"
#define SETTING_STREAM_INFO_OUTPUT_CHANNEL_NAMES  "audiodsp.info.outputchannelnames"
#define SETTING_STREAM_INFO_OUTPUT_SAMPLERATE     "audiodsp.info.outputsamplerate"
#define SETTING_STREAM_INFO_CPU_USAGE             "audiodsp.info.cpuusage"
#define SETTING_STREAM_INFO_TYPE_INPUT            "audiodsp.info.typeinput"
#define SETTING_STREAM_INFO_TYPE_PREPROC          "audiodsp.info.typepreproc"
#define SETTING_STREAM_INFO_TYPE_MASTER           "audiodsp.info.typemaster"
#define SETTING_STREAM_INFO_TYPE_POSTPROC         "audiodsp.info.typepostproc"
#define SETTING_STREAM_INFO_TYPE_OUTPUT           "audiodsp.info.typeoutput"
#define SETTING_STREAM_INFO_MODE_CPU_USAGE        "audiodsp.info.modecpuusage_"


using namespace std;
using namespace ActiveAE;


CGUIDialogAudioSettings::CGUIDialogAudioSettings()
  : CGUIDialogSettingsManualBase(WINDOW_DIALOG_AUDIO_OSD_SETTINGS, "DialogSettings.xml"),
    m_passthrough(false),
    m_ActiveStreamId(0),
    m_GetCPUUsage(false)
{ 
  m_MenuPositions[SETTING_AUDIO_CAT_MAIN]         = CONTROL_SETTINGS_START_CONTROL;
  m_MenuPositions[SETTING_AUDIO_CAT_MASTER]       = CONTROL_SETTINGS_START_CONTROL;
  m_MenuPositions[SETTING_AUDIO_CAT_POST_PROCESS] = CONTROL_SETTINGS_START_CONTROL;
  m_MenuPositions[SETTING_AUDIO_CAT_RESAMPLING]   = CONTROL_SETTINGS_START_CONTROL;
  m_MenuPositions[SETTING_AUDIO_CAT_PRE_PROCESS]  = CONTROL_SETTINGS_START_CONTROL;
  m_MenuPositions[SETTING_AUDIO_CAT_PROC_INFO]    = CONTROL_SETTINGS_START_CONTROL+2;
}

CGUIDialogAudioSettings::~CGUIDialogAudioSettings()
{ }

void CGUIDialogAudioSettings::FrameMove()
{
  // update the volume setting if necessary
  float newVolume = g_application.GetVolume(false);
  if (newVolume != m_volume)
    GetSettingsManager()->SetNumber(SETTING_AUDIO_VOLUME, newVolume);

  if (g_application.m_pPlayer->HasPlayer())
  {
    const CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();

    // these settings can change on the fly
    //! @todo (needs special handling): m_settingsManager->SetInt(SETTING_AUDIO_STREAM, g_application.m_pPlayer->GetAudioStream());
    GetSettingsManager()->SetNumber(SETTING_AUDIO_DELAY, videoSettings.m_AudioDelay);
    GetSettingsManager()->SetBool(SETTING_AUDIO_PASSTHROUGH, CServiceBroker::GetSettings().GetBool(CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH));
    
    bool forceReload = false;
    unsigned int  streamId = 0; // CServiceBroker::GetADSP().GetActiveStreamId();
    if (m_ActiveStreamId != streamId)
    {
      m_ActiveStreamId = streamId;
      //m_ActiveStreamProcess = nullptr; // CServiceBroker::GetADSP().GetDSPProcess(m_ActiveStreamId);
      //if (m_ActiveStreamId == (unsigned int)-1 || !m_ActiveStreamProcess)
      //{
      //  Close(true);
      //  return;
      //}
      forceReload = true;
    }

    int               modeUniqueId = 0;
    AUDIODSP_ADDON_BASETYPE   usedBaseType = AUDIODSP_ADDON_ABASE_UNKNOWN;
    AUDIODSP_ADDON_STREAMTYPE streamTypeUsed = AUDIODSP_ADDON_ASTREAM_INVALID;
    //m_ActiveStreamProcess->GetMasterModeTypeInformation(streamTypeUsed, usedBaseType, modeUniqueId);
    if (forceReload || m_baseTypeUsed != usedBaseType || m_streamTypeUsed != streamTypeUsed)
    {
      m_baseTypeUsed = usedBaseType;
      m_streamTypeUsed = streamTypeUsed;

      /*!
      * Update settings
      */
      int selType = CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterStreamTypeSel;
      CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterStreamBase = usedBaseType;
      CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterStreamType = streamTypeUsed;

      GetSettingsManager()->SetInt(SETTING_AUDIO_MAIN_MODETYPE, modeUniqueId);
    }

    // these settings can change on the fly
    if (m_GetCPUUsage)
    {
      m_CPUUsage = StringUtils::Format("%.02f %%", /*m_ActiveStreamProcess->GetCPUUsage()*/0.0f);
      GetSettingsManager()->SetString(SETTING_STREAM_INFO_CPU_USAGE, m_CPUUsage);
      for (unsigned int i = 0; i < m_ActiveModes.size(); i++)
      {
        std::string settingId = StringUtils::Format("%s%i", SETTING_STREAM_INFO_MODE_CPU_USAGE, i);
        m_ActiveModesData[i].CPUUsage = StringUtils::Format("%.02f %%", 0.0f/*m_ActiveModes[i]->CPUUsage()*/);
        GetSettingsManager()->SetString(settingId, m_ActiveModesData[i].CPUUsage);
      }
    }
  }

  CGUIDialogSettingsManualBase::FrameMove();
}

bool CGUIDialogAudioSettings::OnMessage(CGUIMessage &message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (iControl >= CONTROL_SETTINGS_START_CONTROL && iControl < (int)(CONTROL_SETTINGS_START_CONTROL + m_settingControls.size()))
      {
        std::shared_ptr<CSetting> setting = GetSettingControl(iControl)->GetSetting();
        if (setting != NULL)
        {
          if (setting->GetId() == SETTING_AUDIO_MAIN_BUTTON_MASTER)
            OpenMenu(SETTING_AUDIO_CAT_MASTER);
          else if (setting->GetId() == SETTING_AUDIO_MAIN_BUTTON_OUTPUT)
            OpenMenu(SETTING_AUDIO_CAT_POST_PROCESS);
          else if (setting->GetId() == SETTING_AUDIO_MAIN_BUTTON_RESAMPLE)
            OpenMenu(SETTING_AUDIO_CAT_RESAMPLING);
          else if (setting->GetId() == SETTING_AUDIO_MAIN_BUTTON_PRE_PROC)
            OpenMenu(SETTING_AUDIO_CAT_PRE_PROCESS);
          else if (setting->GetId() == SETTING_AUDIO_MAIN_BUTTON_INFO)
          {
            SetupView();
            OpenMenu(SETTING_AUDIO_CAT_PROC_INFO);
            m_GetCPUUsage = true;
          }
          else
          {
            if (setting->GetId().substr(0, 19) == SETTING_AUDIO_PROC_SETTINGS_MENUS)
              OpenAudioDSPMenu(strtol(setting->GetId().substr(19).c_str(), NULL, 0));
            else if (setting->GetId().substr(0, 21) == SETTING_AUDIO_MASTER_SETTINGS_MENUS)
              OpenAudioDSPMenu(strtol(setting->GetId().substr(21).c_str(), NULL, 0));
            else if (setting->GetId().substr(0, 27) == SETTING_STREAM_INFO_MODE_CPU_USAGE)
            {
              if (!OpenAudioDSPMenu(m_ActiveModesData[strtol(setting->GetId().substr(27).c_str(), NULL, 0)].MenuListPtr))
                CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(15031), g_localizeStrings.Get(416));
            }
          }
        }
      }
    }
    break;
  }

  return CGUIDialogSettingsManualBase::OnMessage(message);
}

bool CGUIDialogAudioSettings::OnBack(int actionID)
{
  // if the setting dialog is not a window but a dialog we need to close differently
  int mainCategory = FindCategoryIndex(SETTING_AUDIO_CAT_MAIN);
  if (m_iCategory == mainCategory)
    return CGUIDialogSettingsManualBase::OnBack(actionID);

  m_MenuPositions[m_categories[m_iCategory]->GetId()] = GetFocusedControlID();
  if (!m_MenuHierarchy.empty())
  {
    m_iCategory = m_MenuHierarchy.back();
    m_MenuHierarchy.pop_back();
  }
  else
  {
    m_iCategory = mainCategory;
  }

  if (m_iCategory == mainCategory)
  {
    SetHeading(13396);
  }

  CreateSettings();
  SET_CONTROL_FOCUS(m_MenuPositions[m_categories[m_iCategory]->GetId()], 0);

  return true;
}

std::string CGUIDialogAudioSettings::FormatDelay(float value, float interval)
{
  if (fabs(value) < 0.5f * interval)
    return StringUtils::Format(g_localizeStrings.Get(22003).c_str(), 0.0);
  if (value < 0)
    return StringUtils::Format(g_localizeStrings.Get(22004).c_str(), fabs(value));

  return StringUtils::Format(g_localizeStrings.Get(22005).c_str(), value);
}

std::string CGUIDialogAudioSettings::FormatDecibel(float value)
{
  return StringUtils::Format(g_localizeStrings.Get(14054).c_str(), value);
}

std::string CGUIDialogAudioSettings::FormatPercentAsDecibel(float value)
{
  return StringUtils::Format(g_localizeStrings.Get(14054).c_str(), CAEUtil::PercentToGain(value));
}

void CGUIDialogAudioSettings::OnSettingChanged(std::shared_ptr<const CSetting> setting)
{
  if (setting == NULL)
    return;

  CGUIDialogSettingsManualBase::OnSettingChanged(setting);
  
  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  const std::string &settingId = setting->GetId();
  if (settingId == SETTING_AUDIO_VOLUME)
  {
    m_volume = static_cast<float>(std::static_pointer_cast<const CSettingNumber>(setting)->GetValue());
    g_application.SetVolume(m_volume, false); // false - value is not in percent
  }
  else if (settingId == SETTING_AUDIO_VOLUME_AMPLIFICATION)
  {
    videoSettings.m_VolumeAmplification = static_cast<float>(std::static_pointer_cast<const CSettingNumber>(setting)->GetValue());
    g_application.m_pPlayer->SetDynamicRangeCompression((long)(videoSettings.m_VolumeAmplification * 100));
  }
  else if (settingId == SETTING_AUDIO_DELAY)
  {
    videoSettings.m_AudioDelay = static_cast<float>(std::static_pointer_cast<const CSettingNumber>(setting)->GetValue());
    g_application.m_pPlayer->SetAVDelay(videoSettings.m_AudioDelay);
  }
  else if (settingId == SETTING_AUDIO_STREAM)
  {
    m_audioStream = std::static_pointer_cast<const CSettingInt>(setting)->GetValue();
    // only change the audio stream if a different one has been asked for
    if (g_application.m_pPlayer->GetAudioStream() != m_audioStream)
    {
      videoSettings.m_AudioStream = m_audioStream;
      g_application.m_pPlayer->SetAudioStream(m_audioStream);    // Set the audio stream to the one selected
    }
  }
  else if (settingId == SETTING_AUDIO_PASSTHROUGH)
  {
    m_passthrough = std::static_pointer_cast<const CSettingBool>(setting)->GetValue();
    CServiceBroker::GetSettings().SetBool(CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH, m_passthrough);
  }
  else if (settingId == SETTING_AUDIO_MAIN_STREAMTYPE)
  {
    int type = (AUDIODSP_ADDON_STREAMTYPE)std::static_pointer_cast<const CSettingInt>(setting)->GetValue();
    CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterStreamTypeSel = type;
    if (type == AUDIODSP_ADDON_ASTREAM_AUTO)
      type = AUDIODSP_ADDON_ASTREAM_INVALID; // m_ActiveStreamProcess->GetDetectedStreamType();

    CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterStreamType = type;

    /* Set the input stream type if any modes are available for this type */
    //if (type > AUDIODSP_ADDON_ASTREAM_INVALID && type < AUDIODSP_ADDON_ASTREAM_AUTO && !m_MasterModes[type].empty())
    //{
      /* Find the master mode id for the selected stream type if it was not known before */
      //if (CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterModes[type][m_baseTypeUsed] < 0)
      //  CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterModes[type][m_baseTypeUsed] = m_MasterModes[type][0]->ModeID();

      /* Switch now the master mode and stream type for audio dsp processing */
      //m_ActiveStreamProcess->SetMasterMode((AUDIODSP_ADDON_STREAMTYPE)type,
      //                                     CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterModes[type][m_baseTypeUsed],
      //                                     true);
    //}
    //else
    //{
    //  CLog::Log(LOGERROR, "ActiveAE DSP Settings - %s - Change of audio stream type failed (type = %i)", __FUNCTION__, type);
    //}
  }
  else if (settingId == SETTING_AUDIO_MAIN_MODETYPE)
  {
    m_modeTypeUsed = std::static_pointer_cast<const CSettingInt>(setting)->GetValue();
    //if (m_ActiveStreamProcess->SetMasterMode(m_streamTypeUsed, m_modeTypeUsed))
    //  CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterModes[m_streamTypeUsed][m_baseTypeUsed] = m_modeTypeUsed;
  }
}

void CGUIDialogAudioSettings::OnSettingAction(std::shared_ptr<const CSetting> setting)
{
  if (setting == NULL)
    return;

  CGUIDialogSettingsManualBase::OnSettingAction(setting);
  
  const std::string &settingId = setting->GetId();
  if (settingId == SETTING_AUDIO_DSP_MANAGER)
  {
    g_windowManager.ActivateWindow(WINDOW_DIALOG_AUDIO_DSP_MANAGER);
  }
  else if (settingId == SETTING_AUDIO_MAKE_DEFAULT)
  {
    Save();
  }
}

void CGUIDialogAudioSettings::Save()
{
  if (!g_passwordManager.CheckSettingLevelLock(SettingLevel::Expert) &&
      CProfilesManager::GetInstance().GetMasterProfile().getLockMode() != LOCK_MODE_EVERYONE)
    return;

  // prompt user if they are sure
  if (!CGUIDialogYesNo::ShowAndGetInput(CVariant{12376}, CVariant{12377}))
    return;

  // reset the settings
  CVideoDatabase db;
  if (!db.Open())
    return;

  db.EraseVideoSettings();
  db.Close();

  // reset the settings
  CActiveAEDSPDatabase adspDB;
  if (!adspDB.Open())
    return;

  adspDB.EraseActiveDSPSettings();
  adspDB.Close();

  CMediaSettings::GetInstance().GetDefaultVideoSettings() = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  CMediaSettings::GetInstance().GetDefaultVideoSettings().m_AudioStream = -1;

  CMediaSettings::GetInstance().GetDefaultAudioSettings() = CMediaSettings::GetInstance().GetCurrentAudioSettings();
  CMediaSettings::GetInstance().GetDefaultAudioSettings().m_MasterStreamType = AUDIODSP_ADDON_ASTREAM_AUTO;

  CServiceBroker::GetSettings().Save();
}

void CGUIDialogAudioSettings::SetupView()
{
  CGUIDialogSettingsManualBase::SetupView();

  SetHeading(13396);
  SET_CONTROL_HIDDEN(CONTROL_SETTINGS_OKAY_BUTTON);
  SET_CONTROL_HIDDEN(CONTROL_SETTINGS_CUSTOM_BUTTON);
  SET_CONTROL_LABEL(CONTROL_SETTINGS_CANCEL_BUTTON, 15067);
}

void CGUIDialogAudioSettings::InitializeSettings()
{
  CGUIDialogSettingsManualBase::InitializeSettings();

  std::shared_ptr<CSettingCategory> category = AddCategory("audiosettings", -1);
  if (category == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogAudioSettings: unable to setup settings");
    return;
  }

  // get all necessary setting groups
  std::shared_ptr<CSettingGroup> groupAudio = AddGroup(category);
  if (groupAudio == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogAudioSettings: unable to setup settings");
    return;
  }
  std::shared_ptr<CSettingGroup> groupAudioSubmenuSel = AddGroup(category);
  if (groupAudioSubmenuSel == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group 'groupAudioSubmenuSel'");
    return;
  }
  std::shared_ptr<CSettingGroup> groupSaveAsDefault = AddGroup(category);
  if (groupSaveAsDefault == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogAudioSettings: unable to setup settings");
    return;
  }

  bool usePopup = g_SkinInfo->HasSkinFile("DialogSlider.xml");

  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  
  if (g_application.m_pPlayer->HasPlayer())
  {
    g_application.m_pPlayer->GetAudioCapabilities(m_audioCaps);
  }

  // register IsPlayingPassthrough condition
  GetSettingsManager()->AddCondition("IsPlayingPassthrough", IsPlayingPassthrough);

  CSettingDependency dependencyAudioOutputPassthroughDisabled(SettingDependencyType::Enable, GetSettingsManager());
  dependencyAudioOutputPassthroughDisabled.Or()
    ->Add(CSettingDependencyConditionPtr(new CSettingDependencyCondition(SETTING_AUDIO_PASSTHROUGH, "false", SettingDependencyOperator::Equals, false, GetSettingsManager())))
    ->Add(CSettingDependencyConditionPtr(new CSettingDependencyCondition("IsPlayingPassthrough", "", "", true, GetSettingsManager())));
  SettingDependencies depsAudioOutputPassthroughDisabled;
  depsAudioOutputPassthroughDisabled.push_back(dependencyAudioOutputPassthroughDisabled);
  
  // audio settings
  // audio volume setting
  m_volume = g_application.GetVolume(false);
  std::shared_ptr<CSettingNumber> settingAudioVolume = AddSlider(groupAudio, SETTING_AUDIO_VOLUME, 13376, SettingLevel::Basic, m_volume, 14054, VOLUME_MINIMUM, VOLUME_MAXIMUM / 100.0f, VOLUME_MAXIMUM);
  settingAudioVolume->SetDependencies(depsAudioOutputPassthroughDisabled);
  std::static_pointer_cast<CSettingControlSlider>(settingAudioVolume->GetControl())->SetFormatter(SettingFormatterPercentAsDecibel);

  // audio volume amplification setting
  if (SupportsAudioFeature(IPC_AUD_AMP))
  {
    std::shared_ptr<CSettingNumber> settingAudioVolumeAmplification = AddSlider(groupAudio, SETTING_AUDIO_VOLUME_AMPLIFICATION, 660, SettingLevel::Basic, videoSettings.m_VolumeAmplification, 14054, VOLUME_DRC_MINIMUM * 0.01f, (VOLUME_DRC_MAXIMUM - VOLUME_DRC_MINIMUM) / 6000.0f, VOLUME_DRC_MAXIMUM * 0.01f);
    settingAudioVolumeAmplification->SetDependencies(depsAudioOutputPassthroughDisabled);
  }

  // audio delay setting
  if (SupportsAudioFeature(IPC_AUD_OFFSET))
  {
    std::shared_ptr<CSettingNumber> settingAudioDelay = AddSlider(groupAudio, SETTING_AUDIO_DELAY, 297, SettingLevel::Basic, videoSettings.m_AudioDelay, 0, -g_advancedSettings.m_videoAudioDelayRange, 0.025f, g_advancedSettings.m_videoAudioDelayRange, 297, usePopup);
    std::static_pointer_cast<CSettingControlSlider>(settingAudioDelay->GetControl())->SetFormatter(SettingFormatterDelay);
  }
  
  // audio stream setting
  if (SupportsAudioFeature(IPC_AUD_SELECT_STREAM))
  {
    AddAudioStreams(groupAudio, SETTING_AUDIO_STREAM);
  }

  // audio digital/analog setting
  if (SupportsAudioFeature(IPC_AUD_SELECT_OUTPUT))
  {
    m_passthrough = CServiceBroker::GetSettings().GetBool(CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH);
    AddToggle(groupAudio, SETTING_AUDIO_PASSTHROUGH, 348, SettingLevel::Basic, m_passthrough);
  }


  // AudioDSP related options
  //CSettingCategory *category = AddCategory(SETTING_AUDIO_CAT_MAIN, -1);
  //if (category == NULL)
  //{
  //  CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings category 'audiodspmainsettings'");
  //  return;
  //}

  //m_ActiveStreamId = 0; // CServiceBroker::GetADSP().GetActiveStreamId();
  //m_ActiveStreamProcess = nullptr; // CServiceBroker::GetADSP().GetDSPProcess(m_ActiveStreamId);
  //if (m_ActiveStreamId == (unsigned int)-1 || !m_ActiveStreamProcess)
  //{
  //  m_iCategory = FindCategoryIndex(SETTING_AUDIO_CAT_MAIN);
  //  Close(true);
  //  return;
  //}

  //int modeUniqueId;
  //m_ActiveStreamProcess->GetMasterModeTypeInformation(m_streamTypeUsed, m_baseTypeUsed, modeUniqueId);

  //int modesAvailable = 0;
  //for (int i = 0; i < AUDIODSP_ADDON_ASTREAM_AUTO; i++)
  //{
  //  m_MasterModes[i].clear();
  //  m_ActiveStreamProcess->GetAvailableMasterModes((AUDIODSP_ADDON_STREAMTYPE)i, m_MasterModes[i]);
  //  if (!m_MasterModes[i].empty()) modesAvailable++;
  //}

  AddButton(groupAudioSubmenuSel, SETTING_AUDIO_DSP_MANAGER, 24136, SettingLevel::Expert);

  //if (modesAvailable > 0)
  //{
    /* about size() > 1, it is always the fallback (ignore of master processing) present. */
    //StaticIntegerSettingOptions modeEntries;
    //if (m_MasterModes[AUDIODSP_ADDON_ASTREAM_BASIC].size() > 1)
    //  modeEntries.push_back(std::pair<int, int>(CServiceBroker::GetADSP().GetStreamTypeName(AUDIODSP_ADDON_ASTREAM_BASIC),   AUDIODSP_ADDON_ASTREAM_BASIC));
    //if (m_MasterModes[AUDIODSP_ADDON_ASTREAM_MUSIC].size() > 1)
    //  modeEntries.push_back(std::pair<int, int>(CServiceBroker::GetADSP().GetStreamTypeName(AUDIODSP_ADDON_ASTREAM_MUSIC),   AUDIODSP_ADDON_ASTREAM_MUSIC));
    //if (m_MasterModes[AUDIODSP_ADDON_ASTREAM_MOVIE].size() > 1)
    //  modeEntries.push_back(std::pair<int, int>(CServiceBroker::GetADSP().GetStreamTypeName(AUDIODSP_ADDON_ASTREAM_MOVIE),   AUDIODSP_ADDON_ASTREAM_MOVIE));
    //if (m_MasterModes[AUDIODSP_ADDON_ASTREAM_GAME].size() > 1)
    //  modeEntries.push_back(std::pair<int, int>(CServiceBroker::GetADSP().GetStreamTypeName(AUDIODSP_ADDON_ASTREAM_GAME),    AUDIODSP_ADDON_ASTREAM_GAME));
    //if (m_MasterModes[AUDIODSP_ADDON_ASTREAM_APP].size() > 1)
    //  modeEntries.push_back(std::pair<int, int>(CServiceBroker::GetADSP().GetStreamTypeName(AUDIODSP_ADDON_ASTREAM_APP),     AUDIODSP_ADDON_ASTREAM_APP));
    //if (m_MasterModes[AUDIODSP_ADDON_ASTREAM_MESSAGE].size() > 1)
    //  modeEntries.push_back(std::pair<int, int>(CServiceBroker::GetADSP().GetStreamTypeName(AUDIODSP_ADDON_ASTREAM_MESSAGE), AUDIODSP_ADDON_ASTREAM_MESSAGE));
    //if (m_MasterModes[AUDIODSP_ADDON_ASTREAM_PHONE].size() > 1)
    //  modeEntries.push_back(std::pair<int, int>(CServiceBroker::GetADSP().GetStreamTypeName(AUDIODSP_ADDON_ASTREAM_PHONE),   AUDIODSP_ADDON_ASTREAM_PHONE));
    //if (modesAvailable > 1 && m_MasterModes[m_streamTypeUsed].size() > 1)
    //  modeEntries.insert(modeEntries.begin(), std::pair<int, int>(CServiceBroker::GetADSP().GetStreamTypeName(AUDIODSP_ADDON_ASTREAM_AUTO), AUDIODSP_ADDON_ASTREAM_AUTO));

    //AddSpinner(groupAudioSubmenuSel,
    //           SETTING_AUDIO_MAIN_STREAMTYPE, 15021, 0,
    //           (AUDIODSP_ADDON_STREAMTYPE)CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterStreamTypeSel,
    //           modeEntries);
  //}

  //bool AddonMasterModeSetupPresent = false;
  //m_ModeList.clear();
  //for (unsigned int i = 0; i < m_MasterModes[m_streamTypeUsed].size(); i++)
  //{
  //  if (m_MasterModes[m_streamTypeUsed][i])
  //  {
  //    AUDIODSP_ADDON_ADDON addon;
  //    int modeId = m_MasterModes[m_streamTypeUsed][i]->ModeID();
  //    if (modeId == AUDIODSP_ADDON_MASTER_MODE_ID_PASSOVER || modeId >= AUDIODSP_ADDON_MASTER_MODE_ID_INTERNAL_TYPES)
  //    {
  //      m_ModeList.push_back(make_pair(g_localizeStrings.Get(m_MasterModes[m_streamTypeUsed][i]->ModeName()), modeId));
  //    }
  //    //else if (CServiceBroker::GetADSP().GetAudioDSPAddon(m_MasterModes[m_streamTypeUsed][i]->AddonID(), addon))
  //    //{
  //    //  m_ModeList.push_back(make_pair(g_localizeStrings.GetAddonString(addon->ID(), m_MasterModes[m_streamTypeUsed][i]->ModeName()), modeId));
  //    //  if (!AddonMasterModeSetupPresent)
  //    //    AddonMasterModeSetupPresent = m_MasterModes[m_streamTypeUsed][i]->HasSettingsDialog();
  //    //}
  //  }
  //}

    m_modeTypeUsed = 0;// CMediaSettings::GetInstance().GetCurrentAudioSettings().m_MasterModes[m_streamTypeUsed][m_baseTypeUsed];
  std::shared_ptr<CSettingInt> spinner = AddSpinner(groupAudioSubmenuSel, SETTING_AUDIO_MAIN_MODETYPE, 15022, SettingLevel::Expert, m_modeTypeUsed, AudioModeOptionFiller);
  spinner->SetOptionsFiller(AudioModeOptionFiller, this);

  ///-----------------------

  AddButton(groupAudioSubmenuSel, SETTING_AUDIO_MAIN_BUTTON_PRE_PROC, 15039, SettingLevel::Expert);
  AddButton(groupAudioSubmenuSel, SETTING_AUDIO_MAIN_BUTTON_MASTER,   15025, SettingLevel::Expert);
  AddButton(groupAudioSubmenuSel, SETTING_AUDIO_MAIN_BUTTON_RESAMPLE, 15033, SettingLevel::Expert);
  AddButton(groupAudioSubmenuSel, SETTING_AUDIO_MAIN_BUTTON_OUTPUT,   15026, SettingLevel::Expert);
  AddButton(groupAudioSubmenuSel, SETTING_AUDIO_MAIN_BUTTON_INFO,     15027, SettingLevel::Expert);

  ///-----------------------

  AddButton(groupSaveAsDefault, SETTING_AUDIO_MAKE_DEFAULT, 12376, SettingLevel::Expert);

  m_Menus.clear();

  /**
   * Audio Master mode settings Dialog init
   */
  {
    std::shared_ptr<CSettingCategory> categoryMaster = AddCategory(SETTING_AUDIO_CAT_MASTER, -1);
    if (categoryMaster == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings category 'audiodspmastersettings'");
      return;
    }

    std::shared_ptr<CSettingGroup> groupMasterMode = AddGroup(categoryMaster);
    if (groupMasterMode == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group 'groupMasterMode'");
      return;
    }

    //for (unsigned int i = 0; i < m_MasterModes[m_streamTypeUsed].size(); i++)
    //{
    //  if (m_MasterModes[m_streamTypeUsed][i]->HasSettingsDialog())
    //  {
        //AUDIODSP_ADDON_ADDON addon;
        //if (CServiceBroker::GetADSP().GetAudioDSPAddon(m_MasterModes[m_streamTypeUsed][i]->AddonID(), addon))
        //{
        //  AE_DSP_MENUHOOKS hooks;
        //  if (CServiceBroker::GetADSP().GetMenuHooks(m_MasterModes[m_streamTypeUsed][i]->AddonID(), AE_DSP_MENUHOOK_MASTER_PROCESS, hooks))
        //  {
        //    for (unsigned int j = 0; j < hooks.size(); j++)
        //    {
        //      if (hooks[j].iRelevantModeId != m_MasterModes[m_streamTypeUsed][i]->AddonModeNumber())
        //        continue;

        //      MenuHookMember menu;
        //      menu.addonId                  = m_MasterModes[m_streamTypeUsed][i]->AddonID();
        //      menu.hook.category            = hooks[j].category;
        //      menu.hook.iHookId             = hooks[j].iHookId;
        //      menu.hook.iLocalizedStringId  = hooks[j].iLocalizedStringId;
        //      menu.hook.iRelevantModeId     = hooks[j].iRelevantModeId;
        //      m_Menus.push_back(menu);

        //      std::string setting = StringUtils::Format("%s%i", SETTING_AUDIO_MASTER_SETTINGS_MENUS, (int)m_Menus.size()-1);
        //      AddButton(groupMasterMode, setting, 15041, 0);
        //      break;
        //    }
        //  }
        //}
    //  }
    //}
  }

  /**
   * Audio post processing settings Dialog init
   */
  {
    std::shared_ptr<CSettingCategory> category = AddCategory(SETTING_AUDIO_CAT_POST_PROCESS, -1);
    if (category == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings category 'audiodsppostsettings'");
      return;
    }

    std::shared_ptr<CSettingGroup> groupInternal = AddGroup(category);
    if (groupInternal == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group 'groupInternal'");
      return;
    }

    std::shared_ptr<CSettingGroup> groupAddon = AddGroup(category);
    if (groupAddon == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group 'groupAddon'");
      return;
    }

    //GetAudioDSPMenus(groupAddon, AE_DSP_MENUHOOK_POST_PROCESS);
  }

  /**
   * Audio add-on resampling setting dialog's
   */
  {
    std::shared_ptr<CSettingCategory> category = AddCategory(SETTING_AUDIO_CAT_RESAMPLING, -1);
    if (category == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings category 'audiodspresamplesettings'");
      return;
    }
    std::shared_ptr<CSettingGroup> group = AddGroup(category);
    if (group == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group 'group'");
      return;
    }
    //GetAudioDSPMenus(group, AUDIODSP_ADDON_MENUHOOK_RESAMPLE);
  }

  /**
   * Audio add-on's pre processing setting dialog's
   */
  {
    std::shared_ptr<CSettingCategory> category = AddCategory(SETTING_AUDIO_CAT_PRE_PROCESS, -1);
    if (category == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings category 'audiodsppresettings'");
      return;
    }
    std::shared_ptr<CSettingGroup> group = AddGroup(category);
    if (group == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group 'group'");
      return;
    }
    //GetAudioDSPMenus(group, AUDIODSP_ADDON_MENUHOOK_PRE_PROCESS);
  }

  /**
   * Audio Information Dialog init
   */
  {
    std::shared_ptr<CSettingGroup> group;
    std::shared_ptr<CSettingCategory> category = AddCategory(SETTING_AUDIO_CAT_PROC_INFO, -1);
    if (category == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings category 'audiodspprocinfo'");
      return;
    }

    m_ActiveModes.clear();
    //m_ActiveStreamProcess->GetActiveModes(AUDIODSP_ADDON_MODE_TYPE_UNDEFINED, m_ActiveModes);
    m_ActiveModesData.resize(m_ActiveModes.size());

    group = AddGroup(category, 15089);
    if (group == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group for '%s'", g_localizeStrings.Get(15089).c_str());
      return;
    }
//! @todo reimplement this with AudioDSP V2.0
//    m_InputChannels = StringUtils::Format("%i", m_ActiveStreamProcess->GetInputChannels());
//    AddInfoLabelButton(group, SETTING_STREAM_INFO_INPUT_CHANNELS, 21444, 0, m_InputChannels);
//    m_InputChannelNames = m_ActiveStreamProcess->GetInputChannelNames();
//    AddInfoLabelButton(group, SETTING_STREAM_INFO_INPUT_CHANNEL_NAMES, 15091, 0, m_InputChannelNames);
//    m_InputSamplerate = StringUtils::Format("%i Hz", (int)m_ActiveStreamProcess->GetInputSamplerate());
//    AddInfoLabelButton(group, SETTING_STREAM_INFO_INPUT_SAMPLERATE, 613, 0, m_InputSamplerate);

    group = AddGroup(category, 15090);
    if (group == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group for '%s'", g_localizeStrings.Get(15090).c_str());
      return;
    }
//! @todo reimplement this with AudioDSP V2.0
//    m_OutputChannels = StringUtils::Format("%i", m_ActiveStreamProcess->GetOutputChannels());
//    AddInfoLabelButton(group, SETTING_STREAM_INFO_OUTPUT_CHANNELS, 21444, 0, m_OutputChannels);
//    m_OutputChannelNames = m_ActiveStreamProcess->GetOutputChannelNames();
//    AddInfoLabelButton(group, SETTING_STREAM_INFO_OUTPUT_CHANNEL_NAMES, 15091, 0, m_OutputChannelNames);
//    m_OutputSamplerate = StringUtils::Format("%i Hz", (int)m_ActiveStreamProcess->GetOutputSamplerate());
//    AddInfoLabelButton(group, SETTING_STREAM_INFO_OUTPUT_SAMPLERATE, 613, 0, m_OutputSamplerate);

    group = AddGroup(category, 15081);
    if (group == NULL)
    {
      CLog::Log(LOGERROR, "CGUIDialogAudioDSPSettings: unable to setup settings group for '%s'", g_localizeStrings.Get(15081).c_str());
      return;
    }
    m_CPUUsage = "";// StringUtils::Format("%.02f %%", /*m_ActiveStreamProcess->GetCPUUsage()*/0);
    AddInfoLabelButton(group, SETTING_STREAM_INFO_CPU_USAGE, 15092, SettingLevel::Expert, m_CPUUsage);

    bool foundPreProcess = false, foundPostProcess = false;
    for (unsigned int i = 0; i < m_ActiveModes.size(); i++)
    {
      AE_DSP_ADDON addon;
//      if (CServiceBroker::GetADSP().GetAudioDSPAddon(m_ActiveModes[i]->AddonID(), addon))
//      {
//        std::string label;
//        switch (m_ActiveModes[i]->ModeType())
//        {
//          case AE_DSP_MODE_TYPE_INPUT_RESAMPLE:
//            group = AddGroup(category, 15087, -1, true, true);
//            label = StringUtils::Format(g_localizeStrings.Get(15082).c_str(), m_ActiveStreamProcess->GetProcessSamplerate());
//            break;
//          case AE_DSP_MODE_TYPE_OUTPUT_RESAMPLE:
//            group = AddGroup(category, 15088, -1, true, true);
////            label = StringUtils::Format(g_localizeStrings.Get(15083).c_str(), m_ActiveStreamProcess->GetOutputSamplerate());
//            break;
//          case AE_DSP_MODE_TYPE_MASTER_PROCESS:
//            group = AddGroup(category, 15084, -1, true, true);
//            label = g_localizeStrings.GetAddonString(addon->ID(), m_ActiveModes[i]->ModeName());
//            break;
//          case AE_DSP_MODE_TYPE_PRE_PROCESS:
//            if (!foundPreProcess)
//            {
//              foundPreProcess = true;
//              group = AddGroup(category, 15085, -1, true, true);
//            }
//            label = g_localizeStrings.GetAddonString(addon->ID(), m_ActiveModes[i]->ModeName());
//            break;
//          case AE_DSP_MODE_TYPE_POST_PROCESS:
//            if (!foundPostProcess)
//            {
//              foundPostProcess = true;
//              group = AddGroup(category, 15086, -1, true, true);
//            }
//            label = g_localizeStrings.GetAddonString(addon->ID(), m_ActiveModes[i]->ModeName());
//            break;
//          default:
//          {
//            label += g_localizeStrings.GetAddonString(addon->ID(), m_ActiveModes[i]->ModeName());
//            label += " - ";
//            label += addon->GetFriendlyName();
//          }
//        };
//        m_ActiveModesData[i].CPUUsage = StringUtils::Format("%.02f %%", m_ActiveModes[i]->CPUUsage());
//
//        MenuHookMember menu;
//        menu.addonId = -1;
//
//        AE_DSP_MENUHOOKS hooks;
//        m_ActiveModesData[i].MenuListPtr = -1;
//        if (CServiceBroker::GetADSP().GetMenuHooks(m_ActiveModes[i]->AddonID(), AE_DSP_MENUHOOK_INFORMATION, hooks))
//        {
//          for (unsigned int j = 0; j < hooks.size(); j++)
//          {
//            if (hooks[j].iRelevantModeId != m_ActiveModes[i]->AddonModeNumber())
//              continue;
//
//            menu.addonId                  = m_ActiveModes[i]->AddonID();
//            menu.hook.category            = hooks[j].category;
//            menu.hook.iHookId             = hooks[j].iHookId;
//            menu.hook.iLocalizedStringId  = hooks[j].iLocalizedStringId;
//            menu.hook.iRelevantModeId     = hooks[j].iRelevantModeId;
//            m_Menus.push_back(menu);
//            m_ActiveModesData[i].MenuListPtr = m_Menus.size()-1;
//            label += " ...";
//            break;
//          }
//        }
//        m_ActiveModesData[i].MenuName = label;
//
//        std::string settingId = StringUtils::Format("%s%i", SETTING_STREAM_INFO_MODE_CPU_USAGE, i);
//        AddInfoLabelButton(group, settingId, 15041, 0, m_ActiveModesData[i].CPUUsage);
//      }
    }
  }
}

bool CGUIDialogAudioSettings::SupportsAudioFeature(int feature)
{
  for (Features::iterator itr = m_audioCaps.begin(); itr != m_audioCaps.end(); ++itr)
  {
    if (*itr == feature || *itr == IPC_AUD_ALL)
      return true;
  }

  return false;
}

void CGUIDialogAudioSettings::AddAudioStreams(std::shared_ptr<CSettingGroup> group, const std::string &settingId)
{
  if (group == NULL || settingId.empty())
    return;

  m_audioStream = g_application.m_pPlayer->GetAudioStream();
  if (m_audioStream < 0)
    m_audioStream = 0;

  AddList(group, settingId, 460, SettingLevel::Basic, m_audioStream, AudioStreamsOptionFiller, 460);
}

bool CGUIDialogAudioSettings::IsPlayingPassthrough(const std::string &condition, const std::string &value, std::shared_ptr<const CSetting> setting, void *data)
{
  return g_application.m_pPlayer->IsPassthrough();
}

void CGUIDialogAudioSettings::AudioStreamsOptionFiller(std::shared_ptr<const CSetting> setting, std::vector< std::pair<std::string, int> > &list, int &current, void *data)
{
  int audioStreamCount = g_application.m_pPlayer->GetAudioStreamCount();

  // cycle through each audio stream and add it to our list control
  for (int i = 0; i < audioStreamCount; ++i)
  {
    std::string strItem;
    std::string strLanguage;

    SPlayerAudioStreamInfo info;
    g_application.m_pPlayer->GetAudioStreamInfo(i, info);

    if (!g_LangCodeExpander.Lookup(info.language, strLanguage))
      strLanguage = g_localizeStrings.Get(13205); // Unknown

    if (info.name.length() == 0)
      strItem = strLanguage;
    else
      strItem = StringUtils::Format("%s - %s", strLanguage.c_str(), info.name.c_str());

    strItem += StringUtils::Format(" (%i/%i)", i + 1, audioStreamCount);
    list.push_back(make_pair(strItem, i));
  }

  if (list.empty())
  {
    list.push_back(make_pair(g_localizeStrings.Get(231), -1));
    current = -1;
  }
}

std::string CGUIDialogAudioSettings::SettingFormatterDelay(std::shared_ptr<const CSettingControlSlider> control, const CVariant &value, const CVariant &minimum, const CVariant &step, const CVariant &maximum)
{
  if (!value.isDouble())
    return "";

  float fValue = value.asFloat();
  float fStep = step.asFloat();

  if (fabs(fValue) < 0.5f * fStep)
    return StringUtils::Format(g_localizeStrings.Get(22003).c_str(), 0.0);
  if (fValue < 0)
    return StringUtils::Format(g_localizeStrings.Get(22004).c_str(), fabs(fValue));

  return StringUtils::Format(g_localizeStrings.Get(22005).c_str(), fValue);
}

std::string CGUIDialogAudioSettings::SettingFormatterPercentAsDecibel(std::shared_ptr<const CSettingControlSlider> control, const CVariant &value, const CVariant &minimum, const CVariant &step, const CVariant &maximum)
{
  if (control == NULL || !value.isDouble())
    return "";

  std::string formatString = control->GetFormatString();
  if (control->GetFormatLabel() > -1)
    formatString = g_localizeStrings.Get(control->GetFormatLabel());

  return StringUtils::Format(formatString.c_str(), CAEUtil::PercentToGain(value.asFloat()));
}

void CGUIDialogAudioSettings::AudioModeOptionFiller(std::shared_ptr<const CSetting> setting, std::vector< std::pair<std::string, int> > &list, int &current, void *data)
{
  CGUIDialogAudioSettings *dialog = reinterpret_cast<CGUIDialogAudioSettings*>(data);
  list = dialog->m_ModeList;

  if (list.empty())
  {
    list.push_back(make_pair(g_localizeStrings.Get(231), -1));
    current = -1;
  }
}

string CGUIDialogAudioSettings::GetSettingsLabel(CSetting *pSetting)
{
  if (pSetting->GetLabel() == 15041)
  {
    const std::string &settingId = pSetting->GetId();

    int ptr = -1;
    AE_DSP_ADDON addon;
    //if (settingId.substr(0, 27) == SETTING_STREAM_INFO_MODE_CPU_USAGE)
    //{
    //  ptr = strtol(settingId.substr(27).c_str(), NULL, 0);
    //  if (ptr >= 0 && CServiceBroker::GetADSP().GetAudioDSPAddon(m_ActiveModes[ptr]->AddonID(), addon))
    //    return m_ActiveModesData[ptr].MenuName;
    //}
    //else
    //{
    //  if (settingId.substr(0, 21) == SETTING_AUDIO_MASTER_SETTINGS_MENUS)
    //    ptr = strtol(settingId.substr(21).c_str(), NULL, 0);
    //  else if (settingId.substr(0, 19) == SETTING_AUDIO_PROC_SETTINGS_MENUS)
    //    ptr = strtol(settingId.substr(19).c_str(), NULL, 0);

    //  if (ptr >= 0 && CServiceBroker::GetADSP().GetAudioDSPAddon(m_Menus[ptr].addonId, addon))
    //    return g_localizeStrings.GetAddonString(addon->ID(), m_Menus[ptr].hook.iLocalizedStringId);
    //}
  }

  return GetLocalizedString(pSetting->GetLabel());
}

void CGUIDialogAudioSettings::OpenMenu(const std::string &id)
{
  m_GetCPUUsage = false;
  m_MenuPositions[m_categories[m_iCategory]->GetId()] = GetFocusedControlID();
  m_MenuHierarchy.push_back(m_iCategory);
  m_iCategory = FindCategoryIndex(id);

  /* Get menu name */
  m_MenuName = -1;
  if (id == SETTING_AUDIO_CAT_MAIN)
  {
    m_MenuName = 13396;
  }
  else if (id == SETTING_AUDIO_CAT_MASTER)
  {
    m_MenuName = 15029;
  }
  else if (id == SETTING_AUDIO_CAT_POST_PROCESS)
  {
    m_MenuName = 15030;
  }
  else if (id == SETTING_AUDIO_CAT_RESAMPLING)
  {
    m_MenuName = 15035;
  }
  else if (id == SETTING_AUDIO_CAT_PRE_PROCESS)
  {
    m_MenuName = 15037;
  }
  else if (id == SETTING_AUDIO_CAT_PROC_INFO)
  {
    m_MenuName = 15031;
  }

  SetHeading(g_localizeStrings.Get(m_MenuName));
  CreateSettings();
  SET_CONTROL_FOCUS(m_MenuPositions[id], 0);
}

bool CGUIDialogAudioSettings::HasActiveMenuHooks(AUDIODSP_MENU_HOOK_CAT category)
{
  /*!> Check menus are active on current stream */
  AE_DSP_ADDONMAP addonMap;
  //if (CServiceBroker::GetADSP().HaveMenuHooks(category) &&
  //    CServiceBroker::GetADSP().GetEnabledAudioDSPAddons(addonMap) > 0)
  //{
  //  for (AE_DSP_ADDONMAP_ITR itr = addonMap.begin(); itr != addonMap.end(); itr++)
  //  {
  //    AE_DSP_MENUHOOKS hooks;
  //    if (CServiceBroker::GetADSP().GetMenuHooks(itr->second->GetID(), category, hooks))
  //    {
  //      for (unsigned int i = 0; i < hooks.size(); i++)
  //      {
  //        if (category != AE_DSP_MENUHOOK_MISCELLANEOUS &&
  //            !m_ActiveStreamProcess->IsMenuHookModeActive(hooks[i].category, itr->second->GetID(), hooks[i].iRelevantModeId))
  //          continue;

  //        return true;
  //      }
  //    }
  //  }
  //}

  return false;
}

void CGUIDialogAudioSettings::GetAudioDSPMenus(std::shared_ptr<CSettingGroup> group, AUDIODSP_MENU_HOOK_CAT category)
{
  AE_DSP_ADDONMAP addonMap;
  //if (CServiceBroker::GetADSP().GetEnabledAudioDSPAddons(addonMap) > 0)
  //{
  //  for (AE_DSP_ADDONMAP_ITR itr = addonMap.begin(); itr != addonMap.end(); itr++)
  //  {
  //    AE_DSP_MENUHOOKS hooks;
  //    if (CServiceBroker::GetADSP().GetMenuHooks(itr->second->GetID(), category, hooks))
  //    {
  //      for (unsigned int i = 0; i < hooks.size(); i++)
  //      {
  //        if (category != hooks[i].category || (category != AE_DSP_MENUHOOK_MISCELLANEOUS &&
  //            !m_ActiveStreamProcess->IsMenuHookModeActive(hooks[i].category, itr->second->GetID(), hooks[i].iRelevantModeId)))
  //          continue;

  //        MenuHookMember menu;
  //        menu.addonId                  = itr->second->GetID();
  //        menu.hook.category            = hooks[i].category;
  //        menu.hook.iHookId             = hooks[i].iHookId;
  //        menu.hook.iLocalizedStringId  = hooks[i].iLocalizedStringId;
  //        menu.hook.iRelevantModeId     = hooks[i].iRelevantModeId;
  //        m_Menus.push_back(menu);
  //      }
  //    }
  //  }
  //}

  for (unsigned int i = 0; i < m_Menus.size(); i++)
  {
    AE_DSP_ADDON addon;
    //if (CServiceBroker::GetADSP().GetAudioDSPAddon(m_Menus[i].addonId, addon) && category == m_Menus[i].hook.category)
    //{
    //  std::string modeName = g_localizeStrings.GetAddonString(addon->ID(), m_Menus[i].hook.iLocalizedStringId);
    //  if (modeName.empty())
    //    modeName = g_localizeStrings.Get(15041);

    //  std::string setting = StringUtils::Format("%s%i", SETTING_AUDIO_PROC_SETTINGS_MENUS, i);
    //  AddButton(group, setting, 15041, 0);
    //}
  }
}

bool CGUIDialogAudioSettings::OpenAudioDSPMenu(unsigned int setupEntry)
{
  if (setupEntry >= m_Menus.size())
    return false;

  AE_DSP_ADDON addon;
  return false;
  //if (!CServiceBroker::GetADSP().GetAudioDSPAddon(m_Menus[setupEntry].addonId, addon))
  //  return false;

  AUDIODSP_MENU_HOOK hook;

  hook.category           = m_Menus[setupEntry].hook.category;
  hook.iHookId            = m_Menus[setupEntry].hook.iHookId;
  hook.iLocalizedStringId = m_Menus[setupEntry].hook.iLocalizedStringId;
  hook.iRelevantModeId    = m_Menus[setupEntry].hook.iRelevantModeId;
  switch (hook.category)
  {
    default:
      return false;
    break;
  }

  /*!
   * @note the addon dialog becomes always opened on the back of Kodi ones for this reason a
   * "<animation effect="fade" start="100" end="0" time="400" condition="Window.IsVisible(Addon)">Conditional</animation>"
   * on skin is needed to hide dialog.
   */
  addon->CallMenuHook(hook);

  return true;
}

int CGUIDialogAudioSettings::FindCategoryIndex(const std::string &catId)
{
  for (unsigned int i =  0; i < m_categories.size(); i++)
  {
    if (m_categories[i]->GetId() == catId)
      return i;
  }
  return 0;
}
