/*
 *      Copyright (C) 2010-2015 Team Kodi
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <vector>
#include "Application.h"
#include "ActiveAEDSPAddon.h"
#include "ActiveAEDSP.h"
#include "filesystem/SpecialProtocol.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

using namespace ADDON;
using namespace ActiveAE;
using namespace DSP;
using namespace DSP::AUDIO;

#define DEFAULT_INFO_STRING_VALUE "unknown"

unsigned int CActiveAEDSPAddon::m_uniqueModeID = 0; //! @todo AudioDSP V2 make this thread safe or remove this if not needed

CActiveAEDSPAddon::CActiveAEDSPAddon(BinaryAddonBasePtr addonInfo)
  : IAddonInstanceHandler(ADDON_INSTANCE_ADSP, addonInfo)
{
  ResetProperties(AE_DSP_INVALID_ADDON_ID);
}

CActiveAEDSPAddon::~CActiveAEDSPAddon(void)
{
  Destroy();
}

void CActiveAEDSPAddon::ResetProperties(unsigned int uiClientId)
{
  /* initialise members */
  m_menuhooks.clear();
  m_strUserPath           = CSpecialProtocol::TranslatePath(Profile());
  m_strAddonPath          = CSpecialProtocol::TranslatePath(Path());
  m_bReadyToUse           = false;
  m_uiClientId            = uiClientId;
  m_strAudioDSPVersion    = DEFAULT_INFO_STRING_VALUE;
  m_strFriendlyName       = DEFAULT_INFO_STRING_VALUE;
  m_strAudioDSPName       = DEFAULT_INFO_STRING_VALUE;

  m_struct = {{0}};
  m_struct.props.strUserPath = m_strUserPath.c_str();
  m_struct.props.strAddonPath = m_strAddonPath.c_str();

  m_struct.toKodi.kodiInstance = this;
  m_struct.toKodi.add_menu_hook = cb_add_menu_hook;
  m_struct.toKodi.remove_menu_hook = cb_remove_menu_hook;
  m_struct.toKodi.register_mode = cb_register_mode;
  m_struct.toKodi.unregister_mode = cb_unregister_mode;
}

bool CActiveAEDSPAddon::Create(unsigned int iClientId)
{
  if (iClientId == AE_DSP_INVALID_ADDON_ID)
    return false;

  /* ensure that a previous instance is destroyed */
  Destroy();

  /* reset all properties to defaults */
  ResetProperties(iClientId);

  /* initialise the add-on */
  CLog::Log(LOGDEBUG, "ActiveAE DSP - %s - creating audio dsp add-on instance '%s'", __FUNCTION__, Name().c_str());
  /* Open the class "kodi::addon::CInstanceAudioDSP" on add-on side */
  m_bReadyToUse = (CreateInstance(&m_struct) == ADDON_STATUS_OK);
  if (!m_bReadyToUse)
  {
    CLog::Log(LOGFATAL, "ActiveAE DSP: failed to create instance for '%s' and not usable!", ID().c_str());
    return false;
  }

  GetAddonProperties();

  return true;
}

void CActiveAEDSPAddon::Destroy(void)
{
  /* reset 'ready to use' to false */
  if (!m_bReadyToUse)
    return;
  m_bReadyToUse = false;

  CLog::Log(LOGDEBUG, "ActiveAE DSP - %s - destroying audio dsp add-on '%s'", __FUNCTION__, GetFriendlyName().c_str());

  /* Destroy the class "kodi::addon::CInstanceAudioDSP" on add-on side */
  DestroyInstance();

  /* reset all properties to defaults */
  ResetProperties(AE_DSP_INVALID_ADDON_ID);
}

bool CActiveAEDSPAddon::ReCreate(void)
{
  unsigned int uiClientID = m_uiClientId;
  Destroy();

  /* recreate the instance */
  return Create(uiClientID);
}

bool CActiveAEDSPAddon::ReadyToUse(void) const
{
  return m_bReadyToUse;
}

unsigned int CActiveAEDSPAddon::GetAudioDSPID(void) const
{
  return m_uiClientId;
}

void CActiveAEDSPAddon::GetAddonProperties(void)
{
  /* get the name of the dsp addon */
  std::string strDSPName = m_struct.toAddon.get_dsp_name(&m_struct);

  /* display name = backend name string */
  std::string strFriendlyName = StringUtils::Format("%s", strDSPName.c_str());

  /* backend version number */
  std::string strAudioDSPVersion = m_struct.toAddon.get_dsp_version(&m_struct);

  /* update the members */
  m_strAudioDSPName     = strDSPName;
  m_strFriendlyName     = strFriendlyName;
  m_strAudioDSPVersion  = strAudioDSPVersion;
}

const std::string &CActiveAEDSPAddon::GetAudioDSPName(void) const
{
  return m_strAudioDSPName;
}

const std::string &CActiveAEDSPAddon::GetAudioDSPVersion(void) const
{
  return m_strAudioDSPVersion;
}

