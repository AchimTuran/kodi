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



#include <string>
#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNode.h"

namespace DSP
{
namespace AUDIO
{
class IADSPBufferNode : public IADSPNode
{
public:
  IADSPBufferNode(::std::string Name, uint64_t ID) :
    IADSPNode(Name, ID)
  {
    m_InputFormat.m_dataFormat = AE_FMT_INVALID;
    m_InputFormat.m_channelLayout.Reset();

    m_OutputFormat.m_dataFormat = AE_FMT_INVALID;
    m_OutputFormat.m_channelLayout.Reset();

    m_lastSamplePts = 0;
  }

  virtual DSPErrorCode_t Create() override
  {
    if (m_InputFormat.m_dataFormat <= AE_FMT_INVALID || m_InputFormat.m_dataFormat >= AE_FMT_MAX || m_InputFormat.m_dataFormat == AE_FMT_RAW)
    {
      return DSP_ERR_INVALID_DATA_FORMAT;
    }

    DSPErrorCode_t err = CreateInstance(m_InputFormat, m_OutputFormat);
    if (err != DSP_ERR_NO_ERR)
    {
      return err;
    }

    return err;
  }

  virtual bool Process() override
  {
    bool busy = false;
    ActiveAE::CSampleBuffer *out = nullptr;
    ActiveAE::CSampleBuffer *in = nullptr;

    if (m_inputSamples.size() > 0)
    {
      in = m_inputSamples.front();
      m_inputSamples.pop_front();
      busy = true;
    }

    if (in /*|| skipInput || m_drain //! @todo AudioDSP V2 check if this is really need*/)
    {
      out = m_processingBuffers->GetFreeBuffer();
      if (!out)
      {
        m_inputSamples.push_front(in);
        return false;
      }

      const uint8_t **inP = (const uint8_t **)(in->pkt->data);
      int out_samples = ProcessInstance(inP, out->pkt->data);
      if (out_samples < 0)
      {
        out_samples = 0;
      }

      out->pkt->nb_samples += out_samples;
      busy = true;
      bool m_empty = out->pkt->nb_samples == 0;

      if (in->timestamp)
      {
        m_lastSamplePts = in->timestamp;
      }
      else
      {
        in->pkt_start_offset = 0;
      }

      // pts of last sample we added to the buffer
      m_lastSamplePts += (in->pkt->nb_samples - in->pkt_start_offset) * 1000 / m_OutputFormat.m_sampleRate;

      // calculate pts for last sample in out
      int bufferedSamples = 0;//! @todo AudioDSP V2 how to implement an interface for m_pTempoFilter->GetBufferedSamples()?
      out->pkt_start_offset = out->pkt->nb_samples;
      out->timestamp = m_lastSamplePts - bufferedSamples * 1000 / m_OutputFormat.m_sampleRate;

      if (in && m_empty)
      {
        if (out->pkt->nb_samples != 0)
        {
          // pad with zero
          int start = out->pkt->nb_samples *
                      out->pkt->bytes_per_sample *
                      out->pkt->config.channels /
                      out->pkt->planes;
          for (int ch = 0; ch < out->pkt->planes; ch++)
          {
            memset(out->pkt->data[ch] + start, 0, out->pkt->linesize - start);
          }
        }

        // check if draining is finished
        if (out->pkt->nb_samples == 0)
        {
          out->Return();
          busy = false;
        }
        else
        {
          m_outputSamples.push_back(out);
        }
      }
      // some methods like encode require completely filled packets
      else if (out->pkt->nb_samples == out->pkt->max_nb_samples)
      {
        m_outputSamples.push_back(out);
      }

      if (in)
      {
        in->Return();
      }
    }

    return busy;
  }
  
  virtual DSPErrorCode_t Destroy() override
  {
    return DestroyInstance(); 
  }

protected:
  virtual DSPErrorCode_t CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat) = 0;
  virtual int ProcessInstance(const uint8_t **In, uint8_t **Out) = 0;
  virtual DSPErrorCode_t DestroyInstance() = 0;

private:
  int64_t m_lastSamplePts;
};
}
}
