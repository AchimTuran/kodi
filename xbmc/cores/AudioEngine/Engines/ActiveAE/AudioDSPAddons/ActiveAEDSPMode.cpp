/*
 *      Copyright (C) 2010-2014 Team KODI
 *      http://kodi.tv
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
 *  along with KODI; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "ActiveAEDSPMode.h"
#include "ActiveAEDSPDatabase.h"
#include "ActiveAEDSP.h"
#include "guilib/LocalizeStrings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

using namespace ActiveAE;

bool CActiveAEDSPMode::operator==(const CActiveAEDSPMode &right) const
{
  return (m_uiModeId          == right.m_uiModeId &&
          m_iAddonId          == right.m_iAddonId &&
          m_iAddonModeNumber  == right.m_iAddonModeNumber &&
          m_iModePosition     == right.m_iModePosition);
}

bool CActiveAEDSPMode::operator!=(const CActiveAEDSPMode &right) const
{
  return !(*this == right);
}

CActiveAEDSPMode::CActiveAEDSPMode()
{
  m_uiModeId                = AE_DSP_INVALID_ADDON_ID;
  m_iModePosition           = -1;
  m_bIsEnabled              = false;
  m_strOwnIconPath          = "";
  m_strOverrideIconPath     = "";
  m_iStreamTypeFlags        = 0;
  m_iBaseType               = AUDIODSP_ADDON_ABASE_UNKNOWN;
  m_iModeName               = -1;
  m_iModeSetupName          = -1;
  m_iModeDescription        = -1;
  m_iModeHelp               = -1;
  m_bChanged                = false;
  m_bIsInternal             = false;
  m_bHasSettingsDialog      = false;

  m_fCPUUsage               = 0.0f;

  m_iAddonId                = -1;
  m_iAddonModeNumber        = -1;
  m_strModeName             = "";
}

CActiveAEDSPMode::CActiveAEDSPMode(unsigned int modeId, const AUDIODSP_ADDON_BASETYPE baseType)
{
  m_uiModeId                = modeId;
  m_iModePosition           = 0;
  m_bIsEnabled              = true;
  m_strOwnIconPath          = "";
  m_strOverrideIconPath     = "";
  m_iStreamTypeFlags        = 0x0;/*AE_DSP_PRSNT_ASTREAM_BASIC |
                              AE_DSP_PRSNT_ASTREAM_MUSIC |
                              AE_DSP_PRSNT_ASTREAM_MOVIE*/
                              /* |
                              AE_DSP_PRSNT_ASTREAM_GAME |
                              AE_DSP_PRSNT_ASTREAM_APP |
                              AE_DSP_PRSNT_ASTREAM_PHONE |
                              AE_DSP_PRSNT_ASTREAM_MESSAGE*/;
  m_iBaseType               = baseType;

  switch (modeId)
  {
    case AE_DSP_MASTER_MODE_ID_PASSOVER:
      m_iModeName           = 16039;
      m_iModeDescription    = -1;
      m_iModeHelp           = -1;
      m_iModeSetupName      = -1;
      m_strModeName         = "Passover";
      break;
    case AE_DSP_MASTER_MODE_ID_INTERNAL_STEREO_UPMIX:
      m_iModeName           = 252;
      m_iModeDescription    = 36364;
      m_iModeHelp           = -1;
      m_iModeSetupName      = -1;
      m_strModeName         = "Stereo Upmix";
      break;
    default:
      m_iModeName           = 16039;
      m_iModeDescription    = -1;
      m_iModeHelp           = -1;
      m_iModeSetupName      = -1;
      m_strModeName         = "Unknown";
      break;
  };

  m_bChanged                = false;
  m_bIsInternal             = true;
  m_bHasSettingsDialog      = false;

  m_fCPUUsage               = 0.0f;

  m_iAddonId                = -1;
  m_iAddonModeNumber        = -1;
}

CActiveAEDSPMode::CActiveAEDSPMode(const AUDIODSP_ADDON_MODE_DATA &mode, unsigned int iAddonId)
{
  m_iModePosition           = -1;
  m_uiModeId                = mode.uiUniqueDBModeId;
  m_iAddonId                = iAddonId;
  m_iBaseType               = AUDIODSP_ADDON_ABASE_UNKNOWN;
  m_strOwnIconPath          = mode.strOwnModeImage;
  m_strOverrideIconPath     = mode.strOverrideModeImage;
  m_iModeName               = mode.iModeName;
  m_iModeSetupName          = mode.iModeSetupName;
  m_iModeDescription        = mode.iModeDescription;
  m_iModeHelp               = mode.iModeHelp;
  m_iAddonModeNumber        = mode.uiModeNumber;
  m_strModeName             = mode.strModeName;
  m_bHasSettingsDialog      = mode.bHasSettingsDialog;
  m_bChanged                = false;
  m_bIsInternal             = false;

  m_fCPUUsage               = 0.0f;

  if (m_strModeName.empty())
    m_strModeName = StringUtils::Format("%s %d", g_localizeStrings.Get(15023).c_str(), m_uiModeId);
}

