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

#include "GUIDialogSubtitleSettings.h"

#include <string>
#include <vector>

#include "addons/Skin.h"
#include "Application.h"
#include "ServiceBroker.h"
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

#define SETTING_SUBTITLE_ENABLE                "subtitles.enable"
#define SETTING_SUBTITLE_DELAY                 "subtitles.delay"
#define SETTING_SUBTITLE_STREAM                "subtitles.stream"
#define SETTING_SUBTITLE_BROWSER               "subtitles.browser"

#define SETTING_SUBTITLE_MAKE_DEFAULT          "subtitles.makedefault"

CGUIDialogSubtitleSettings::CGUIDialogSubtitleSettings()
  : CGUIDialogSettingsManualBase(WINDOW_DIALOG_SUBTITLE_OSD_SETTINGS, "DialogSettings.xml")
{ }

CGUIDialogSubtitleSettings::~CGUIDialogSubtitleSettings()
{ }

void CGUIDialogSubtitleSettings::FrameMove()
{
  if (g_application.m_pPlayer->HasPlayer())
  {
    const CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();

    //! @todo m_settingsManager->SetBool(SETTING_SUBTITLE_ENABLE, g_application.m_pPlayer->GetSubtitleVisible());
    //   \-> Unless subtitle visibility can change on the fly, while Dialog is up, this code should be removed.
    GetSettingsManager()->SetNumber(SETTING_SUBTITLE_DELAY, videoSettings.m_SubtitleDelay);
    //! @todo (needs special handling): m_settingsManager->SetInt(SETTING_SUBTITLE_STREAM, g_application.m_pPlayer->GetSubtitle());
  }

  CGUIDialogSettingsManualBase::FrameMove();
}

std::string CGUIDialogSubtitleSettings::BrowseForSubtitle()
{
  std::string strPath;
  if (URIUtils::IsInRAR(g_application.CurrentFileItem().GetPath()) || URIUtils::IsInZIP(g_application.CurrentFileItem().GetPath()))
    strPath = CURL(g_application.CurrentFileItem().GetPath()).GetHostName();
  else
    strPath = g_application.CurrentFileItem().GetPath();

  std::string strMask = ".utf|.utf8|.utf-8|.sub|.srt|.smi|.rt|.txt|.ssa|.aqt|.jss|.ass|.idx|.rar|.zip";
  if (g_application.GetCurrentPlayer() == "VideoPlayer")
    strMask = ".srt|.rar|.zip|.ifo|.smi|.sub|.idx|.ass|.ssa|.txt";
  VECSOURCES shares(*CMediaSourceSettings::GetInstance().GetSources("video"));
  if (CMediaSettings::GetInstance().GetAdditionalSubtitleDirectoryChecked() != -1 && !CServiceBroker::GetSettings().GetString(CSettings::SETTING_SUBTITLES_CUSTOMPATH).empty())
  {
    CMediaSource share;
    std::vector<std::string> paths;
    paths.push_back(URIUtils::GetDirectory(strPath));
    paths.push_back(CServiceBroker::GetSettings().GetString(CSettings::SETTING_SUBTITLES_CUSTOMPATH));
    share.FromNameAndPaths("video",g_localizeStrings.Get(21367),paths);
    shares.push_back(share);
    strPath = share.strPath;
    URIUtils::AddSlashAtEnd(strPath);
  }

  if (CGUIDialogFileBrowser::ShowAndGetFile(shares, strMask, g_localizeStrings.Get(293), strPath, false, true)) // "subtitles"
  {
    if (URIUtils::HasExtension(strPath, ".sub"))
    {
      if (XFILE::CFile::Exists(URIUtils::ReplaceExtension(strPath, ".idx")))
        strPath = URIUtils::ReplaceExtension(strPath, ".idx");
    }

    return strPath;
  }

  return "";
}

void CGUIDialogSubtitleSettings::OnSettingChanged(std::shared_ptr<const CSetting> setting)
{
  if (setting == NULL)
    return;

  CGUIDialogSettingsManualBase::OnSettingChanged(setting);
  
  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  const std::string &settingId = setting->GetId();
  if (settingId == SETTING_SUBTITLE_ENABLE)
  {
    m_subtitleVisible = videoSettings.m_SubtitleOn = std::static_pointer_cast<const CSettingBool>(setting)->GetValue();
    g_application.m_pPlayer->SetSubtitleVisible(videoSettings.m_SubtitleOn);
  }
  else if (settingId == SETTING_SUBTITLE_DELAY)
  {
    videoSettings.m_SubtitleDelay = static_cast<float>(std::static_pointer_cast<const CSettingNumber>(setting)->GetValue());
    g_application.m_pPlayer->SetSubTitleDelay(videoSettings.m_SubtitleDelay);
  }
  else if (settingId == SETTING_SUBTITLE_STREAM)
  {
    m_subtitleStream = videoSettings.m_SubtitleStream = std::static_pointer_cast<const CSettingInt>(setting)->GetValue();
    g_application.m_pPlayer->SetSubtitle(m_subtitleStream);
  }
}

