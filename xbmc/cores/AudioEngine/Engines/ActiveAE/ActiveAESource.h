#pragma once
/*
 *      Copyright (C) 2010-2013 Team XBMC
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

#include "threads/Event.h"
#include "threads/Thread.h"
#include "utils/ActorProtocol.h"
#include "cores/AudioEngine/Interfaces/AE.h"
#include "cores/AudioEngine/Interfaces/AESource.h"
#include "cores/AudioEngine/AESourceFactory.h"
#include "cores/AudioEngine/Interfaces/AEResample.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"

namespace ActiveAE
{
using namespace Actor;

class CEngineStats;

struct SourceConfig
{
  AEAudioFormat format;
  CEngineStats *stats;
  const std::string *device;
};

struct SourceReply
{
  AEAudioFormat format;
  float cacheTotal;
  float latency;
  bool hasVolume;
};

class CSourceControlProtocol : public Protocol
{
public:
  CSourceControlProtocol(std::string name, CEvent* inEvent, CEvent *outEvent) : Protocol(name, inEvent, outEvent) {};
  enum OutSignal
  {
    CONFIGURE,
    UNCONFIGURE,
    STREAMING,
    APPFOCUSED,
    VOLUME,
    FLUSH,
    TIMEOUT,
  };
  // ToDo: for what is this used?
  enum InSignal
  {
    ACC,
    ERR,
    STATS,
  };
};

class CSourceDataProtocol : public Protocol
{
public:
  CSourceDataProtocol(std::string name, CEvent* inEvent, CEvent *outEvent) : Protocol(name, inEvent, outEvent) {};
  enum OutSignal
  {
    SAMPLE = 0,
    DRAIN,
  };
  enum InSignal
  {
    RETURNSAMPLE,
    ACC,
  };
};

class CActiveAESource : private CThread
{
public:
  CActiveAESource(CEvent *inMsgEvent);
  void EnumerateSourceList(bool force);
  void EnumerateInputDevices(AEDeviceList &devices, bool passthrough);
  std::string GetDefaultInputDevice(bool passthrough);
  void Start();
  void Dispose();
  AEDeviceType GetInputDeviceType(const std::string &device);
  bool HasPassthroughInputDevice();
  bool SupportsFormat(const std::string &device, AEDataFormat format, int samplerate);
  CSourceControlProtocol m_controlPort;
  CSourceDataProtocol m_dataPort;

protected:
  void Process();
  void StateMachine(int signal, Protocol *port, Message *msg);
  void PrintSources();
  void GetDeviceFriendlyName(std::string &device);
  void OpenSource();
  void ReturnBuffers();
  void SetSilenceTimer();

  unsigned int InputSamples(CSampleBuffer* samples);
  void SwapInit(CSampleBuffer* samples);

  void GenerateNoise();

  CEvent m_outMsgEvent;
  CEvent *m_inMsgEvent;
  int m_state;
  bool m_bStateMachineSelfTrigger;
  int m_extTimeout;
  bool m_extError;
  unsigned int m_extSilenceTimeout;
  bool m_extAppFocused;
  bool m_extStreaming;
  XbmcThreads::EndTime m_extSilenceTimer;

  CSampleBuffer m_sampleOfSilence;
  enum
  {
    CHECK_SWAP,
    NEED_CONVERT,
    NEED_BYTESWAP,
    SKIP_SWAP,
  } m_swapState;

  std::string m_deviceFriendlyName;
  std::string m_device;
  AESourceInfoList m_sourceInfoList;
  IAESource *m_source;
  AEAudioFormat m_sourceFormat, m_requestedFormat;
  CEngineStats *m_stats;
  float m_volume;
  int m_sourceLatency;
};

}
