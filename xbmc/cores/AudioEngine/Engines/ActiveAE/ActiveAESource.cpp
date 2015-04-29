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

#include <sstream>

#include "ActiveAESource.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "utils/EndianSwap.h"
#include "ActiveAE.h"
#include "cores/AudioEngine/AEResampleFactory.h"

#include "settings/Settings.h"

#include <new> // for std::bad_alloc
#include <algorithm>

using namespace ActiveAE;

CActiveAESource::CActiveAESource(CEvent *inMsgEvent) :
  CThread("AESource"),
  m_controlPort("SourceControlPort", inMsgEvent, &m_outMsgEvent),
  m_dataPort("SourceDataPort", inMsgEvent, &m_outMsgEvent)
{
  m_inMsgEvent = inMsgEvent;
  m_source = NULL;
  m_stats = NULL;
  m_volume = 0.0;
}

void CActiveAESource::Start()
{
  if (!IsRunning())
  {
    Create();
    SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
  }
}

void CActiveAESource::Dispose()
{
  m_bStop = true;
  m_outMsgEvent.Set();
  StopThread();
  m_controlPort.Purge();
  m_dataPort.Purge();

  if (m_source)
  {
    m_source->Drain();
    m_source->Deinitialize();
    delete m_source;
    m_source = NULL;
  }

  if(m_sampleOfSilence.pkt)
  {
    delete m_sampleOfSilence.pkt;
    m_sampleOfSilence.pkt = NULL;
  }
}

AEDeviceType CActiveAESource::GetInputDeviceType(const std::string &device)
{
  std::string dev = device;
  std::string dri;
  CAESourceFactory::ParseDevice(dev, dri);
  for (AESourceInfoList::iterator itt = m_sourceInfoList.begin(); itt != m_sourceInfoList.end(); ++itt)
  {
    for (AEDeviceInfoList::iterator itt2 = itt->m_deviceInfoList.begin(); itt2 != itt->m_deviceInfoList.end(); ++itt2)
    {
      CAEDeviceInfo& info = *itt2;
      if (info.m_deviceName == dev)
        return info.m_deviceType;
    }
  }
  return AE_DEVTYPE_PCM;
}

bool CActiveAESource::HasPassthroughInputDevice()
{
  for (AESourceInfoList::iterator itt = m_sourceInfoList.begin(); itt != m_sourceInfoList.end(); ++itt)
  {
    for (AEDeviceInfoList::iterator itt2 = itt->m_deviceInfoList.begin(); itt2 != itt->m_deviceInfoList.end(); ++itt2)
    {
      CAEDeviceInfo& info = *itt2;
      if (info.m_deviceType != AE_DEVTYPE_PCM)
        return true;
    }
  }
  return false;
}

bool CActiveAESource::SupportsFormat(const std::string &device, AEDataFormat format, int samplerate)
{
  std::string dev = device;
  std::string dri;
  CAESourceFactory::ParseDevice(dev, dri);
  for (AESourceInfoList::iterator itt = m_sourceInfoList.begin(); itt != m_sourceInfoList.end(); ++itt)
  {
    if (dri == itt->m_sourceName)
    {
      for (AEDeviceInfoList::iterator itt2 = itt->m_deviceInfoList.begin(); itt2 != itt->m_deviceInfoList.end(); ++itt2)
      {
        CAEDeviceInfo& info = *itt2;
        if (info.m_deviceName == dev)
        {
          AEDataFormatList::iterator itt3;
          itt3 = find(info.m_dataFormats.begin(), info.m_dataFormats.end(), format);
          if (itt3 != info.m_dataFormats.end())
          {
            AESampleRateList::iterator itt4;
            itt4 = find(info.m_sampleRates.begin(), info.m_sampleRates.end(), samplerate);
            if (itt4 != info.m_sampleRates.end())
              return true;
            else
              return false;
          }
          else
            return false;
        }
      }
    }
  }
  return false;
}

enum SOURCE_STATES
{
  S_TOP = 0,                      // 0
  S_TOP_UNCONFIGURED,             // 1
  S_TOP_CONFIGURED,               // 2
  S_TOP_CONFIGURED_SUSPEND,       // 3
  S_TOP_CONFIGURED_IDLE,          // 4
  S_TOP_CONFIGURED_CAPTURE,       // 5
  S_TOP_CONFIGURED_SILENCE,       // 6
};