void CGUIDialogSubtitleSettings::OnSettingAction(std::shared_ptr<const CSetting> setting)
{
  if (setting == NULL)
    return;

  CGUIDialogSettingsManualBase::OnSettingAction(setting);
  
  const std::string &settingId = setting->GetId();
  if (settingId == SETTING_SUBTITLE_BROWSER)
  {
    std::string strPath;
    if (URIUtils::IsInRAR(g_application.CurrentFileItem().GetPath()) || URIUtils::IsInZIP(g_application.CurrentFileItem().GetPath()))
      strPath = CURL(g_application.CurrentFileItem().GetPath()).GetHostName();
    else
      strPath = g_application.CurrentFileItem().GetPath();

    std::string strMask = ".utf|.utf8|.utf-8|.sub|.srt|.smi|.rt|.txt|.ssa|.aqt|.jss|.ass|.idx|.rar|.zip";
    if (g_application.GetCurrentPlayer() == "VideoPlayer")
      strMask = ".srt|.rar|.zip|.ifo|.smi|.sub|.idx|.ass|.ssa|.txt";
    VECSOURCES shares(*CMediaSourceSettings::GetInstance().GetSources("video"));
    if (CMediaSettings::GetInstance().GetAdditionalSubtitleDirectoryChecked() != -1 && !CServiceBroker::GetSettings().GetString(CSettings::SETTING_SUBTITLES_CUSTOMPATH).empty())
    {
      CMediaSource share;
      std::vector<std::string> paths;
      paths.push_back(URIUtils::GetDirectory(strPath));
      paths.push_back(CServiceBroker::GetSettings().GetString(CSettings::SETTING_SUBTITLES_CUSTOMPATH));
      share.FromNameAndPaths("video",g_localizeStrings.Get(21367),paths);
      shares.push_back(share);
      strPath = share.strPath;
      URIUtils::AddSlashAtEnd(strPath);
    }
    if (CGUIDialogFileBrowser::ShowAndGetFile(shares, strMask, g_localizeStrings.Get(293), strPath, false, true)) // "subtitles"
    {
      if (URIUtils::HasExtension(strPath, ".sub"))
      {
        if (XFILE::CFile::Exists(URIUtils::ReplaceExtension(strPath, ".idx")))
          strPath = URIUtils::ReplaceExtension(strPath, ".idx");
      }
      
      g_application.m_pPlayer->AddSubtitle(strPath);
      Close();
    }
  }
  else if (settingId == SETTING_SUBTITLE_MAKE_DEFAULT)
    Save();
}

void CGUIDialogSubtitleSettings::Save()
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

  CMediaSettings::GetInstance().GetDefaultVideoSettings() = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  CMediaSettings::GetInstance().GetDefaultVideoSettings().m_SubtitleStream = -1;
  CServiceBroker::GetSettings().Save();
}

void CGUIDialogSubtitleSettings::SetupView()
{
  CGUIDialogSettingsManualBase::SetupView();

  SetHeading(13396);
  SET_CONTROL_HIDDEN(CONTROL_SETTINGS_OKAY_BUTTON);
  SET_CONTROL_HIDDEN(CONTROL_SETTINGS_CUSTOM_BUTTON);
  SET_CONTROL_LABEL(CONTROL_SETTINGS_CANCEL_BUTTON, 15067);
}

