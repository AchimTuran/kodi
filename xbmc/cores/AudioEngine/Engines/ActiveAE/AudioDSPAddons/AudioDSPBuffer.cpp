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

#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/AudioDSPBuffer.h"
#include "cores/AudioEngine/Utils/AEUtil.h"

using namespace ActiveAE;


CAudioDSPBufferPool::CAudioDSPBufferPool(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat)
{
  m_processingBuffer = nullptr;
  m_inputFormat = InputFormat;
  m_outputFormat = OutputFormat;
  m_needsConversion = false;
  m_audioConverter = nullptr;
}

CAudioDSPBufferPool::~CAudioDSPBufferPool()
{
  CSampleBuffer *buffer;
  while (!m_allSamples.empty())
  {
    buffer = m_allSamples.front();
    m_allSamples.pop_front();
    delete buffer;
  }
}

bool CAudioDSPBufferPool::Create(unsigned int totaltime)
{
  if (m_outputFormat.m_dataFormat == AE_FMT_RAW)
  {
    return false;
  }

  if (!(m_inputFormat == m_outputFormat))
  {
    //! @todo AudioDSP V2 get audio converter
    m_audioConverter = nullptr;
    if(m_audioConverter)
      m_audioConverter->Create(m_inputFormat, m_outputFormat);

    CSampleBuffer *buffer;
    SampleConfig config;
    config.fmt = CAEUtil::GetAVSampleFormat(m_outputFormat.m_dataFormat);
    config.bits_per_sample = CAEUtil::DataFormatToUsedBits(m_outputFormat.m_dataFormat);
    config.dither_bits = CAEUtil::DataFormatToDitherBits(m_outputFormat.m_dataFormat);
    config.channels = m_outputFormat.m_channelLayout.Count();
    config.sample_rate = m_outputFormat.m_sampleRate;
    config.channel_layout = CAEUtil::GetAVChannelLayout(m_outputFormat.m_channelLayout);

    unsigned int time = 0;
    unsigned int buffertime = (m_outputFormat.m_frames*1000) / m_outputFormat.m_sampleRate;

    for(unsigned int ii = 0; ii < 5 || time < totaltime; ii++)
    {
      buffer = new CSampleBuffer();
      buffer->pool = this;
      buffer->pkt = new CSoundPacket(config, m_outputFormat.m_frames);

      m_allSamples.push_back(buffer);
      m_freeSamples.push_back(buffer);
      time += buffertime;
    }

    m_needsConversion = true;
  }

  return true;
}

bool CAudioDSPBufferPool::AddInputBuffer(CSampleBuffer *Buffer)
{
  if (!Buffer)
  {
    return false;
  }

  m_inputSamples.push_back(Buffer);

  return true;
}

CSampleBuffer *CAudioDSPBufferPool::GetFreeBuffer()
{ 
  CSampleBuffer* buf = nullptr;

  if (!m_freeSamples.empty())
  {
    buf = m_freeSamples.front();
    m_freeSamples.pop_front();
    buf->refCount = 1;
  }

  return buf;
}

void CAudioDSPBufferPool::ReturnBuffer(CSampleBuffer *Buffer)
{
  Buffer->pkt->nb_samples = 0;
  Buffer->pkt->pause_burst_ms = 0;
  m_freeSamples.push_back(Buffer);
}

CSampleBuffer* CAudioDSPBufferPool::GetBuffer()
{
  CSampleBuffer* buf = nullptr;

  if (m_needsConversion)
  {
    if (!m_processingBuffer)
    {
      m_processingBuffer = GetFreeBuffer();
    }

    if (!m_inputBuffer && !m_inputSamples.empty())
    {
      m_inputBuffer = m_inputSamples.front();
      m_inputSamples.pop_front();
    }

    if (m_inputBuffer && m_processingBuffer)
    {
      // do conversion
      // timestamp calculation
      m_processingBuffer->timestamp;
      m_processingBuffer->pkt->nb_samples;
      
      m_audioConverter->ProcessInstance(m_inputBuffer, m_processingBuffer);

      if (m_inputBuffer->pkt->processedSamples == m_inputBuffer->pkt->max_nb_samples)
      {
        m_inputBuffer->Return();
        m_inputBuffer = nullptr;
      }

      if (m_processingBuffer->pkt->nb_samples == m_processingBuffer->pkt->max_nb_samples)
      {
        buf = m_processingBuffer;
        m_processingBuffer = nullptr;
      }
    }
  }
  else
  {
    if (!m_inputSamples.empty())
    {
      buf = m_inputSamples.front();
      m_inputSamples.pop_front();
    }
  }

  return buf;
}

float CAudioDSPBufferPool::GetDelay()
{
  float delay = 0.0;
  std::deque<CSampleBuffer*>::iterator itBuf;

  for (itBuf = m_inputSamples.begin(); itBuf != m_inputSamples.end(); ++itBuf)
  {
    delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
  }

  if (m_processingBuffer)
  {
    delay += (float)m_processingBuffer->pkt->nb_samples / m_processingBuffer->pkt->config.sample_rate;
  }

  return delay;
}