int SOURCE_parentStates[] = {
    -1,
    0, //TOP_UNCONFIGURED
    0, //TOP_CONFIGURED
    2, //TOP_CONFIGURED_SUSPEND
    2, //TOP_CONFIGURED_IDLE
    2, //TOP_CONFIGURED_CAPTURE
    2, //TOP_CONFIGURED_SILENCE
};

void CActiveAESource::StateMachine(int signal, Protocol *port, Message *msg)
{
  for (int state = m_state; ; state = SOURCE_parentStates[state])
  {
    switch (state)
    {
    case S_TOP: // TOP
      if (port == &m_controlPort)
      {
        switch (signal)
        {
        case CSourceControlProtocol::CONFIGURE:
          SourceConfig *data;
          data = (SourceConfig*)msg->data;
          if (data)
          {
            m_requestedFormat = data->format;
            m_stats = data->stats;
            m_device = *(data->device);
          }
          m_extError = false;
          m_extSilenceTimer = 0;
          m_extStreaming = false;
          ReturnBuffers();
          OpenSource();

          if (!m_extError)
          {
            SourceReply reply;
            reply.format = m_sourceFormat;
            reply.cacheTotal = m_source->GetCacheTotal();
            reply.latency = m_source->GetLatency();
            reply.hasVolume = m_source->HasVolume();
            m_state = S_TOP_CONFIGURED_IDLE;
            m_extTimeout = 10000;
            m_sourceLatency = (int64_t)(reply.latency * 1000);
            msg->Reply(CSourceControlProtocol::ACC, &reply, sizeof(SourceReply));
          }
          else
          {
            m_state = S_TOP_UNCONFIGURED;
            msg->Reply(CSourceControlProtocol::ERR);
          }
          return;

        case CSourceControlProtocol::UNCONFIGURE:
          ReturnBuffers();
          if (m_source)
          {
            m_source->Drain();
            m_source->Deinitialize();
            delete m_source;
            m_source = NULL;
          }
          m_state = S_TOP_UNCONFIGURED;
          msg->Reply(CSourceControlProtocol::ACC);
          return;

        case CSourceControlProtocol::FLUSH:
          ReturnBuffers();
          msg->Reply(CSourceControlProtocol::ACC);
          return;

        case CSourceControlProtocol::APPFOCUSED:
          m_extAppFocused = *(bool*)msg->data;
          SetSilenceTimer();
          m_extTimeout = 0;
          return;

        case CSourceControlProtocol::STREAMING:
          m_extStreaming = *(bool*)msg->data;
          return;

        default:
          break;
        }
      }
      else if (port == &m_dataPort)
      {
        switch (signal)
        {
        case CSourceDataProtocol::DRAIN:
          msg->Reply(CSourceDataProtocol::ACC);
          m_state = S_TOP_UNCONFIGURED;
          m_extTimeout = 0;
          return;
        default:
          break;
        }
      }
      {
        std::string portName = port == NULL ? "timer" : port->portName;
        CLog::Log(LOGWARNING, "CActiveAESource::%s - signal: %d form port: %s not handled for state: %d", __FUNCTION__, signal, portName.c_str(), m_state);
      }
      return;

    case S_TOP_UNCONFIGURED:
      if (port == NULL) // timeout
      {
        switch (signal)
        {
        case CSourceControlProtocol::TIMEOUT:
          m_extTimeout = 1000;
          return;
        default:
          break;
        }
      }
      else if (port == &m_dataPort)
      {
        switch (signal)
        {
        case CSourceDataProtocol::SAMPLE:
          CSampleBuffer *samples;
          int timeout;
          samples = *((CSampleBuffer**)msg->data);
          timeout = 1000*samples->pkt->nb_samples/samples->pkt->config.sample_rate;
          Sleep(timeout);
          msg->Reply(CSourceDataProtocol::RETURNSAMPLE, &samples, sizeof(CSampleBuffer*));
          m_extTimeout = 0;
          return;
        default:
          break;
        }
      }
      break;

    case S_TOP_CONFIGURED:
      if (port == &m_controlPort)
      {
        switch (signal)
        {
        case CSourceControlProtocol::STREAMING:
          m_extStreaming = *(bool*)msg->data;
          SetSilenceTimer();
          if (!m_extSilenceTimer.IsTimePast())
          {
            m_state = S_TOP_CONFIGURED_SILENCE;
          }
          m_extTimeout = 0;
          return;
        case CSourceControlProtocol::VOLUME:
          m_volume = *(float*)msg->data;
          m_source->SetVolume(m_volume);
          return;
        default:
          break;
        }
      }
      else if (port == &m_dataPort)
      {
        switch (signal)
        {
        case CSourceDataProtocol::DRAIN:
          m_source->Drain();
          msg->Reply(CSourceDataProtocol::ACC);
          m_state = S_TOP_CONFIGURED_IDLE;
          m_extTimeout = 10000;
          return;
        case CSourceDataProtocol::SAMPLE:
          CSampleBuffer *samples;
          unsigned int delay;
          samples = *((CSampleBuffer**)msg->data);
          delay = InputSamples(samples);
          msg->Reply(CSourceDataProtocol::RETURNSAMPLE, &samples, sizeof(CSampleBuffer*));
          if (m_extError)
          {
            m_source->Deinitialize();
            delete m_source;
            m_source = NULL;
            m_state = S_TOP_CONFIGURED_SUSPEND;
            m_extTimeout = 0;
          }
          else
          {
            m_state = S_TOP_CONFIGURED_CAPTURE;
            m_extTimeout = delay / 2;
            m_extSilenceTimer.Set(m_extSilenceTimeout);
          }
          return;
        default:
          break;
        }
      }
      break;

    case S_TOP_CONFIGURED_SUSPEND:
      if (port == &m_controlPort)
      {
        switch (signal)
        {
        case CSourceControlProtocol::STREAMING:
          m_extStreaming = *(bool*)msg->data;
          SetSilenceTimer();
          m_extTimeout = 0;
          return;
        case CSourceControlProtocol::VOLUME:
          m_volume = *(float*)msg->data;
          return;
        default:
          break;
        }
      }
      else if (port == &m_dataPort)
      {
        switch (signal)
        {
        case CSourceDataProtocol::SAMPLE:
          m_extError = false;
          OpenSource();
          InputSamples(&m_sampleOfSilence);
          m_state = S_TOP_CONFIGURED_CAPTURE;
          m_extTimeout = 0;
          m_bStateMachineSelfTrigger = true;
          return;
        case CSourceDataProtocol::DRAIN:
          msg->Reply(CSourceDataProtocol::ACC);
          return;
        default:
          break;
        }
      }
      else if (port == NULL) // timeout
      {
        switch (signal)
        {
        case CSourceControlProtocol::TIMEOUT:
          m_extTimeout = 10000;
          return;
        default:
          break;
        }
      }
      break;

    case S_TOP_CONFIGURED_IDLE:
      if (port == &m_dataPort)
      {
        switch (signal)
        {
        case CSourceDataProtocol::SAMPLE:
          InputSamples(&m_sampleOfSilence);
          m_state = S_TOP_CONFIGURED_CAPTURE;
          m_extTimeout = 0;
          m_bStateMachineSelfTrigger = true;
          return;
        default:
          break;
        }
      }
      else if (port == NULL) // timeout
      {
        switch (signal)
        {
        case CSourceControlProtocol::TIMEOUT:
          m_source->Deinitialize();
          delete m_source;
          m_source = NULL;
          m_state = S_TOP_CONFIGURED_SUSPEND;
          m_extTimeout = 10000;
          return;
        default:
          break;
        }
      }
      break;

    case S_TOP_CONFIGURED_CAPTURE:
      if (port == NULL) // timeout
      {
        switch (signal)
        {
        case CSourceControlProtocol::TIMEOUT:
          if (!m_extSilenceTimer.IsTimePast())
          {
            m_state = S_TOP_CONFIGURED_SILENCE;
            m_extTimeout = 0;
          }
          else
          {
            m_source->Drain();
            m_state = S_TOP_CONFIGURED_IDLE;
            if (m_extAppFocused)
              m_extTimeout = 10000;
            else
              m_extTimeout = 0;
          }
          return;
        default:
          break;
        }
      }
      break;

    case S_TOP_CONFIGURED_SILENCE:
      if (port == NULL) // timeout
      {
        switch (signal)
        {
        case CSourceControlProtocol::TIMEOUT:
          InputSamples(&m_sampleOfSilence);
          if (m_extError)
          {
            m_source->Deinitialize();
            delete m_source;
            m_source = NULL;
            m_state = S_TOP_CONFIGURED_SUSPEND;
          }
          else
            m_state = S_TOP_CONFIGURED_CAPTURE;
            m_extTimeout = 0;
          return;
        default:
          break;
        }
      }
      break;

    default: // we are in no state, should not happen
      CLog::Log(LOGERROR, "CActiveAESource::%s - no valid state: %d", __FUNCTION__, m_state);
      return;
    }
  } // for
}