void CGUIDialogSubtitleSettings::InitializeSettings()
{
  CGUIDialogSettingsManualBase::InitializeSettings();

  std::shared_ptr<CSettingCategory> category = AddCategory("subtitlesettings", -1);
  if (category == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogSubtitleSettings: unable to setup settings");
    return;
  }

  // get all necessary setting groups
  std::shared_ptr<CSettingGroup> groupSubtitles = AddGroup(category);
  if (groupSubtitles == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogSubtitleSettings: unable to setup settings");
    return;
  }
  std::shared_ptr<CSettingGroup> groupSaveAsDefault = AddGroup(category);
  if (groupSaveAsDefault == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogSubtitleSettings: unable to setup settings");
    return;
  }

  bool usePopup = g_SkinInfo->HasSkinFile("DialogSlider.xml");

  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  
  if (g_application.m_pPlayer->HasPlayer())
  {
    g_application.m_pPlayer->GetSubtitleCapabilities(m_subCaps);
  }

  // subitlte settings
  m_subtitleVisible = g_application.m_pPlayer->GetSubtitleVisible();
  // subtitle enabled setting
  AddToggle(groupSubtitles, SETTING_SUBTITLE_ENABLE, 13397, SettingLevel::Basic, m_subtitleVisible);

  // subtitle delay setting
  if (SupportsSubtitleFeature(IPC_SUBS_OFFSET))
  {
    std::shared_ptr<CSettingNumber> settingSubtitleDelay = AddSlider(groupSubtitles, SETTING_SUBTITLE_DELAY, 22006, SettingLevel::Basic, videoSettings.m_SubtitleDelay, 0, -g_advancedSettings.m_videoSubsDelayRange, 0.1f, g_advancedSettings.m_videoSubsDelayRange, 22006, usePopup);
    std::static_pointer_cast<CSettingControlSlider>(settingSubtitleDelay->GetControl())->SetFormatter(SettingFormatterDelay);
  }

  // subtitle stream setting
  if (SupportsSubtitleFeature(IPC_SUBS_SELECT))
    AddSubtitleStreams(groupSubtitles, SETTING_SUBTITLE_STREAM);

  // subtitle browser setting
  if (SupportsSubtitleFeature(IPC_SUBS_EXTERNAL))
    AddButton(groupSubtitles, SETTING_SUBTITLE_BROWSER, 13250, SettingLevel::Basic);

  // subtitle stream setting
  AddButton(groupSaveAsDefault, SETTING_SUBTITLE_MAKE_DEFAULT, 12376, SettingLevel::Basic);
}

bool CGUIDialogSubtitleSettings::SupportsSubtitleFeature(int feature)
{
  for (Features::iterator itr = m_subCaps.begin(); itr != m_subCaps.end(); ++itr)
  {
    if (*itr == feature || *itr == IPC_SUBS_ALL)
      return true;
  }

  return false;
}

void CGUIDialogSubtitleSettings::AddSubtitleStreams(std::shared_ptr<CSettingGroup> group, const std::string &settingId)
{
  if (group == NULL || settingId.empty())
    return;

  m_subtitleStream = g_application.m_pPlayer->GetSubtitle();
  if (m_subtitleStream < 0)
    m_subtitleStream = 0;

  AddList(group, settingId, 462, SettingLevel::Basic, m_subtitleStream, SubtitleStreamsOptionFiller, 462);
}

bool CGUIDialogSubtitleSettings::IsPlayingPassthrough(const std::string &condition, const std::string &value, const CSetting *setting, void *data)
{
  return g_application.m_pPlayer->IsPassthrough();
}

void CGUIDialogSubtitleSettings::SubtitleStreamsOptionFiller(std::shared_ptr<const CSetting> setting, std::vector< std::pair<std::string, int> > &list, int &current, void *data)
{
  int subtitleStreamCount = g_application.m_pPlayer->GetSubtitleCount();

  // cycle through each subtitle and add it to our entry list
  for (int i = 0; i < subtitleStreamCount; ++i)
  {
    SPlayerSubtitleStreamInfo info;
    g_application.m_pPlayer->GetSubtitleStreamInfo(i, info);

    std::string strItem;
    std::string strLanguage;

    if (!g_LangCodeExpander.Lookup(info.language, strLanguage))
      strLanguage = g_localizeStrings.Get(13205); // Unknown

    if (info.name.length() == 0)
      strItem = strLanguage;
    else
      strItem = StringUtils::Format("%s - %s", strLanguage.c_str(), info.name.c_str());

    strItem += StringUtils::Format(" (%i/%i)", i + 1, subtitleStreamCount);

    list.push_back(make_pair(strItem, i));
  }

  // no subtitle streams - just add a "None" entry
  if (list.empty())
  {
    list.push_back(make_pair(g_localizeStrings.Get(231), -1));
    current = -1;
  }
}

std::string CGUIDialogSubtitleSettings::SettingFormatterDelay(std::shared_ptr<const CSettingControlSlider> control, const CVariant &value, const CVariant &minimum, const CVariant &step, const CVariant &maximum)
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
