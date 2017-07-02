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

#include "cores/AudioEngine/Utils/AEUtil.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioConverter/AudioDSPConverterFFMPEG.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioConverter/AudioConverterModel.h"
#include "cores/AudioEngine/AEResampleFactory.h"
#include "utils/log.h"

extern "C" {
#include "libavutil/channel_layout.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
}

using namespace ActiveAE;
using namespace DSP;
using namespace DSP::AUDIO;


CAudioDSPConverter::CAudioDSPConverter(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, uint64_t ID, CAudioConverterModel &Model) :
  IADSPNode("CAudioDSPConverter", ID),
  m_model(Model)
{
  m_needsSettingsUpdate = false;
  m_forceResampling = false;
  m_remapLayoutUsed = false;
  m_resampleRatio = 1.0;
  
  m_lastSamplePts = 0;

  m_resampler = nullptr;

  m_InputFormat = InputFormat;
  m_OutputFormat = OutputFormat;
}

CAudioDSPConverter::~CAudioDSPConverter()
{
}

DSPErrorCode_t CAudioDSPConverter::Create()
{
  if (m_InputFormat.m_dataFormat <= AE_FMT_INVALID || m_InputFormat.m_dataFormat >= AE_FMT_MAX || m_InputFormat.m_dataFormat == AE_FMT_RAW)
  {
    return DSP_ERR_INVALID_DATA_FORMAT;
  }

  m_needsSettingsUpdate = true;
  if (!UpdateSettings())
  {
    return DSP_ERR_FATAL_ERROR;
  }

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPConverter::Destroy()
{
  m_needsSettingsUpdate = false;

  delete m_resampler;

  return DSP_ERR_NO_ERR;
}

bool CAudioDSPConverter::Process()
{
  if (m_needsSettingsUpdate)
  {
    UpdateSettings();
  }

  bool busy = false;
  ActiveAE::CSampleBuffer *out = nullptr;
  ActiveAE::CSampleBuffer *in = nullptr;

  if (!m_resampler)
  {
    while (!m_inputSamples.empty())
    {
      in = m_inputSamples.front();
      m_inputSamples.pop_front();
      m_outputSamples.push_back(in);
      busy = true;
    }
  }
  else
  {

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

      for (int ch = 0; ch < m_planes.size(); ch++)
      {
        m_planes.at(ch) = in->pkt->data[0] + ch * m_InputFormat.m_frames * CAEUtil::DataFormatToBits(m_InputFormat.m_dataFormat) / 8;
      }
      uint8_t **inputPlanes = m_planes.data();
      if (!inputPlanes)
      {
        inputPlanes = in->pkt->data;
      }

      int out_samples = m_resampler->Resample(out->pkt->data, m_OutputFormat.m_frames, inputPlanes, m_InputFormat.m_frames, m_resampleRatio);
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
      int bufferedSamples = m_resampler->GetBufferedSamples();
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
  }

  return busy;
}

bool CAudioDSPConverter::UpdateSettings()
{
  if (!m_needsSettingsUpdate)
  {
    return false;
  }

  if (m_resampler)
  {
    delete m_resampler;
  }

  if (!m_model.StereoUpmix() && m_InputFormat.m_channelLayout.Count() == 2)
  {
    m_OutputFormat.m_channelLayout = m_InputFormat.m_channelLayout;
  }

  if (m_InputFormat.m_channelLayout != m_OutputFormat.m_channelLayout ||
      m_InputFormat.m_sampleRate    != m_OutputFormat.m_sampleRate ||
      m_InputFormat.m_dataFormat    != m_OutputFormat.m_dataFormat)
  {
    m_resampler = CAEResampleFactory::Create();
    if (!m_resampler)
    {
      return false;
    }

    CAEChannelInfo *remapLayout = m_remapLayoutUsed ? &m_InputFormat.m_channelLayout : NULL;

    if (!m_resampler->Init( CAEUtil::GetAVChannelLayout(m_OutputFormat.m_channelLayout),
                            m_OutputFormat.m_channelLayout.Count(),
                            m_OutputFormat.m_sampleRate,
                            CAEUtil::GetAVSampleFormat(m_OutputFormat.m_dataFormat),
                            CAEUtil::DataFormatToUsedBits(m_OutputFormat.m_dataFormat),
                            CAEUtil::DataFormatToDitherBits(m_OutputFormat.m_dataFormat),
                            CAEUtil::GetAVChannelLayout(m_InputFormat.m_channelLayout),
                            m_InputFormat.m_channelLayout.Count(),
                            m_InputFormat.m_sampleRate,
                            CAEUtil::GetAVSampleFormat(m_InputFormat.m_dataFormat),
                            CAEUtil::DataFormatToUsedBits(m_InputFormat.m_dataFormat),
                            CAEUtil::DataFormatToDitherBits(m_InputFormat.m_dataFormat),
                            m_model.StereoUpmix(),
                            m_model.NormalizeLevels(),
                            remapLayout,
                            m_model.ResampleQuality(),
                            m_forceResampling))
    {
      return false;
    }

    m_resampleRatio = m_OutputFormat.m_sampleRate / m_InputFormat.m_sampleRate;
    if (!AE_IS_PLANAR(m_InputFormat.m_dataFormat))
    {
      m_planes.resize(m_InputFormat.m_channelLayout.Count());
    }
  }

  m_needsSettingsUpdate = false;

  return true;
}

void CAudioDSPConverter::AudioConverterCallback()
{
  m_needsSettingsUpdate = true;
}