void CActiveAESource::Process()
{
  Message *msg = NULL;
  Protocol *port = NULL;
  bool gotMsg;
  XbmcThreads::EndTime timer;

  m_state = S_TOP_UNCONFIGURED;
  m_extTimeout = 1000;
  m_bStateMachineSelfTrigger = false;
  m_extAppFocused = true;

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
        msg = NULL;
      }
      continue;
    }
    // check control port
    else if (m_controlPort.ReceiveOutMessage(&msg))
    {
      gotMsg = true;
      port = &m_controlPort;
    }
    // check data port
    else if (m_dataPort.ReceiveOutMessage(&msg))
    {
      gotMsg = true;
      port = &m_dataPort;
    }

    if (gotMsg)
    {
      StateMachine(msg->signal, port, msg);
      if (!m_bStateMachineSelfTrigger)
      {
        msg->Release();
        msg = NULL;
      }
      continue;
    }

    // wait for message
    else if (m_outMsgEvent.WaitMSec(m_extTimeout))
    {
      m_extTimeout = timer.MillisLeft();
      continue;
    }
    // time out
    else
    {
      msg = m_controlPort.GetMessage();
      msg->signal = CSourceControlProtocol::TIMEOUT;
      port = 0;
      // signal timeout to state machine
      StateMachine(msg->signal, port, msg);
      if (!m_bStateMachineSelfTrigger)
      {
        msg->Release();
        msg = NULL;
      }
    }
  }
}