const std::string &CActiveAEDSPAddon::GetFriendlyName(void) const
{
  return m_strFriendlyName;
}

bool CActiveAEDSPAddon::HaveMenuHooks(AUDIODSP_MENU_HOOK_CAT cat) const
{
  if (m_bReadyToUse && !m_menuhooks.empty())
  {
    for (unsigned int i = 0; i < m_menuhooks.size(); ++i)
    {
      if (m_menuhooks[i].category == cat)
        return true;
    }
  }
  return false;
}

//! @todo AudioDSP V2 make this thread safe
AE_DSP_MENUHOOKS *CActiveAEDSPAddon::GetMenuHooks(void)
{
  return &m_menuhooks;
}

void CActiveAEDSPAddon::GetAudioDSPModes(CAudioDSPModeVector_t &Modes)
{
  Modes.clear();

  CSingleLock lock(m_critSection);

  for (unsigned int ii = 0; ii < m_registeredModes.size(); ii++)
  {
    Modes.push_back(m_registeredModes[ii]);
  }
}

void CActiveAEDSPAddon::CallMenuHook(const AUDIODSP_MENU_HOOK &hook)
{
  if (!m_bReadyToUse)
    return;

  m_struct.toAddon.menu_hook(&m_struct, &hook);
}

int CActiveAEDSPAddon::CreateModeInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat, ADDON_HANDLE ModeHandle)
{
  return m_struct.toAddon.create_mode(&m_struct, ModeHandle);
}

int CActiveAEDSPAddon::ProcessModeInstance(const ADDON_HANDLE ModeHandle, const uint8_t **In, uint8_t **Out)
{
  return m_struct.toAddon.process_mode(&m_struct, ModeHandle, In, Out);
}

int CActiveAEDSPAddon::DestroyModeInstance(const ADDON_HANDLE ModeHandle)
{
  return m_struct.toAddon.destroy_mode(&m_struct, ModeHandle);
}

//AUDIODSP_ADDON_ERROR CActiveAEDSPAddon::StreamCreate(const AUDIODSP_ADDON_SETTINGS *addonSettings, const AUDIODSP_ADDON_STREAM_PROPERTIES* pProperties, ADDON_HANDLE handle)
//{
//  AUDIODSP_ADDON_ERROR retVal = m_struct.toAddon.stream_create(&m_struct, addonSettings, pProperties, handle);
//  
//  LogError(retVal, __FUNCTION__);
//
//  return retVal;
//}
//
//void CActiveAEDSPAddon::StreamDestroy(const ADDON_HANDLE handle)
//{
//  m_struct.toAddon.stream_destroy(&m_struct, handle);
//}

const char *CActiveAEDSPAddon::ErrorCodeToString(const AUDIODSP_ADDON_ERROR error)
{
  switch (error)
  {
  case AUDIODSP_ADDON_ERROR_NO_ERROR:
    return "no error";
  case AUDIODSP_ADDON_ERROR_NOT_IMPLEMENTED:
    return "not implemented";
  case AUDIODSP_ADDON_ERROR_REJECTED:
    return "rejected by the backend";
  case AUDIODSP_ADDON_ERROR_INVALID_PARAMETERS:
    return "invalid parameters for this method";
  case AUDIODSP_ADDON_ERROR_INVALID_SAMPLERATE:
    return "invalid samplerate for this method";
  case AUDIODSP_ADDON_ERROR_INVALID_IN_CHANNELS:
    return "invalid input channel layout for this method";
  case AUDIODSP_ADDON_ERROR_INVALID_OUT_CHANNELS:
    return "invalid output channel layout for this method";
  case AUDIODSP_ADDON_ERROR_FAILED:
    return "the command failed";
  case AUDIODSP_ADDON_ERROR_UNKNOWN:
  default:
    return "unknown error";
  }
}

unsigned int CActiveAEDSPAddon::RegisterMode(const CAudioDSPMode &Mode)
{
  CSingleLock lock(m_critSection);

  std::vector<CAudioDSPMode>::iterator iter = m_registeredModes.end();
  unsigned int id = AE_DSP_INVALID_ADDON_ID; //! @todo AudioDSP V2 replace this with a new define which is not specific to ADDON_ID
  for (unsigned int ii = 0; ii < m_registeredModes.size(); ii++)
  {
    if (m_registeredModes[ii].strModeName == Mode.strModeName && 
        m_registeredModes[ii].iModeNumber == Mode.iModeNumber)
    {
      iter = m_registeredModes.begin() + ii;
      break;
    }
  }

  if (iter != m_registeredModes.end())
  {
    id = iter->uiUniqueDBModeId;
    DeregisterMode(*iter);
    //! @todo AudioDSP V2 assign new ModeID
  }
  else
  {
    m_uniqueModeID++;
  }
  
  m_registeredModes.push_back(Mode);
  m_registeredModes.back().uiUniqueDBModeId = id != AE_DSP_INVALID_ADDON_ID ? id : m_uniqueModeID; //! @todo AudioDSP V2 is uiUniqueDBModeId still required?

  return m_registeredModes.back().uiUniqueDBModeId;
}

