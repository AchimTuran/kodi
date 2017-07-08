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

#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#include "cores/AudioEngine/Interfaces/AE.h"
#include <deque>
#include <memory>

extern "C" {
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
}

namespace ActiveAE
{
class CActiveAudioDSP;

struct SampleConfig
{
  AVSampleFormat fmt;
  uint64_t channel_layout;
  int channels;
  int sample_rate;
  int bits_per_sample;
  int dither_bits;
};

/**
 * the variables here follow ffmpeg naming
 */
class CSoundPacket
{
public:
  CSoundPacket(SampleConfig conf, int samples);
  ~CSoundPacket();
  uint8_t **data;                        // array with pointers to planes of data
  SampleConfig config;
  int bytes_per_sample;                  // bytes per sample and per channel
  int linesize;                          // see ffmpeg, required for planar formats
  int planes;                            // 1 for non planar formats, #channels for planar
  int nb_samples;                        // number of frames used
  int processedSamples;                  // number of offset samples
  int max_nb_samples;                    // max number of frames this packet can hold
  int pause_burst_ms;
};

class CActiveAEBufferPool;

class IAEBufferPoolCallback;

class CSampleBuffer
{
public:
  CSampleBuffer();
  ~CSampleBuffer();
  CSampleBuffer *Acquire();
  void Return();
  CSoundPacket *pkt;
  IAEBufferPoolCallback *pool;
  int64_t timestamp;
  int pkt_start_offset;
  int refCount;
};

class IAEBufferPoolCallback
{
public:
  virtual void ReturnBuffer(CSampleBuffer *buffer) = 0;
};

class CActiveAEBufferPool : public IAEBufferPoolCallback
{
public:
  CActiveAEBufferPool(const AEAudioFormat& format);
  virtual ~CActiveAEBufferPool();
  virtual bool Create(unsigned int totaltime);
  CSampleBuffer *GetFreeBuffer();

  // IAEBufferPoolCallback
  virtual void ReturnBuffer(CSampleBuffer *buffer) override;
  
  AEAudioFormat m_format;
  std::deque<CSampleBuffer*> m_allSamples;
  std::deque<CSampleBuffer*> m_freeSamples;
};

class IAEResample;

#define MAX_BUFFER_PLANES 16

class CActiveAEBufferPoolResample : public CActiveAEBufferPool
{
public:
  CActiveAEBufferPoolResample(AEAudioFormat inputFormat, AEAudioFormat outputFormat);
  ~CActiveAEBufferPoolResample() override;
  
  // generic methods
  bool Create(unsigned int totaltime);
  bool ResampleBuffers(int64_t timestamp = 0);
  float GetDelay();
  void Flush();
  void SetDrain(bool drain);
  void FillBuffer();
  
  // specific methods
  void SetOutputSampleRate(unsigned int OutputSampleRate);
  void SetResampleRatio(double resampleRatio);
  double GetResampleRatio();

  void ConfigureResampler(bool normalizelevels, bool stereoUpmix, AEQuality quality);
  void RemapChannelLayout(bool remap);
  bool GetNormalize();
  void ForceResampler(bool forceResampler);
  
  // buffer input format
  AEAudioFormat m_inputFormat;
  
  // public buffers
  std::deque<CSampleBuffer*> m_inputSamples;
  std::deque<CSampleBuffer*> m_outputSamples;

protected:
  void ChangeResampler();

  uint8_t *m_planes[MAX_BUFFER_PLANES];  //! @todo use std::vector<uint8_t*>!
  bool m_empty;
  bool m_drain;
  int64_t m_lastSamplePts;
  CSampleBuffer *m_procSample;
  bool m_fillPackets;
  
  // resampler (processing node)
  IAEResample *m_resampler;
  
  // resampler options
  bool m_remap;
  double m_resampleRatio;
  bool m_normalize;
  bool m_changeResampler;
  bool m_forceResampler;
  AEQuality m_resampleQuality;
  bool m_stereoUpmix;
};

class CActiveAEFilter;

class CActiveAEBufferPoolAtempo : public CActiveAEBufferPool
{
public:
  CActiveAEBufferPoolAtempo(const AEAudioFormat& format);
  ~CActiveAEBufferPoolAtempo() override;
  bool Create(unsigned int totaltime) override;
  bool ProcessBuffers();
  float GetDelay();
  void Flush();
  void SetTempo(float tempo);
  float GetTempo();
  void FillBuffer();
  void SetDrain(bool drain);
  std::deque<CSampleBuffer*> m_inputSamples;
  std::deque<CSampleBuffer*> m_outputSamples;

protected:
  void ChangeFilter();
  std::unique_ptr<CActiveAEFilter> m_pTempoFilter;
  uint8_t *m_planes[16];
  CSampleBuffer *m_procSample;
  bool m_empty;
  bool m_drain;
  bool m_changeFilter;
  float m_tempo;
  int64_t m_lastSamplePts;
  bool m_fillPackets;
};
  
}