void CActiveAESource::EnumerateSourceList(bool force)
{
  if (!m_sourceInfoList.empty() && !force)
    return;

  unsigned int c_retry = 4;
  m_sourceInfoList.clear();
  CAESourceFactory::EnumerateEx(m_sourceInfoList);
  while(m_sourceInfoList.size() == 0 && c_retry > 0)
  {
    CLog::Log(LOGDEBUG, "No Devices found - retry: %d", c_retry);
    Sleep(1500);
    c_retry--;
    // retry the enumeration
    CAESourceFactory::EnumerateEx(m_sourceInfoList, true);
  }
  CLog::Log(LOGDEBUG, "Found %lu Lists of Devices", m_sourceInfoList.size());
  PrintSources();
}

void CActiveAESource::PrintSources()
{
  for (AESourceInfoList::iterator itt = m_sourceInfoList.begin(); itt != m_sourceInfoList.end(); ++itt)
  {
    CLog::Log(LOGDEBUG, "Enumerated %s devices:", itt->m_sourceName.c_str());
    int count = 0;
    for (AEDeviceInfoList::iterator itt2 = itt->m_deviceInfoList.begin(); itt2 != itt->m_deviceInfoList.end(); ++itt2)
    {
      CLog::Log(LOGDEBUG, "    Device %d", ++count);
      CAEDeviceInfo& info = *itt2;
      std::stringstream ss((std::string)info);
      std::string line;
      while(std::getline(ss, line, '\n'))
        CLog::Log(LOGDEBUG, "        %s", line.c_str());
    }
  }
}

void CActiveAESource::EnumerateInputDevices(AEDeviceList &devices, bool passthrough)
{
  EnumerateSourceList(false);

  for (AESourceInfoList::iterator itt = m_sourceInfoList.begin(); itt != m_sourceInfoList.end(); ++itt)
  {
    AESourceInfo sourceInfo = *itt;
    for (AEDeviceInfoList::iterator itt2 = sourceInfo.m_deviceInfoList.begin(); itt2 != sourceInfo.m_deviceInfoList.end(); ++itt2)
    {
      CAEDeviceInfo devInfo = *itt2;
      if (passthrough && devInfo.m_deviceType == AE_DEVTYPE_PCM)
        continue;

      std::string device = sourceInfo.m_sourceName + ":" + devInfo.m_deviceName;

      std::stringstream ss;

      /* add the source name if we have more then one source type */
      if (m_sourceInfoList.size() > 1)
        ss << sourceInfo.m_sourceName << ": ";

      ss << devInfo.m_displayName;
      if (!devInfo.m_displayNameExtra.empty())
        ss << ", " << devInfo.m_displayNameExtra;

      devices.push_back(AEDevice(ss.str(), device));
    }
  }
}

