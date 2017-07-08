/*
 *      Copyright (C) 2005-2017 Team Kodi
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

#include <sstream>

#include "utils/log.h"
#include "ServiceBroker.h"
#include "settings/Settings.h"
#include "guilib/GUIWindowManager.h"
#include "settings/dialogs/GUIDialogAudioDSPManager.h"
#include "addons/binary-addons/BinaryAddonBase.h"

#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/ActiveAudioDSP.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEStream.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPProcessingBuffer.h"

 // includes for AudioDSP add-on modes
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonNodeCreator.h"

using namespace Actor;
using namespace ADDON;
using namespace std;
using namespace DSP;
using namespace DSP::AUDIO;


namespace ActiveAE
{
CActiveAudioDSP::CActiveAudioDSP(CEvent *inMsgEvent) :
  CThread("ActiveAudioDSP"),
  m_AddonControlPort("AudioDSPAddonControlPort", inMsgEvent, &m_outMsgEvent),
  m_ControlPort("AudioDSPControlPort", inMsgEvent, &m_outMsgEvent),
  m_Controller(m_DSPChainModelObject)
{
  m_inMsgEvent = inMsgEvent;
}

CActiveAudioDSP::~CActiveAudioDSP()
{
  Stop();
}

void CActiveAudioDSP::Start()
{
  if (!IsRunning())
  {
    Create(); // create thread for this object
    SetPriority(THREAD_PRIORITY_NORMAL);
  }
  
  m_ControlPort.SendOutMessage(CAudioDSPControlProtocol::INIT);
}

void CActiveAudioDSP::Stop()
{
  Message *replyMsg;
  if (m_ControlPort.SendOutMessageSync(CAudioDSPControlProtocol::DEINIT, &replyMsg, 0))
  {
    if (replyMsg->signal != CAudioDSPControlProtocol::ACC)
    {
      CLog::Log(LOGERROR, "%s an error occured during shutting down AudioDSP", __FUNCTION__);
    }

    replyMsg->Release();
  }

  m_bStop = true;
  m_outMsgEvent.Set();
  StopThread();

  m_KodiModes.ReleaseAllModes(m_DSPChainModelObject);

  m_AddonControlPort.Purge();
  m_ControlPort.Purge();

  CServiceBroker::GetBinaryAddonManager().UnregisterCallback(ADDON_ADSPDLL);
}

void CActiveAudioDSP::EnableEvent(BinaryAddonBasePtr addon)
{
  //! @todo AudioDSP V2 implement messaging
}

void CActiveAudioDSP::DisableEvent(BinaryAddonBasePtr addon)
{
  //! @todo AudioDSP V2 implement messaging
}

void CActiveAudioDSP::InstalledEvent(BinaryAddonBasePtr addon)
{
  //! @todo AudioDSP V2 implement messaging
}

void CActiveAudioDSP::DeinstalledEvent(BinaryAddonBasePtr addon)
{
  //! @todo AudioDSP V2 implement messaging
}

void CActiveAudioDSP::EnableAddon(const string &Id, bool Enable)
{
  if (Enable)
  {
    m_AddonControlPort.SendOutMessage(CAudioDSPAddonControlProtocol::ENABLE_ADDON, (void*)Id.c_str(), sizeof(char)*Id.length() + 1);
  }
  else
  {
    m_AddonControlPort.SendOutMessage(CAudioDSPAddonControlProtocol::DISABLE_ADDON, (void*)Id.c_str(), sizeof(char)*Id.length() + 1);
  }
}

bool CActiveAudioDSP::GetAddon(const string &Id, AddonPtr &addon)
{
  return false;
}

void CActiveAudioDSP::RegisterAddon(const string &Id, bool restart, bool update)
{
}

void CActiveAudioDSP::UnregisterAddon(const string &Id)
{
}

void* CActiveAudioDSP::GetControllerHandle(void * ControllerCallback)
{
  return nullptr;
}

bool CActiveAudioDSP::ReleaseControllerHandle(void **Handle)
{
  return false;
}

IActiveAEProcessingBuffer* CActiveAudioDSP::GetProcessingBuffer(const CActiveAEStream *AudioStream, AEAudioFormat &OutputFormat)
{
  if (!AudioStream)
  {
    CLog::Log(LOGERROR, "%s - Invalid audio stream!", __FUNCTION__);
    return nullptr;
  }

  Actor::Message *replyMsg = nullptr;
  CAudioDSPControlProtocol::CCreateBuffer bufferMsg(AudioStream, OutputFormat);
  if (!m_ControlPort.SendOutMessageSync(CAudioDSPControlProtocol::GET_PROCESSING_BUFFER, &replyMsg, 30000, &bufferMsg, sizeof(CAudioDSPControlProtocol::CCreateBuffer)))
  {
    if (replyMsg)
    {
      replyMsg->Release();
    }
    return nullptr;
  }

  if (replyMsg->signal != CAudioDSPControlProtocol::SUCCESS)
  {
    replyMsg->Release();
    CLog::Log(LOGERROR, "%s an error occured during shutting down AudioDSP", __FUNCTION__);
    return nullptr;
  }

  IActiveAEProcessingBuffer *buffer = *reinterpret_cast<IActiveAEProcessingBuffer**>(replyMsg->data);
  replyMsg->Release();

  return buffer;
}

DSPErrorCode_t CActiveAudioDSP::ReleaseProcessingBuffer(int StreamID)
{
  if (!m_ControlPort.SendOutMessage(CAudioDSPControlProtocol::RELEASE_PROCESSING_BUFFER, &StreamID, sizeof(int)))
  {
    return DSP_ERR_FATAL_ERROR;
  }

  return DSP_ERR_NO_ERR;
}

enum ACTIVEAUDIODSP_STATES
{
  ADSP_TOP = 0,                             // 0
  ADSP_TOP_UNCONFIGURED,                    // 1
  ADSP_TOP_CONFIGURED,                      // 2
  ADSP_TOP_GET_PROCESSING_BUFFER,           // 3
  ADSP_TOP_RELEASE_PROCESSING_BUFFER,       // 4
};

int ADSP_parentStates[] = {
    -1,
    ADSP_TOP,                             //TOP_UNCONFIGURED
    ADSP_TOP,                             //TOP_CONFIGURED
    ADSP_TOP,                             //ADSP_TOP_GET_PROCESSING_BUFFER
    ADSP_TOP,                             //ADSP_TOP_RELEASE_PROCESSING_BUFFER
};

void CActiveAudioDSP::StateMachine(int signal, Protocol *port, Message *msg)
{
  for (int state = m_state; 1; state = ADSP_parentStates[state])
  {
    switch (state)
    {
      case ADSP_TOP: // TOP
        if (port == nullptr) // timeout
        {
          switch (signal)
          {
          case CAudioDSPControlProtocol::TIMEOUT:
            m_extTimeout = 1000;
            return;

          default:
            break;
          }
        }
        if (port == &m_ControlPort)
        {
          switch (signal)
          {
            case CAudioDSPControlProtocol::INIT:
              return;

            case CAudioDSPControlProtocol::GET_PROCESSING_BUFFER:
              return;

            case CAudioDSPControlProtocol::RELEASE_PROCESSING_BUFFER:
              return;

            case CAudioDSPControlProtocol::DEINIT:
              return;

            default:
            break;
          }
        }
        else if (port == &m_AddonControlPort)
        {
          switch (signal)
          {
            default:
            break;
          }
        }

        {
          string portName = port == nullptr ? "timer" : port->portName;
          CLog::Log(LOGWARNING, "%s - signal: %d form port: %s not handled for state: %d", __FUNCTION__, signal, portName.c_str(), m_state);
          m_hasError = true;
        }
        return;

      case ADSP_TOP_UNCONFIGURED:
        if (port == &m_ControlPort)
        {
          switch (signal)
          {
            case CAudioDSPControlProtocol::INIT:
            {
              //if (!m_databaseDSP.Open())
              //{
              //  msg->Reply(CAudioDSPControlProtocol::ERR);
              //  CLog::Log(LOGERROR, "%s during opening database an error occured!", __FUNCTION__);
              //  return;
              //}

              std::set<std::string> settingSet;
              settingSet.insert(CSettings::SETTING_AUDIOOUTPUT_DSPSETTINGS);
              CServiceBroker::GetSettings().RegisterCallback(this, settingSet);
              
              if (!CServiceBroker::GetBinaryAddonManager().RegisterCallback(ADDON_ADSPDLL, this))
              {
                msg->Reply(CAudioDSPControlProtocol::ERR);
                CLog::Log(LOGERROR, "%s during add-on manager callback registration an error occured!", __FUNCTION__);
                m_hasError = true;
                return;
              }

              //CLog::Log(LOGNOTICE, "ActiveAE DSP - starting");
              m_KodiModes.ReleaseAllModes(m_DSPChainModelObject);
              m_KodiModes.PrepareModes(m_DSPChainModelObject);
              PrepareAddons();
              PrepareAddonModes();

              m_state = ADSP_TOP_CONFIGURED;
              m_hasError = false;

              {
                CSingleLock lock(m_lock);
                m_isInitialized = true;
              }
            }
            break;

            default:
            break;
          }
        }
        else
        {
          m_hasError = true;
        }
      break;

      case ADSP_TOP_CONFIGURED:
        if (port == &m_ControlPort)
        {
          switch (signal)
          {
            case CAudioDSPControlProtocol::DEINIT:
              m_databaseDSP.Close();
              CServiceBroker::GetSettings().UnregisterCallback(this);
              {
                CSingleLock lock(m_lock);
                m_isInitialized = false;
              }
            break;

            case CAudioDSPControlProtocol::GET_PROCESSING_BUFFER:
              m_bStateMachineSelfTrigger = true;
              m_state = ADSP_TOP_GET_PROCESSING_BUFFER;
            break;
            
            case CAudioDSPControlProtocol::RELEASE_PROCESSING_BUFFER:
              m_bStateMachineSelfTrigger = true;
              m_state = ADSP_TOP_RELEASE_PROCESSING_BUFFER;
            break;

            default:
            break;
          }
        }
        else if (port == &m_AddonControlPort)
        {
          switch (signal)
          {
            case CAudioDSPAddonControlProtocol::ENABLE_ADDON:
            {
              string addonId = (char*)msg->data;
              AudioDSPAddonMap_t::iterator iter = m_DisabledAddons.find(addonId);
              if (iter == m_DisabledAddons.end())
              {
                CLog::Log(LOGERROR, "%s, Tried to enable the unknown addon \"%s\"", __FUNCTION__, addonId.c_str());
              }
              else
              {
                std::hash<std::string> hasher;
                unsigned int uiAddonId = hasher(iter->second->ID());
                if (!iter->second->Create(uiAddonId))
                {
                  iter->second->Destroy();
                  CLog::Log(LOGERROR, "%s - failed to create AudioDSP add-on %s", __FUNCTION__, addonId.c_str());
                }
                else
                {
                  m_EnabledAddons[addonId] = iter->second;
                  //! @todo AudioDSP V2 add addon to processing object
                  m_DisabledAddons.erase(addonId);
                }
              }
            }
            break;

            case CAudioDSPAddonControlProtocol::DISABLE_ADDON:
            {
              string addonId = (char*)msg->data;
              AudioDSPAddonMap_t::iterator iter = m_EnabledAddons.find(addonId);
              if (iter == m_EnabledAddons.end())
              {
                CLog::Log(LOGERROR, "%s, Tried to enable the unknown addon \"%s\"", __FUNCTION__, addonId.c_str());
              }
              else
              {
                iter->second->Destroy();
                m_DisabledAddons[addonId] = iter->second;
                //! @todo AudioDSP V2 remove addon from processing object
                m_EnabledAddons.erase(addonId);
              }
            }
            break;

            case CAudioDSPAddonControlProtocol::REMOVE_ADDON:
            break;

            default:
            break;
          }
        }
        else
        {
          m_hasError = true;
        }
      break;

      case ADSP_TOP_GET_PROCESSING_BUFFER:
        if (port == &m_ControlPort)
        {
          CAudioDSPControlProtocol::CCreateBuffer *bufferMsg = reinterpret_cast<CAudioDSPControlProtocol::CCreateBuffer*>(msg->data);
          IActiveAEProcessingBuffer *buffer = nullptr;

          static bool forceResampleBuffer = false;
          if (bufferMsg->audioStream->m_inputBuffers->m_format.m_dataFormat == AE_FMT_RAW || forceResampleBuffer)
          {
            buffer = dynamic_cast<IActiveAEProcessingBuffer*>(new CActiveAEStreamBuffers(bufferMsg->audioStream->m_inputBuffers->m_format, bufferMsg->outputFormat));
            CLog::Log(LOGDEBUG, "%s - Created CActiveAEStreamBuffers", __FUNCTION__);
          }
          else
          {
            CAudioDSPProcessingBuffer *adspBuffer = new CAudioDSPProcessingBuffer(bufferMsg->audioStream->m_id,
                                                                                  bufferMsg->audioStream->m_streamProperties,
                                                                                  bufferMsg->audioStream->m_inputBuffers->m_format,
                                                                                  bufferMsg->outputFormat,
                                                                                  m_Controller,
                                                                                  m_DSPChainModelObject);
            m_ProcessingBuffers[bufferMsg->audioStream->m_id] = adspBuffer;

            buffer = dynamic_cast<IActiveAEProcessingBuffer*>(adspBuffer);

            CLog::Log(LOGDEBUG, "%s - Created CAudioDSPProcessingBuffer", __FUNCTION__);
          }

          if (!buffer)
          {
            msg->Reply(CAudioDSPControlProtocol::ERR);
            CLog::Log(LOGERROR, "%s - Failed to create processing buffer!", __FUNCTION__);
          }
          else
          {
            msg->Reply(CAudioDSPControlProtocol::SUCCESS, &buffer, sizeof(IActiveAEProcessingBuffer*));
          }

          m_state = ADSP_TOP_CONFIGURED;
        }
        else
        {
          m_hasError = true;
        }
      break;

      case ADSP_TOP_RELEASE_PROCESSING_BUFFER:
        if (port == &m_ControlPort)
        {
          int streamID = *reinterpret_cast<int*>(msg->data);
          AudioDSPProcessingBufferMap_t::iterator iter = m_ProcessingBuffers.find(streamID);
          if (iter == m_ProcessingBuffers.end())
          {
            CLog::Log(LOGERROR, "%s - streamID \"%i\" in processing buffer map not found!", __FUNCTION__, streamID);
          }
          else
          {
            CAudioDSPProcessingBuffer *adspBuffer = dynamic_cast<CAudioDSPProcessingBuffer*>(iter->second);
            if (adspBuffer)
            {
              //! @todo AudioDSP V2 add specific deinitialization if needed
            }

            CActiveAEStreamBuffers *streamBuffer = dynamic_cast<CActiveAEStreamBuffers*>(iter->second);
            if (streamBuffer)
            {
              std::list<CActiveAEBufferPool*> discardBufferPools;
              discardBufferPools.push_back(streamBuffer->GetAtempoBuffers());
              discardBufferPools.push_back(streamBuffer->GetResampleBuffers());

              auto it = discardBufferPools.begin();
              while (it != discardBufferPools.end())
              {
                if (*it)
                {
                  CActiveAEBufferPoolResample *rbuf = dynamic_cast<CActiveAEBufferPoolResample*>(*it);
                  if (rbuf)
                  {
                    rbuf->Flush();
                  }

                  // if all buffers have returned, we can delete the buffer pool
                  if ((*it)->m_allSamples.size() == (*it)->m_freeSamples.size())
                  {
                    delete (*it);
                    CLog::Log(LOGDEBUG, "%s - buffer pool deleted", __FUNCTION__);
                    it = discardBufferPools.erase(it);
                  }
                  else
                  {
                    ++it;
                  }
                }
              }
            }

            iter->second->Flush();
            iter->second->Destroy();
            delete iter->second;

            CLog::Log(LOGDEBUG, "%s - Destroyed processing buffer with stream ID %i", __FUNCTION__, streamID);

            m_ProcessingBuffers.erase(iter);
          }

          m_state = ADSP_TOP_CONFIGURED;
        }
        else
        {
          m_hasError = true;
        }
      break;

      default: // AudioDSP is in an unknown state, should not happen!
        CLog::Log(LOGERROR, "CActiveAudioDSP::%s - no valid state: %d", __FUNCTION__, m_state);
        return;
    } // switch
  } // for
}

void CActiveAudioDSP::PrepareAddons()
{
  BinaryAddonBaseList addonInfos;
  CServiceBroker::GetBinaryAddonManager().GetAddonInfos(addonInfos, false, ADDON_ADSPDLL);

  if (addonInfos.empty())
    return;

  for (auto &addonInfo : addonInfos)
  {
    AE_DSP_ADDON dspAddon = std::make_shared<CActiveAEDSPAddon>(addonInfo);
    if(!dspAddon)
    {
      CLog::Log(LOGERROR, "%s - failed to cast addon to CActiveAEDSPAddon", __FUNCTION__);
      continue;
    }
    if (CServiceBroker::GetAddonMgr().IsAddonDisabled(addonInfo->ID()))
    {
      m_DisabledAddons[addonInfo->ID()] = dspAddon;
    }
    else
    {
      std::hash<std::string> hasher;
      unsigned int uiAddonId = hasher(addonInfo->ID());
      if (!dspAddon->Create(uiAddonId))
      {
        CServiceBroker::GetAddonMgr().DisableAddon(dspAddon->ID());
        CLog::Log(LOGERROR, "failed to create add-on \"%s\"", dspAddon->ID().c_str());
      }
      else
      {
        m_EnabledAddons[addonInfo->ID()] = dspAddon;
      }
    }
  }
}

void CActiveAudioDSP::PrepareAddonModes()
{
  IDSPNodeModel::DSPNodeInfoVector_t tmpNodeInfos;
  IDSPNodeModel::DSPNodeInfoVector_t tmpActiveNodeInfos;

  // get all add-on modes
  for (AudioDSPAddonMap_t::iterator iter = m_EnabledAddons.begin(); iter != m_EnabledAddons.end(); ++iter)
  {
    CActiveAEDSPAddon::CAudioDSPModeVector_t modes;
    iter->second->GetAudioDSPModes(modes);

    if (modes.size() <= 0)
    {
      CLog::Log(LOGWARNING, "%s - the add-on %s doesn't provide any valid signal processing modes", __FUNCTION__, iter->second->ID().c_str());
    }

    for (unsigned int modeCount = 0; modeCount < modes.size(); modeCount++)
    {
      CAudioDSPAddonNodeCreator addonNodeCreator(iter->second);
      DSPErrorCode_t err = m_DSPChainModelObject.RegisterNode(IDSPNodeModel::CDSPNodeInfoQuery({ iter->second->ID(), modes[modeCount].strModeName }), addonNodeCreator);
      if (err != DSP_ERR_NO_ERR)
      {
        CLog::Log(LOGERROR, "%s failed to register %s from add-on %s!", __FUNCTION__, modes[modeCount].strModeName.c_str(), iter->second->Name().c_str());
      }
    }
  }

  // set mode order from json file
  //GetActiveNodesFromJsonFile(tmpActiveNodeInfos);
  for (IDSPNodeModel::DSPNodeInfoVector_t::iterator iter = tmpActiveNodeInfos.begin(); iter != tmpActiveNodeInfos.end(); ++iter)
  {
    uint32_t addonID;         //! @todo AudioDSP V2 get real parameters
    uint16_t modeID;          //! @todo AudioDSP V2 get real parameters
    uint16_t modeInstanceID;  //! @todo AudioDSP V2 get real parameters
    uint32_t pos;             //! @todo AudioDSP V2 get real parameters
    
    NodeID_t id(addonID, modeID, modeInstanceID);

    m_DSPChainModelObject.EnableNode(id, pos);
  }


  // temporarily enable adsp.freesurround and audio converter
  IDSPNodeModel::DSPNodeInfoVector_t nodeInfos;
  m_DSPChainModelObject.GetNodeInfos(nodeInfos);

  for (auto &it : nodeInfos)
  {
    if (it.Name == "Kodi::AudioConverter")
    {
      m_DSPChainModelObject.EnableNode(it.ID, 0);
    }

    if (it.Name == "adsp.freesurround::Free Surround")
    {
      m_DSPChainModelObject.EnableNode(it.ID, 1);
    }
  }
}

void CActiveAudioDSP::CreateDSPNodeModel()
{
}

bool CActiveAudioDSP::IsInitialized()
{
  CSingleLock lock(m_lock);

  return m_isInitialized;
}

void CActiveAudioDSP::OnSettingAction(std::shared_ptr<const CSetting> Setting)
{
  if (Setting == NULL || !IsInitialized())
  {
    return;
  }

  const std::string &settingId = Setting->GetId();
  if (settingId == CSettings::SETTING_AUDIOOUTPUT_DSPSETTINGS)
  {
    CGUIDialogAudioDSPManager *dialog = dynamic_cast<CGUIDialogAudioDSPManager*>(g_windowManager.GetWindow(WINDOW_DIALOG_AUDIO_DSP_MANAGER));
    if (dialog)
    {
      dialog->Open();
    }
  }
}

void CActiveAudioDSP::Process()
{
  Message *msg = nullptr;
  Protocol *port = nullptr;
  bool gotMsg;
  XbmcThreads::EndTime timer;

  m_state = ADSP_TOP_UNCONFIGURED;
  m_extTimeout = 1000;
  m_bStateMachineSelfTrigger = false;

  while (!m_bStop)
  {
    gotMsg = false;
    timer.Set(m_extTimeout);

    if (m_bStateMachineSelfTrigger)
    {
      m_bStateMachineSelfTrigger = false;
      // self trigger state machine
      StateMachine(msg->signal, port, msg);
      if (!m_bStateMachineSelfTrigger)
      {
        msg->Release();
        msg = nullptr;
      }
      continue;
    }
    else if (m_ControlPort.ReceiveOutMessage(&msg))
    {
      gotMsg = true;
      port = &m_ControlPort;
    }
    // check control port
    else if (m_AddonControlPort.ReceiveOutMessage(&msg))
    {
      gotMsg = true;
      port = &m_AddonControlPort;
    }

    if (gotMsg)
    {
      StateMachine(msg->signal, port, msg);
      if (!m_bStateMachineSelfTrigger)
      {
        msg->Release();
        msg = nullptr;
      }
      continue;
    }
    else if (m_outMsgEvent.WaitMSec(m_extTimeout))
    { // wait for message
      m_extTimeout = timer.MillisLeft();
      continue;
    }
    else
    { // time out
      msg = m_AddonControlPort.GetMessage();
      msg->signal = CAudioDSPControlProtocol::TIMEOUT;
      port = nullptr;
      // signal timeout to state machine
      StateMachine(msg->signal, port, msg);
      if (!m_bStateMachineSelfTrigger)
      {
        msg->Release();
        msg = nullptr;
      }
    }
  }
}
};
