#pragma once
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

#include "threads/Event.h"
#include "threads/Thread.h"
#include "utils/ActorProtocol.h"
#include "settings/Settings.h"

#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPDatabase.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"

#include "cores/AudioEngine/Interfaces/AEAudioDSP.h"

// MVC Pattern object includes
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPNodeModel.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPController.h"

// internal Kodi AudioDSP processing mode include files
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioDSPKodiModes.h"

#include "addons/binary-addons/BinaryAddonManager.h"

#include <map>
#include <memory>

// notes for configuration of ActiveAE and AudioDSP
// first configure AE
// then AudioDSP
// after that configure the sink

namespace ActiveAE
{
using namespace Actor;
class CActiveAEStream;
class IActiveAEProcessingBuffer;
class CAudioDSPProcessingBuffer;

class CAudioDSPAddonControlProtocol : public Protocol
{
public:
  CAudioDSPAddonControlProtocol(std::string name, CEvent *inEvent, CEvent *outEvent) : Protocol(name, inEvent, outEvent) {}
  enum OutSignal
  {
    REMOVE_ADDON,
    ENABLE_ADDON,
    DISABLE_ADDON,
  };
  enum InSignal
  {
    ACC,
    ERR,
  };
};

class CAudioDSPControlProtocol : public Protocol
{
public:
  struct CCreateBuffer
  {
    CCreateBuffer(const CActiveAEStream *AudioStream, AEAudioFormat &OutputFormat) : audioStream(AudioStream), outputFormat(OutputFormat) {}
    const CActiveAEStream *audioStream;
    AEAudioFormat &outputFormat;
  };

  CAudioDSPControlProtocol(std::string name, CEvent *inEvent, CEvent *outEvent) : Protocol(name, inEvent, outEvent) {}
  enum OutSignal
  {
    DEINIT = 0,
    INIT,
    GET_PROCESSING_BUFFER,
    RELEASE_PROCESSING_BUFFER,
    TIMEOUT,
  };
  enum InSignal
  {
    ACC,
    SUCCESS,
    ERR,
  };
};

class CActiveAudioDSP : public IAEAudioDSP,
                        public ADDON::IBinaryAddonManagerCallback,
                        public ISettingCallback,
                        private CThread
{
  typedef std::shared_ptr<ActiveAE::CActiveAEDSPAddon>  pAudioDSPAddon_t;
  typedef std::map<std::string, pAudioDSPAddon_t>       AudioDSPAddonMap_t;
  typedef std::map<int, CAudioDSPProcessingBuffer*>     AudioDSPProcessingBufferMap_t;

public:
  CActiveAudioDSP(CEvent *inMsgEvent);
  ~CActiveAudioDSP();
  void Start();
  void Stop();
  
  // IBinaryAddonManagerCallback interface implementation
  virtual void EnableEvent(ADDON::BinaryAddonBasePtr addon) override;
  virtual void DisableEvent(ADDON::BinaryAddonBasePtr addon) override;
  virtual void InstalledEvent(ADDON::BinaryAddonBasePtr addon) override;
  virtual void DeinstalledEvent(ADDON::BinaryAddonBasePtr addon) override;

  virtual void EnableAddon(const std::string &Id, bool Enable) override;
  virtual bool GetAddon(const std::string &Id, ADDON::AddonPtr &addon) override;

  virtual void RegisterAddon(const std::string &Id, bool restart = false, bool update = false) override;
  virtual void UnregisterAddon(const std::string &Id) override;

  void* GetControllerHandle(void *ControllerCallback = nullptr);
  bool ReleaseControllerHandle(void **Handle);

  IActiveAEProcessingBuffer* GetProcessingBuffer(const CActiveAEStream *AudioStream, AEAudioFormat &OutputFormat);
  DSPErrorCode_t ReleaseProcessingBuffer(int StreamID);

  // internal Kodi AudioDSP modes
  CAudioDSPKodiModes m_KodiModes;

protected:
  // ports
  CAudioDSPControlProtocol m_ControlPort;
  CAudioDSPAddonControlProtocol m_AddonControlPort;

  // state machine variables
  void Process();
  void StateMachine(int signal, Protocol *port, Message *msg);

  CEvent m_outMsgEvent;
  CEvent *m_inMsgEvent;
  int m_state;
  unsigned int m_extSilenceTimeout;
  XbmcThreads::EndTime m_extSilenceTimer;

  bool m_hasError;
  int m_extTimeout;
  bool m_bStateMachineSelfTrigger;
  
private:
  void PrepareAddons();
  void PrepareAddonModes();
  void CreateDSPNodeModel();
  
  CCriticalSection m_lock;
  bool m_isInitialized;
  bool IsInitialized();


  // ISettingCallback interface implementation
  virtual void OnSettingAction(std::shared_ptr<const CSetting> Setting) override;

  AudioDSPProcessingBufferMap_t m_ProcessingBuffers;

  DSP::CDSPNodeModel m_DSPChainModelObject;
  CAudioDSPController m_Controller;
  AudioDSPAddonMap_t m_EnabledAddons;
  AudioDSPAddonMap_t m_DisabledAddons;
  CActiveAEDSPDatabase m_databaseDSP;  /*!< database for all audio DSP related data */
};
}