std::string CActiveAESource::GetDefaultInputDevice(bool passthrough)
{
  EnumerateSourceList(false);

  for (AESourceInfoList::iterator itt = m_sourceInfoList.begin(); itt != m_sourceInfoList.end(); ++itt)
  {
    AESourceInfo sourceInfo = *itt;
    for (AEDeviceInfoList::iterator itt2 = sourceInfo.m_deviceInfoList.begin(); itt2 != sourceInfo.m_deviceInfoList.end(); ++itt2)
    {
      CAEDeviceInfo devInfo = *itt2;
      if (passthrough && devInfo.m_deviceType == AE_DEVTYPE_PCM)
        continue;

      std::string device = sourceInfo.m_sourceName + ":" + devInfo.m_deviceName;
      return device;
    }
  }
  return "default";
}

void CActiveAESource::GetDeviceFriendlyName(std::string &device)
{
  m_deviceFriendlyName = "Device not found";
  /* Match the device and find its friendly name */
  for (AESourceInfoList::iterator itt = m_sourceInfoList.begin(); itt != m_sourceInfoList.end(); ++itt)
  {
    AESourceInfo sourceInfo = *itt;
    for (AEDeviceInfoList::iterator itt2 = sourceInfo.m_deviceInfoList.begin(); itt2 != sourceInfo.m_deviceInfoList.end(); ++itt2)
    {
      CAEDeviceInfo& devInfo = *itt2;
      if (devInfo.m_deviceName == device)
      {
        m_deviceFriendlyName = devInfo.m_displayName;
        break;
      }
    }
  }
  return;
}

