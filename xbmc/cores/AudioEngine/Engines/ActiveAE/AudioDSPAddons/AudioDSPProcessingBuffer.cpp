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
  CSampleBuffer *in;
  int timestamp = 0;

  if (!m_processor)
  {
    if (m_changeProcessor)
    {
      if (m_changeProcessor)
      {
        ChangeProcessor();
      }

      return true;
    }

    while (!m_inputSamples.empty())
    {
      in = m_inputSamples.front();
      m_inputSamples.pop_front();
      if (timestamp)
      {
        in->timestamp = timestamp;
      }
      m_outputSamples.push_back(in);
      busy = true;
    }
  }
  else if (m_procSample || !m_freeSamples.empty())
  {
    int free_samples;
    if (m_procSample)
      free_samples = m_procSample->pkt->max_nb_samples - m_procSample->pkt->nb_samples;
    else
      free_samples = m_outputFormat.m_frames;

    //! @todo AudioDSP is this needed?
    //bool skipInput = false;
    // avoid that ffmpeg resample buffer grows too large
    //if (!m_resampler->WantsNewSamples(free_samples) && !m_empty)
    //  skipInput = true;

    bool hasInput = !m_inputSamples.empty();

    if (hasInput || /*skipInput ||*/ m_drain || m_changeProcessor)
    {
      if (!m_procSample)
      {
        m_procSample = GetFreeBuffer();
      }

      if (hasInput && /*!skipInput &&*/ !m_changeProcessor)
      {
        in = m_inputSamples.front();
        m_inputSamples.pop_front();
      }
      else
        in = nullptr;

      {
      }

      int out_samples = in ? in->pkt->nb_samples : 0;
      DSPErrorCode_t dspErr = m_processor->Process(in, m_procSample);

      //! @todo AudioDSP V2 implement the processor here.
      //m_resampler->Resample(m_planes,
      //                      m_procSample->pkt->max_nb_samples - m_procSample->pkt->nb_samples,
      //                      in ? in->pkt->data : NULL,
      //                      in ? in->pkt->nb_samples : 0,
      //                      m_resampleRatio);
      // in case of error, trigger re-create of resampler

      busy = true;
      m_changeProcessor = dspErr != DSP_ERR_NO_ERR;
      m_empty = (dspErr != DSP_ERR_NO_ERR);

      if (in)
      {
        if (!timestamp)
        {
          if (in->timestamp)
            m_lastSamplePts = in->timestamp;
          else
            in->pkt_start_offset = 0;
        }
        else
        {
          m_lastSamplePts = timestamp;
          in->pkt_start_offset = 0;
        }

        // pts of last sample we added to the buffer
        m_lastSamplePts += (in->pkt->nb_samples - in->pkt_start_offset) * 1000 / m_outputFormat.m_sampleRate;

        in->Return();
      }

      // calculate pts for last sample in m_procSample
      int bufferedSamples = 0;//! @todo AudioDSP V2 implement this m_resampler->GetBufferedSamples();
      m_procSample->pkt_start_offset = m_procSample->pkt->nb_samples;
      m_procSample->timestamp = m_lastSamplePts - bufferedSamples * 1000 / m_outputFormat.m_sampleRate;

      if ((m_drain || m_changeProcessor) && m_empty)
      {
        if (m_fillPackets && m_procSample->pkt->nb_samples != 0)
        {
          // pad with zero
          start = m_procSample->pkt->nb_samples *
            m_procSample->pkt->bytes_per_sample *
            m_procSample->pkt->config.channels /
            m_procSample->pkt->planes;
          for (int i = 0; i<m_procSample->pkt->planes; i++)
          {
            memset(m_procSample->pkt->data[i] + start, 0, m_procSample->pkt->linesize - start);
          }
        }

        // check if draining is finished
        if (m_drain && m_procSample->pkt->nb_samples == 0)
        {
          m_procSample->Return();
          busy = false;
        }
        else
          m_outputSamples.push_back(m_procSample);

        m_procSample = NULL;
        if (m_changeProcessor)
          ChangeProcessor();
      }
      // some methods like encode require completely filled packets
      else if (!m_fillPackets || (m_procSample->pkt->nb_samples == m_procSample->pkt->max_nb_samples))
      {
        m_outputSamples.push_back(m_procSample);
        m_procSample = NULL;
      }
    }
  }
  return busy;
}

bool CAudioDSPProcessingBuffer::HasInputLevel(int level)
{
  if ((m_inputSamples.size()) >
    (this->m_allSamples.size() * level / 100))
    return true;
  else
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
    int samples = 0;//! @todo AudioDSP V2 implement this m_processor->GetBufferedSamples().
    delay += (float)samples / m_outputFormat.m_sampleRate;
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
  //! @todo AudioDSP V2 implement this.
  //if (!m_processor->m_inputSamples.empty())
  //  return true;
  //if (!m_processor->m_outputSamples.empty())
  //  return true;

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
  if (m_changeProcessor || !m_processor)
  {
    m_processor->Destroy();
  }
}
