#pragma once
/*
 *      Copyright (C) 2010-2017 Team Kodi
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

#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/Interfaces/IADSPNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ADSPTypedefs.h"
#include "cores/DSP/Typedefs/DSPTypedefs.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/KodiModes/AudioConverter/AudioConverterModel.h"

extern "C" {
#include "libavutil/samplefmt.h"
}

struct SwrContext;

namespace ActiveAE
{
class CAudioDSPConverterFFMPEG : public DSP::AUDIO::IADSPNode, public IAudioConverterNodeCallback
{
public:
  CAudioDSPConverterFFMPEG(uint64_t ID, CAudioConverterModel &Model);
  virtual ~CAudioDSPConverterFFMPEG();

protected:
  virtual DSPErrorCode_t CreateInstance(AEAudioFormat &InputProperties, AEAudioFormat &OutputProperties, void *Options = nullptr) override;
  virtual DSPErrorCode_t DestroyInstance() override;

  virtual DSPErrorCode_t ProcessInstance(void *In, void *Out) override;

private:
  bool Init(uint64_t dst_chan_layout, int dst_channels, int dst_rate, AVSampleFormat dst_fmt, int dst_bits, int dst_dither, uint64_t src_chan_layout, int src_channels, int src_rate, AVSampleFormat src_fmt, int src_bits, int src_dither, bool upmix, bool normalize, const CAEChannelInfo *remapLayout, AEQuality quality, bool force_resample);
  int Resample(uint8_t **dst_buffer, int dst_samples, uint8_t **src_buffer, int src_samples, double ratio);
  int64_t GetDelay(int64_t base);
  int GetBufferedSamples();
  bool WantsNewSamples(int samples) { return GetBufferedSamples() <= samples * 2; }
  int CalcDstSampleCount(int src_samples, int dst_rate, int src_rate);
  int GetSrcBufferSize(int samples);
  int GetDstBufferSize(int samples);

  bool m_loaded;
  bool m_doesResample;
  
  uint64_t m_src_chan_layout;
  uint64_t m_dst_chan_layout;
  int m_src_rate;
  int m_dst_rate;
  int m_src_channels;
  int m_dst_channels;
  AVSampleFormat m_src_fmt;
  AVSampleFormat m_dst_fmt;
  int m_src_bits;
  int m_dst_bits;
  int m_src_dither_bits;
  int m_dst_dither_bits;
  SwrContext *m_pContext;
  double m_rematrix[AE_CH_MAX][AE_CH_MAX];
  double m_resampleRatio;

  bool m_remapLayoutUsed;
  CAEChannelInfo m_remapLayout;
  bool m_forceResampling;
  
  // model and callback implementations
  bool m_needsSettingsUpdate;
  bool UpdateSettings();
  virtual void AudioConverterCallback() override;
  CAudioConverterModel &m_model;
  std::vector<uint8_t*> m_planes;
};

}