void CActiveAESource::OpenSource()
{
  // we need a copy of m_device here because ParseDevice and CreateDevice write back
  // into this variable
  std::string device = m_device;
  std::string driver;
  bool passthrough = AE_IS_RAW(m_requestedFormat.m_dataFormat);

  CAESourceFactory::ParseDevice(device, driver);
  if (driver.empty() && m_source)
    driver = m_source->GetName();

  CLog::Log(LOGINFO, "CActiveAESource::OpenSink - initialize source");

  if (m_source)
  {
    m_source->Drain();
    m_source->Deinitialize();
    delete m_source;
    m_source = NULL;
  }

  // get the display name of the device
  GetDeviceFriendlyName(device);

  // if we already have a driver, prepend it to the device string
  if (!driver.empty())
    device = driver + ":" + device;

  // WARNING: this changes format and does not use passthrough
  m_sourceFormat = m_requestedFormat;
  CLog::Log(LOGDEBUG, "CActiveAESource::OpenSink - trying to open device %s", device.c_str());
  m_source = CAESourceFactory::Create(device, m_sourceFormat, passthrough);

  // try first device in out list
  if (!m_source && !m_sourceInfoList.empty())
  {
    driver = m_sourceInfoList.front().m_sourceName;
    device = m_sourceInfoList.front().m_deviceInfoList.front().m_deviceName;
    GetDeviceFriendlyName(device);
    if (!driver.empty())
      device = driver + ":" + device;
    m_sourceFormat = m_requestedFormat;
    CLog::Log(LOGDEBUG, "CActiveAESource::OpenSink - trying to open device %s", device.c_str());
    m_source = CAESourceFactory::Create(device, m_sourceFormat, passthrough);
  }

  // open NULL source
  // TODO: should not be required by ActiveAE
  if (!m_source)
  {
    device = "NULL:NULL";
    m_sourceFormat = m_requestedFormat;
    CLog::Log(LOGDEBUG, "CActiveAESource::OpenSink - open NULL source");
    m_source = CAESourceFactory::Create(device, m_sourceFormat, passthrough);
  }

  if (!m_source)
  {
    CLog::Log(LOGERROR, "CActiveAESource::OpenSink - no source was returned");
    m_extError = true;
    return;
  }

  m_source->SetVolume(m_volume);

#ifdef WORDS_BIGENDIAN
  if (m_sourceFormat.m_dataFormat == AE_FMT_S16BE)
    m_sourceFormat.m_dataFormat = AE_FMT_S16NE;
  else if (m_sourceFormat.m_dataFormat == AE_FMT_S32BE)
    m_sourceFormat.m_dataFormat = AE_FMT_S32NE;
#else
  if (m_sourceFormat.m_dataFormat == AE_FMT_S16LE)
    m_sourceFormat.m_dataFormat = AE_FMT_S16NE;
  else if (m_sourceFormat.m_dataFormat == AE_FMT_S32LE)
    m_sourceFormat.m_dataFormat = AE_FMT_S32NE;
#endif

  CLog::Log(LOGDEBUG, "CActiveAESource::OpenSource - %s Initialized:", m_source->GetName());
  CLog::Log(LOGDEBUG, "  Input Device  : %s", m_deviceFriendlyName.c_str());
  CLog::Log(LOGDEBUG, "  Sample Rate   : %d", m_sourceFormat.m_sampleRate);
  CLog::Log(LOGDEBUG, "  Sample Format : %s", CAEUtil::DataFormatToStr(m_sourceFormat.m_dataFormat));
  CLog::Log(LOGDEBUG, "  Channel Count : %d", m_sourceFormat.m_channelLayout.Count());
  CLog::Log(LOGDEBUG, "  Channel Layout: %s", ((std::string)m_sourceFormat.m_channelLayout).c_str());
  CLog::Log(LOGDEBUG, "  Frames        : %d", m_sourceFormat.m_frames);
  CLog::Log(LOGDEBUG, "  Frame Samples : %d", m_sourceFormat.m_frameSamples);
  CLog::Log(LOGDEBUG, "  Frame Size    : %d", m_sourceFormat.m_frameSize);

  // init sample of silence
  SampleConfig config;
  config.fmt = CAEUtil::GetAVSampleFormat(m_sourceFormat.m_dataFormat);
  config.bits_per_sample = CAEUtil::DataFormatToUsedBits(m_sourceFormat.m_dataFormat);
  config.dither_bits = CAEUtil::DataFormatToDitherBits(m_sourceFormat.m_dataFormat);
  config.channel_layout = CAEUtil::GetAVChannelLayout(m_sourceFormat.m_channelLayout);
  config.channels = m_sourceFormat.m_channelLayout.Count();
  config.sample_rate = m_sourceFormat.m_sampleRate;

  // init sample of silence/noise
  delete m_sampleOfSilence.pkt;
  m_sampleOfSilence.pkt = new CSoundPacket(config, m_sourceFormat.m_frames);
  m_sampleOfSilence.pkt->nb_samples = m_sampleOfSilence.pkt->max_nb_samples;
  if (!passthrough)
    GenerateNoise();

  m_swapState = CHECK_SWAP;
}

void CActiveAESource::ReturnBuffers()
{
  Message *msg = NULL;
  CSampleBuffer *samples;
  while (m_dataPort.ReceiveOutMessage(&msg))
  {
    if (msg->signal == CSourceDataProtocol::SAMPLE)
    {
      samples = *((CSampleBuffer**)msg->data);
      msg->Reply(CSourceDataProtocol::RETURNSAMPLE, &samples, sizeof(CSampleBuffer*));
    }
  }
}