CActiveAEDSPMode::CActiveAEDSPMode(const CActiveAEDSPMode &mode)
{
  *this = mode;
}

CActiveAEDSPMode &CActiveAEDSPMode::operator=(const CActiveAEDSPMode &mode)
{
  m_uiModeId                = mode.m_uiModeId;
  m_iModePosition           = mode.m_iModePosition;
  m_bIsEnabled              = mode.m_bIsEnabled;
  m_strOwnIconPath          = mode.m_strOwnIconPath;
  m_strOverrideIconPath     = mode.m_strOverrideIconPath;
  m_iStreamTypeFlags        = mode.m_iStreamTypeFlags;
  m_iBaseType               = mode.m_iBaseType;
  m_iModeName               = mode.m_iModeName;
  m_iModeSetupName          = mode.m_iModeSetupName;
  m_iModeDescription        = mode.m_iModeDescription;
  m_iModeHelp               = mode.m_iModeHelp;
  m_iAddonId                = mode.m_iAddonId;
  m_iAddonModeNumber        = mode.m_iAddonModeNumber;
  m_strModeName             = mode.m_strModeName;
  m_bChanged                = mode.m_bChanged;
  m_bIsInternal             = mode.m_bIsInternal;
  m_bHasSettingsDialog      = mode.m_bHasSettingsDialog;
  m_fCPUUsage               = mode.m_fCPUUsage;

  return *this;
}

/********** General mode related functions **********/

bool CActiveAEDSPMode::IsNew(void) const
{
  CSingleLock lock(m_critSection);
  return m_uiModeId == AE_DSP_INVALID_ADDON_ID;
}

bool CActiveAEDSPMode::IsChanged(void) const
{
  CSingleLock lock(m_critSection);
  return m_bChanged;
}


bool CActiveAEDSPMode::IsInternal(void) const
{
  CSingleLock lock(m_critSection);
  return m_bIsInternal;
}

bool CActiveAEDSPMode::IsEnabled(void) const
{
  CSingleLock lock(m_critSection);
  return m_bIsEnabled;
}

bool CActiveAEDSPMode::SetEnabled(bool bIsEnabled)
{
  CSingleLock lock(m_critSection);

  if (m_bIsEnabled != bIsEnabled)
  {
    /* update the Enabled flag */
    m_bIsEnabled = bIsEnabled;
    SetChanged();
    m_bChanged = true;

    return true;
  }

  return false;
}

int CActiveAEDSPMode::ModePosition(void) const
{
  CSingleLock lock(m_critSection);
  return m_iModePosition;
}

bool CActiveAEDSPMode::SetModePosition(int iModePosition)
{
  CSingleLock lock(m_critSection);
  if (m_iModePosition != iModePosition)
  {
    /* update the type */
    m_iModePosition = iModePosition;
    SetChanged();
    m_bChanged = true;

    return true;
  }

  return false;
}

bool CActiveAEDSPMode::SupportStreamType(AUDIODSP_ADDON_STREAMTYPE streamType, unsigned int flags)
{
  //! @todo AudioDSP V2 readd present stream flags
  //     if (streamType == AUDIODSP_ADDON_ASTREAM_BASIC   && (flags & AUDIODSP_ADDON_PRSNT_ASTREAM_BASIC))   return true;
  //else if (streamType == AUDIODSP_ADDON_ASTREAM_MUSIC   && (flags & AUDIODSP_ADDON_PRSNT_ASTREAM_MUSIC))   return true;
  //else if (streamType == AUDIODSP_ADDON_ASTREAM_MOVIE   && (flags & AUDIODSP_ADDON_PRSNT_ASTREAM_MOVIE))   return true;
  //else if (streamType == AUDIODSP_ADDON_ASTREAM_GAME    && (flags & AUDIODSP_ADDON_PRSNT_ASTREAM_GAME))    return true;
  //else if (streamType == AUDIODSP_ADDON_ASTREAM_APP     && (flags & AUDIODSP_ADDON_PRSNT_ASTREAM_APP))     return true;
  //else if (streamType == AUDIODSP_ADDON_ASTREAM_PHONE   && (flags & AUDIODSP_ADDON_PRSNT_ASTREAM_PHONE))   return true;
  //else if (streamType == AUDIODSP_ADDON_ASTREAM_MESSAGE && (flags & AUDIODSP_ADDON_PRSNT_ASTREAM_MESSAGE)) return true;
  return false;
}

