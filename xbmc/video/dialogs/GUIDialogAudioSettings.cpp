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
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "cores/IPlayer.h"
#include "dialogs/GUIDialogFileBrowser.h"
#include "dialogs/GUIDialogYesNo.h"
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

#define SETTING_AUDIO_VOLUME                   "audio.volume"
#define SETTING_AUDIO_VOLUME_AMPLIFICATION     "audio.volumeamplification"
#define SETTING_AUDIO_DELAY                    "audio.delay"
#define SETTING_AUDIO_STREAM                   "audio.stream"
#define SETTING_AUDIO_PASSTHROUGH              "audio.digitalanalog"
#define SETTING_AUDIO_DSP_MANAGER              "audio.dspmanager"
#define SETTING_AUDIO_DSP_SETTINGS             "audio.dspsettings"

#define SETTING_AUDIO_MAKE_DEFAULT             "audio.makedefault"

CGUIDialogAudioSettings::CGUIDialogAudioSettings()
  : CGUIDialogSettingsManualBase(WINDOW_DIALOG_AUDIO_OSD_SETTINGS, "DialogSettings.xml"),
    m_passthrough(false)
{ }

CGUIDialogAudioSettings::~CGUIDialogAudioSettings()
{ }

void CGUIDialogAudioSettings::FrameMove()
{
  // update the volume setting if necessary
  float newVolume = g_application.GetVolume(false);
  if (newVolume != m_volume)
    m_settingsManager->SetNumber(SETTING_AUDIO_VOLUME, newVolume);

  if (g_application.m_pPlayer->HasPlayer())
  {
    const CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
    
    // these settings can change on the fly
    //! @todo (needs special handling): m_settingsManager->SetInt(SETTING_AUDIO_STREAM, g_application.m_pPlayer->GetAudioStream());
    m_settingsManager->SetNumber(SETTING_AUDIO_DELAY, videoSettings.m_AudioDelay);
    m_settingsManager->SetBool(SETTING_AUDIO_PASSTHROUGH, CSettings::GetInstance().GetBool(CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH));
  }

  CGUIDialogSettingsManualBase::FrameMove();
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

void CGUIDialogAudioSettings::OnSettingChanged(const CSetting *setting)
{
  if (setting == NULL)
    return;

  CGUIDialogSettingsManualBase::OnSettingChanged(setting);
  
  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  const std::string &settingId = setting->GetId();
  if (settingId == SETTING_AUDIO_VOLUME)
  {
    m_volume = static_cast<float>(static_cast<const CSettingNumber*>(setting)->GetValue());
    g_application.SetVolume(m_volume, false); // false - value is not in percent
  }
  else if (settingId == SETTING_AUDIO_VOLUME_AMPLIFICATION)
  {
    videoSettings.m_VolumeAmplification = static_cast<float>(static_cast<const CSettingNumber*>(setting)->GetValue());
    g_application.m_pPlayer->SetDynamicRangeCompression((long)(videoSettings.m_VolumeAmplification * 100));
  }
  else if (settingId == SETTING_AUDIO_DELAY)
  {
    videoSettings.m_AudioDelay = static_cast<float>(static_cast<const CSettingNumber*>(setting)->GetValue());
    g_application.m_pPlayer->SetAVDelay(videoSettings.m_AudioDelay);
  }
  else if (settingId == SETTING_AUDIO_STREAM)
  {
    m_audioStream = static_cast<const CSettingInt*>(setting)->GetValue();
    // only change the audio stream if a different one has been asked for
    if (g_application.m_pPlayer->GetAudioStream() != m_audioStream)
    {
      videoSettings.m_AudioStream = m_audioStream;
      g_application.m_pPlayer->SetAudioStream(m_audioStream);    // Set the audio stream to the one selected
    }
  }
  else if (settingId == SETTING_AUDIO_PASSTHROUGH)
  {
    m_passthrough = static_cast<const CSettingBool*>(setting)->GetValue();
    CSettings::GetInstance().SetBool(CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH, m_passthrough);
  }
}

void CGUIDialogAudioSettings::OnSettingAction(const CSetting *setting)
{
  if (setting == NULL)
    return;

  CGUIDialogSettingsManualBase::OnSettingAction(setting);
  
  const std::string &settingId = setting->GetId();
  if (settingId == SETTING_AUDIO_DSP_MANAGER)
  {
    g_windowManager.ActivateWindow(WINDOW_DIALOG_AUDIO_DSP_MANAGER);
  }
  else if (settingId == SETTING_AUDIO_DSP_SETTINGS)
  {
    g_windowManager.ActivateWindow(WINDOW_DIALOG_AUDIO_DSP_OSD_SETTINGS);
  }
  else if (settingId == SETTING_AUDIO_MAKE_DEFAULT)
  {
    Save();
  }
}

void CGUIDialogAudioSettings::Save()
{
  if (!g_passwordManager.CheckSettingLevelLock(SettingLevelExpert) &&
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

  CMediaSettings::GetInstance().GetDefaultVideoSettings() = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  CMediaSettings::GetInstance().GetDefaultVideoSettings().m_AudioStream = -1;
  CSettings::GetInstance().Save();
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

  CSettingCategory *category = AddCategory("audiosettings", -1);
  if (category == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogAudioSettings: unable to setup settings");
    return;
  }

  // get all necessary setting groups
  CSettingGroup *groupAudio = AddGroup(category);
  if (groupAudio == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogAudioSettings: unable to setup settings");
    return;
  }
  CSettingGroup *groupSaveAsDefault = AddGroup(category);
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
  m_settingsManager->AddCondition("IsPlayingPassthrough", IsPlayingPassthrough);

  CSettingDependency dependencyAudioOutputPassthroughDisabled(SettingDependencyTypeEnable, m_settingsManager);
  dependencyAudioOutputPassthroughDisabled.Or()
    ->Add(CSettingDependencyConditionPtr(new CSettingDependencyCondition(SETTING_AUDIO_PASSTHROUGH, "false", SettingDependencyOperatorEquals, false, m_settingsManager)))
    ->Add(CSettingDependencyConditionPtr(new CSettingDependencyCondition("IsPlayingPassthrough", "", "", true, m_settingsManager)));
  SettingDependencies depsAudioOutputPassthroughDisabled;
  depsAudioOutputPassthroughDisabled.push_back(dependencyAudioOutputPassthroughDisabled);

  // audio settings
  // audio volume setting
  m_volume = g_application.GetVolume(false);
  CSettingNumber *settingAudioVolume = AddSlider(groupAudio, SETTING_AUDIO_VOLUME, 13376, 0, m_volume, 14054, VOLUME_MINIMUM, VOLUME_MAXIMUM / 100.0f, VOLUME_MAXIMUM);
  settingAudioVolume->SetDependencies(depsAudioOutputPassthroughDisabled);
  static_cast<CSettingControlSlider*>(settingAudioVolume->GetControl())->SetFormatter(SettingFormatterPercentAsDecibel);


  // audio volume amplification setting
  if (SupportsAudioFeature(IPC_AUD_AMP))
  {
    CSettingNumber *settingAudioVolumeAmplification = AddSlider(groupAudio, SETTING_AUDIO_VOLUME_AMPLIFICATION, 660, 0, videoSettings.m_VolumeAmplification, 14054, VOLUME_DRC_MINIMUM * 0.01f, (VOLUME_DRC_MAXIMUM - VOLUME_DRC_MINIMUM) / 6000.0f, VOLUME_DRC_MAXIMUM * 0.01f);
    settingAudioVolumeAmplification->SetDependencies(depsAudioOutputPassthroughDisabled);
  }

  // audio delay setting
  if (SupportsAudioFeature(IPC_AUD_OFFSET))
  {
    CSettingNumber *settingAudioDelay = AddSlider(groupAudio, SETTING_AUDIO_DELAY, 297, 0, videoSettings.m_AudioDelay, 0, -g_advancedSettings.m_videoAudioDelayRange, 0.025f, g_advancedSettings.m_videoAudioDelayRange, 297, usePopup);
    static_cast<CSettingControlSlider*>(settingAudioDelay->GetControl())->SetFormatter(SettingFormatterDelay);
  }
  
  // audio stream setting
  if (SupportsAudioFeature(IPC_AUD_SELECT_STREAM))
    AddAudioStreams(groupAudio, SETTING_AUDIO_STREAM);

  AddButton(groupAudio, SETTING_AUDIO_DSP_SETTINGS, 24136, 0);
  AddButton(groupAudio, SETTING_AUDIO_DSP_MANAGER, 24136, 0);

  // audio digital/analog setting
  if (SupportsAudioFeature(IPC_AUD_SELECT_OUTPUT))
  {
    m_passthrough = CSettings::GetInstance().GetBool(CSettings::SETTING_AUDIOOUTPUT_PASSTHROUGH);
    AddToggle(groupAudio, SETTING_AUDIO_PASSTHROUGH, 348, 0, m_passthrough);
  }

  AddButton(groupSaveAsDefault, SETTING_AUDIO_MAKE_DEFAULT, 12376, 0);
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

void CGUIDialogAudioSettings::AddAudioStreams(CSettingGroup *group, const std::string &settingId)
{
  if (group == NULL || settingId.empty())
    return;

  m_audioStream = g_application.m_pPlayer->GetAudioStream();
  if (m_audioStream < 0)
    m_audioStream = 0;

  AddList(group, settingId, 460, 0, m_audioStream, AudioStreamsOptionFiller, 460);
}

bool CGUIDialogAudioSettings::IsPlayingPassthrough(const std::string &condition, const std::string &value, const CSetting *setting, void *data)
{
  return g_application.m_pPlayer->IsPassthrough();
}

void CGUIDialogAudioSettings::AudioStreamsOptionFiller(const CSetting *setting, std::vector< std::pair<std::string, int> > &list, int &current, void *data)
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

std::string CGUIDialogAudioSettings::SettingFormatterDelay(const CSettingControlSlider *control, const CVariant &value, const CVariant &minimum, const CVariant &step, const CVariant &maximum)
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

std::string CGUIDialogAudioSettings::SettingFormatterPercentAsDecibel(const CSettingControlSlider *control, const CVariant &value, const CVariant &minimum, const CVariant &step, const CVariant &maximum)
{
  if (control == NULL || !value.isDouble())
    return "";

  std::string formatString = control->GetFormatString();
  if (control->GetFormatLabel() > -1)
    formatString = g_localizeStrings.Get(control->GetFormatLabel());

  return StringUtils::Format(formatString.c_str(), CAEUtil::PercentToGain(value.asFloat()));
}
