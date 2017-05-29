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

#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/AudioDSPProcessingBuffer.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"
#include "utils/log.h"

using namespace ActiveAE;


CAudioDSPProcessingBuffer::CAudioDSPProcessingBuffer(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat) :
  IActiveAEProcessingBuffer(InputFormat, OutputFormat),
  CActiveAEBufferPool(OutputFormat)
{
  m_procSample = nullptr;
  m_processor = nullptr;
  m_changeProcessor = false;
}

bool CAudioDSPProcessingBuffer::Create(unsigned int totaltime)
{
  if (m_inputFormat.m_channelLayout != m_outputFormat.m_channelLayout ||
      m_inputFormat.m_sampleRate != m_outputFormat.m_sampleRate ||
      m_inputFormat.m_dataFormat != m_outputFormat.m_dataFormat)
  {
    ChangeProcessor();
  }

  DSPErrorCode_t dspErr = m_processor->Create(&m_inputFormat, &m_outputFormat);
  if(dspErr != DSP_ERR_NO_ERR)
  {
    CLog::Log(LOGERROR, "%s - failed to create AudioDSP processor with error: %i", __FUNCTION__, dspErr);

    return false;
  }


  m_format = m_outputFormat;
  //! @todo AudioDSP V2 is this needed?
  if(!CActiveAEBufferPool::Create(totaltime))
  {
    CLog::Log(LOGERROR, "%s - failed to create AudioDSP buffer pool!", __FUNCTION__);

    return false;
  }

  return true;
}

void CAudioDSPProcessingBuffer::Destroy()
{
}

bool CAudioDSPProcessingBuffer::ProcessBuffer()
{
  bool busy = false;
  CSampleBuffer *buffer;

  if (m_changeProcessor)
  {
    ChangeProcessor();
        
    busy = true;
  }

  static bool copyInput = false;
  if (!m_processor || copyInput)
  {   
    while (!m_inputSamples.empty())
    {
      m_outputSamples.push_back(m_inputSamples.front());
      m_inputSamples.pop_front();
      busy = true;
    }
  }
  else
  {
    while (!m_inputSamples.empty())
    {
      m_processor->m_inputSamples.push_back(m_inputSamples.front());
      m_inputSamples.pop_front();
      busy = true;
    }

    busy |= m_processor->ProcessBuffer();

    while (!m_processor->m_outputSamples.empty())
    {
      m_outputSamples.push_back(m_processor->m_outputSamples.front());
      m_processor->m_outputSamples.pop_front();
      busy = true;
    }
  }

  return busy;
}

bool CAudioDSPProcessingBuffer::HasInputLevel(int level)
{
  if (m_processor)
  {
    return m_processor->HasInputLevel(level);
  }
   
  return false;
}

float CAudioDSPProcessingBuffer::GetDelay()
{
  float delay = 0;
  std::deque<CSampleBuffer*>::iterator itBuf;

  if (m_procSample)
  {
    delay += (float)m_procSample->pkt->nb_samples / m_procSample->pkt->config.sample_rate;
  }

  for (itBuf = m_inputSamples.begin(); itBuf != m_inputSamples.end(); ++itBuf)
  {
    delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
  }

  for (itBuf = m_outputSamples.begin(); itBuf != m_outputSamples.end(); ++itBuf)
  {
    delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
  }

  if (m_processor)
  {
    for (itBuf = m_processor->m_inputSamples.begin(); itBuf != m_processor->m_inputSamples.end(); ++itBuf)
    {
      delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
    }

    for (itBuf = m_processor->m_outputSamples.begin(); itBuf != m_processor->m_outputSamples.end(); ++itBuf)
    {
      delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
    }

    delay += m_processor->GetDelay();
  }

  return delay;
}

void CAudioDSPProcessingBuffer::Flush()
{
  if (m_procSample)
  {
    m_procSample->Return();
    m_procSample = NULL;
  }
  
  while (!m_inputSamples.empty())
  {
    m_inputSamples.front()->Return();
    m_inputSamples.pop_front();
  }
  
  while (!m_outputSamples.empty())
  {
    m_outputSamples.front()->Return();
    m_outputSamples.pop_front();
  }

  if (m_processor)
  {
    m_changeProcessor = true;
    ChangeProcessor();
  }
}

void CAudioDSPProcessingBuffer::SetDrain(bool drain)
{
  m_drain = drain;
}

bool CAudioDSPProcessingBuffer::IsDrained()
{
  if (//! @todo AudioDSP V2 implement this /*m_processor->m_inputSamples.empty() &&*/
    !m_inputSamples.empty() ||
    !m_outputSamples.empty())

  {
    return false;
  }

  return true;
}

void CAudioDSPProcessingBuffer::FillBuffer()
{
  m_fillPackets = true;
}

bool CAudioDSPProcessingBuffer::HasWork()
{
  if (!m_inputSamples.empty())
    return true;
  if (!m_outputSamples.empty())
    return true;
  if (m_processor && m_processor->HasWork())
    return true;

  return false;
}

void CAudioDSPProcessingBuffer::SetOutputSampleRate(unsigned int OutputSampleRate)
{
  //! @todo AudioDSP V2 implement this
  //m_resampleRatio = (double)m_inputFormat.m_sampleRate / OutputSampleRate;
  m_outputFormat.m_sampleRate = OutputSampleRate;
}

void CAudioDSPProcessingBuffer::ChangeProcessor()
{
  if (m_processor)
  {
    m_processor->Destroy();
  }

  if (m_processor && m_changeProcessor)
  {
    m_processor->Create(&m_inputFormat, &m_outputFormat);
    m_changeProcessor = false;
  }
}