bool CActiveAEDSPMode::SupportStreamType(AUDIODSP_ADDON_STREAMTYPE streamType) const
{
  return SupportStreamType(streamType, m_iStreamTypeFlags);
}

/********** Mode user interface related data functions **********/

int CActiveAEDSPMode::ModeName(void) const
{
  CSingleLock lock(m_critSection);
  return m_iModeName;
}

int CActiveAEDSPMode::ModeSetupName(void) const
{
  CSingleLock lock(m_critSection);
  return m_iModeSetupName;
}

int CActiveAEDSPMode::ModeDescription(void) const
{
  CSingleLock lock(m_critSection);
  return m_iModeDescription;
}

int CActiveAEDSPMode::ModeHelp(void) const
{
  CSingleLock lock(m_critSection);
  return m_iModeHelp;
}

const std::string &CActiveAEDSPMode::IconOwnModePath(void) const
{
  CSingleLock lock(m_critSection);
  return m_strOwnIconPath;
}

const std::string &CActiveAEDSPMode::IconOverrideModePath(void) const
{
  CSingleLock lock(m_critSection);
  return m_strOverrideIconPath;
}


/********** Master mode type related functions **********/

bool CActiveAEDSPMode::SetBaseType(AUDIODSP_ADDON_BASETYPE baseType)
{
  CSingleLock lock(m_critSection);
  if (m_iBaseType != baseType)
  {
    /* update the mode base */
    m_iBaseType = baseType;
    SetChanged();
    m_bChanged = true;

    return true;
  }

  return false;
}

AUDIODSP_ADDON_BASETYPE CActiveAEDSPMode::BaseType(void) const
{
  CSingleLock lock(m_critSection);
  return m_iBaseType;
}


/********** Audio DSP database related functions **********/

unsigned int CActiveAEDSPMode::ModeID(void) const
{
  CSingleLock lock(m_critSection);
  return m_uiModeId;
}

unsigned int CActiveAEDSPMode::AddUpdate(bool force)
{
  if (!force)
  {
    // not changed
    CSingleLock lock(m_critSection);
    if (!m_bChanged && m_uiModeId > AE_DSP_MASTER_MODE_ID_INVALID)
      return m_uiModeId;
  }

  CActiveAEDSPDatabase *database = nullptr;
  if (!database || !database->IsOpen())
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - failed to open the database");
    return AE_DSP_MASTER_MODE_ID_INVALID;
  }

  database->AddUpdateMode(*this);
  m_uiModeId = database->GetModeId(*this);

  return m_uiModeId;
}

bool CActiveAEDSPMode::Delete(void)
{
  CActiveAEDSPDatabase *database = nullptr;
  if (!database || !database->IsOpen())
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - failed to open the database");
    return false;
  }

  return database->DeleteMode(*this);
}

bool CActiveAEDSPMode::IsKnown(void) const
{
  CActiveAEDSPDatabase *database = nullptr;
  if (!database || !database->IsOpen())
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - failed to open the database");
    return false;
  }

  return database->GetModeId(*this) > 0;
}


/********** Dynamic processing related data methods **********/

void CActiveAEDSPMode::SetCPUUsage(float percent)
{
  CSingleLock lock(m_critSection);
  m_fCPUUsage = percent;
}

float CActiveAEDSPMode::CPUUsage(void) const
{
  CSingleLock lock(m_critSection);
  return m_fCPUUsage;
}


/********** Fixed addon related Mode methods **********/

int CActiveAEDSPMode::AddonID(void) const
{
  CSingleLock lock(m_critSection);
  return m_iAddonId;
}

unsigned int CActiveAEDSPMode::AddonModeNumber(void) const
{
  CSingleLock lock(m_critSection);
  return m_iAddonModeNumber;
}

const std::string &CActiveAEDSPMode::AddonModeName(void) const
{
  CSingleLock lock(m_critSection);
  return m_strModeName;
}

bool CActiveAEDSPMode::HasSettingsDialog(void) const
{
  CSingleLock lock(m_critSection);
  return m_bHasSettingsDialog;
}

unsigned int CActiveAEDSPMode::StreamTypeFlags(void) const
{
  CSingleLock lock(m_critSection);
  return m_iStreamTypeFlags;
}