unsigned int CActiveAESource::InputSamples(CSampleBuffer* samples)
{
  uint8_t **buffer = samples->pkt->data;
  unsigned int frames = samples->pkt->nb_samples;
  unsigned int maxFrames;
  int retry = 0;
  unsigned int read = 0;

  switch(m_swapState)
  {
  case SKIP_SWAP:
    break;
  case NEED_BYTESWAP:
    Endian_Swap16_buf((uint16_t *)buffer[0], (uint16_t *)buffer[0], frames * samples->pkt->config.channels);
    break;
  case CHECK_SWAP:
    SwapInit(samples);
    if (m_swapState == NEED_BYTESWAP)
      Endian_Swap16_buf((uint16_t *)buffer[0], (uint16_t *)buffer[0], frames * samples->pkt->config.channels);
    break;
  default:
    break;
  }

  AEDelayStatus status;

  while(frames > 0)
  {
    maxFrames = std::min(frames, m_sourceFormat.m_frames);
    read = m_source->GetPackets(buffer, maxFrames, samples->pkt->nb_samples-frames);
    if (read == 0)
    {
      Sleep(500*m_sourceFormat.m_frames/m_sourceFormat.m_sampleRate);
      retry++;
      if (retry > 4)
      {
        m_extError = true;
        CLog::Log(LOGERROR, "CActiveAESource::InputSamples - failed");
        status.SetDelay(0);
        m_stats->UpdateSourceDelay(status, frames, 0);
        return 0;
      }
      else
        continue;
    }
    else if (read > maxFrames)
    {
      m_extError = true;
      CLog::Log(LOGERROR, "CActiveAESource::InputSamples - source returned error");
      status.SetDelay(0);
      m_stats->UpdateSourceDelay(status, samples->pool ? maxFrames : 0, 0);
      return 0;
    }
    frames -= read;

    m_source->GetDelay(status);

    int64_t pts = 0;
    if (samples->timestamp)
    {
      int pastSamples = samples->pkt->nb_samples - samples->pkt_start_offset;
      pts = samples->timestamp + pastSamples/m_sourceFormat.m_sampleRate*1000;
      pts -= m_sourceLatency;
      if (pts < 0)
        pts = 0;
    }
    m_stats->UpdateSourceDelay(status, samples->pool ? read : 0, pts, samples->clockId);
  }
  return status.delay * 1000;
}

void CActiveAESource::SwapInit(CSampleBuffer* samples)
{
  if (AE_IS_RAW(m_requestedFormat.m_dataFormat) && CAEUtil::S16NeedsByteSwap(AE_FMT_S16NE, m_sourceFormat.m_dataFormat))
  {
    m_swapState = NEED_BYTESWAP;
  }
  else
    m_swapState = SKIP_SWAP;
}

#define PI 3.1415926536f

void CActiveAESource::GenerateNoise()
{
  int nb_floats = m_sampleOfSilence.pkt->max_nb_samples;
  nb_floats *= m_sampleOfSilence.pkt->config.channels;

  float *noise = (float*)_aligned_malloc(nb_floats*sizeof(float), 16);
  if (!noise)
    throw std::bad_alloc();

  float R1, R2;
  for(int i=0; i<nb_floats;i++)
  {
    do
    {
      R1 = (float) rand() / (float) RAND_MAX;
      R2 = (float) rand() / (float) RAND_MAX;
    }
    while(R1 == 0.0f);
    
    noise[i] = (float) sqrt( -2.0f * log( R1 )) * cos( 2.0f * PI * R2 ) * 0.00001f;
  }

  SampleConfig config = m_sampleOfSilence.pkt->config;
  IAEResample *resampler = CAEResampleFactory::Create(AERESAMPLEFACTORY_QUICK_RESAMPLE);
  resampler->Init(config.channel_layout,
                 config.channels,
                 config.sample_rate,
                 config.fmt,
                 config.bits_per_sample,
                 config.dither_bits,
                 config.channel_layout,
                 config.channels,
                 config.sample_rate,
                 AV_SAMPLE_FMT_FLT,
                 CAEUtil::DataFormatToUsedBits(m_sourceFormat.m_dataFormat),
                 CAEUtil::DataFormatToDitherBits(m_sourceFormat.m_dataFormat),
                 false, false, NULL, AE_QUALITY_UNKNOWN, false);
  resampler->Resample(m_sampleOfSilence.pkt->data, m_sampleOfSilence.pkt->max_nb_samples,
                     (uint8_t**)&noise, m_sampleOfSilence.pkt->max_nb_samples, 1.0);

  _aligned_free(noise);
  delete resampler;
}

void CActiveAESource::SetSilenceTimer()
{
  if (m_extStreaming)
    m_extSilenceTimeout = XbmcThreads::EndTime::InfiniteValue;
  else if (m_extAppFocused)
    m_extSilenceTimeout = CSettings::Get().GetInt("audiooutput.streamsilence") * 60000;
  else
    m_extSilenceTimeout = 0;
  m_extSilenceTimer.Set(m_extSilenceTimeout);
}