void CActiveAEDSPAddon::DeregisterMode(const CAudioDSPMode &Mode)
{
  CSingleLock lock(m_critSection);

  std::vector<CAudioDSPMode>::iterator iter = m_registeredModes.end();
  for (unsigned int ii = 0; ii < m_registeredModes.size(); ii++)
  {
    if (m_registeredModes[ii].strModeName == Mode.strModeName &&
      m_registeredModes[ii].iModeNumber == Mode.iModeNumber)
    {
      iter = m_registeredModes.begin() + ii;
    }
  }

  if (iter != m_registeredModes.end())
  {
    m_registeredModes.erase(iter);
  }
}

bool CActiveAEDSPAddon::LogError(const AUDIODSP_ADDON_ERROR error, const char *strMethod) const
{
  if (error != AUDIODSP_ADDON_ERROR_NO_ERROR && error != AUDIODSP_ADDON_ERROR_IGNORE_ME)
  {
    CLog::Log(LOGERROR, "ActiveAE DSP - %s - addon '%s' returned an error: %s",
        strMethod, GetFriendlyName().c_str(), ErrorCodeToString(error));
    return false;
  }
  return true;
}

void CActiveAEDSPAddon::cb_add_menu_hook(void *kodiInstance, AUDIODSP_MENU_HOOK *hook)
{
  CActiveAEDSPAddon *addon = static_cast<CActiveAEDSPAddon*>(kodiInstance);
  if (!hook || !addon)
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - invalid handler data", __FUNCTION__);
    return;
  }

  AE_DSP_MENUHOOKS *hooks = addon->GetMenuHooks();
  if (hooks)
  {
    AUDIODSP_MENU_HOOK hookInt;
    hookInt.iHookId            = hook->iHookId;
    hookInt.iLocalizedStringId = hook->iLocalizedStringId;
    hookInt.category           = hook->category;
    hookInt.iRelevantModeId    = hook->iRelevantModeId;

    /* add this new hook */
    hooks->push_back(hookInt);
  }
}

void CActiveAEDSPAddon::cb_remove_menu_hook(void *kodiInstance, AUDIODSP_MENU_HOOK *hook)
{
  CActiveAEDSPAddon *addon = static_cast<CActiveAEDSPAddon*>(kodiInstance);
  if (!hook || !addon)
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - invalid handler data", __FUNCTION__);
    return;
  }

  AE_DSP_MENUHOOKS *hooks = addon->GetMenuHooks();
  if (hooks)
  {
    for (unsigned int i = 0; i < hooks->size(); i++)
    {
      if (hooks->at(i).iHookId == hook->iHookId)
      {
        /* remove this hook */
        hooks->erase(hooks->begin()+i);
        break;
      }
    }
  }
}

void CActiveAEDSPAddon::cb_register_mode(void* kodiInstance, AUDIODSP_ADDON_MODE_DATA* mode)
{
  CActiveAEDSPAddon *addon = static_cast<CActiveAEDSPAddon*>(kodiInstance);
  if (!mode || !addon)
  {
    CLog::Log(LOGERROR, "AudioDSP - %s - Fatal error invalid mode data or kodiInstance pointer!", __FUNCTION__);
    return;
  }

  CActiveAEDSPAddon::CAudioDSPMode kodiMode = mode;
  mode->uiUniqueDBModeId = addon->RegisterMode(kodiMode);

  if (mode->uiUniqueDBModeId > AE_DSP_INVALID_ADDON_ID)
  {
    CLog::Log(LOGDEBUG, "Audio DSP - %s - successfully registered mode %s of %s adsp-addon", __FUNCTION__, mode->strModeName, addon->Name().c_str());
  }
  else
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - failed to register mode %s of %s adsp-addon", __FUNCTION__, mode->strModeName, addon->Name().c_str());
  }
}

void CActiveAEDSPAddon::cb_unregister_mode(void* kodiInstance, AUDIODSP_ADDON_MODE_DATA* mode)
{
  CActiveAEDSPAddon *addon = static_cast<CActiveAEDSPAddon*>(kodiInstance);
  if (!mode || !addon)
  {
    CLog::Log(LOGERROR, "Audio DSP - %s - invalid mode data", __FUNCTION__);
    return;
  }

  addon->DeregisterMode(mode);
}

IADSPNode* ActiveAE::CActiveAEDSPAddon::InstantiateNode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, const AEStreamProperties &StreamProperties, uint64_t ID)
{
  return nullptr;
}

DSPErrorCode_t ActiveAE::CActiveAEDSPAddon::DestroyNode(IADSPNode *&Node)
{
  return DSPErrorCode_t();
}
