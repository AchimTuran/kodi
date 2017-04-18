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

#include "cores/AudioEngine/Interfaces/AEStream.h"
#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#include "cores/AudioEngine/Utils/AELimiter.h"
#include <atomic>

namespace ActiveAE
{
class CActiveAE;

class CSyncError
{
public:
  CSyncError()
  {
    Flush();
  }
  void Add(double error)
  {
    m_buffer += error;
    m_count++;
  }

  void Flush(int interval = 100)
  {
    m_buffer = 0.0f;
    m_lastError = 0.0;
    m_count  = 0;
    m_timer.Set(interval);
  }

  bool Get(double& error, int interval = 100)
  {
    if(m_timer.IsTimePast())
    {
      error = Get();
      Flush(interval);
      m_lastError = error;
      return true;
    }
    else
    {
      error = m_lastError;
      return false;
    }
  }

  double GetLastError(unsigned int &time)
  {
    time = m_timer.GetStartTime();
    return m_lastError;
  }

  void Correction(double correction)
  {
    m_lastError += correction;
  }

protected:
  double Get()
  {
    if(m_count)
      return m_buffer / m_count;
    else
      return 0.0;
  }
  double m_buffer;
  double m_lastError;
  int m_count;
  XbmcThreads::EndTime m_timer;
};


class IActiveAEProcessingBuffer
{
public:
  IActiveAEProcessingBuffer(const AEAudioFormat &inputFormat, const AEAudioFormat &outputFormat) : m_inputFormat(inputFormat), m_outputFormat(outputFormat) {}

  virtual bool Create(unsigned int totaltime) = 0;
  virtual void Destroy() = 0;
  virtual bool ProcessBuffer() = 0;
  virtual bool HasInputLevel(int level) = 0;
  virtual float GetDelay() = 0;
  virtual void Flush() = 0;
  virtual void SetDrain(bool drain) = 0;
  virtual bool IsDrained() = 0;
  virtual void FillBuffer() = 0;
  virtual bool HasWork() = 0;
  virtual void SetOutputSampleRate(unsigned int OutputSampleRate) = 0;

  AEAudioFormat m_inputFormat;
  AEAudioFormat m_outputFormat;
  std::deque<CSampleBuffer*> m_outputSamples;
  std::deque<CSampleBuffer*> m_inputSamples;
};


class CActiveAEStreamBuffers : public IActiveAEProcessingBuffer
{
public:
  CActiveAEStreamBuffers(AEAudioFormat inputFormat, AEAudioFormat outputFormat);
  virtual ~CActiveAEStreamBuffers();

  // IActiveAEProcessingBuffer interface methods
  virtual bool Create(unsigned int totaltime) override;
  virtual void Destroy() override {}
  virtual bool ProcessBuffer() override;
  virtual bool HasInputLevel(int level) override;
  virtual float GetDelay() override;
  virtual void Flush() override;
  virtual void SetDrain(bool drain) override;
  virtual bool IsDrained() override;
  virtual void FillBuffer() override;
  virtual bool HasWork() override;
  virtual void SetOutputSampleRate(unsigned int OutputSampleRate) override;
  
  // specific methods
  bool GetNormalize();
  void SetResampleRatio(double resampleRatio, double atempoThreshold);
  double GetResampleRatio();
  void ConfigureResampler(bool normalize, bool stereoUpmix, AEQuality quality);
  void SetExtraData(int profile, enum AVMatrixEncoding matrixEncoding, enum AVAudioServiceType audioServiceType);
  void ForceResampler(bool forceResampler);
  CActiveAEBufferPool *GetResampleBuffers();
  CActiveAEBufferPool *GetAtempoBuffers();

protected:
  CActiveAEBufferPoolResample *m_resampleBuffers;
  CActiveAEBufferPoolAtempo *m_atempoBuffers;
};

class CActiveAEStream : public IAEStream
{
protected:
  friend class CActiveAE;
  friend class CEngineStats;
  CActiveAEStream(AEAudioFormat *format, unsigned int streamid, CActiveAE &ae);
  virtual ~CActiveAEStream();
  void FadingFinished();
  void IncFreeBuffers();
  void DecFreeBuffers();
  void ResetFreeBuffers();
  void InitRemapper();
  void RemapBuffer();
  double CalcResampleRatio(double error);

public:
  // public interface methods
  virtual unsigned int GetSpace();
  virtual unsigned int AddData(const uint8_t* const *data, unsigned int offset, unsigned int frames, double pts = 0.0) override;
  virtual double GetDelay() override;
  virtual CAESyncInfo GetSyncInfo();
  virtual bool IsBuffering();
  virtual double GetCacheTime();
  virtual double GetCacheTotal();

  virtual void Pause();
  virtual void Resume();
  virtual void Drain(bool wait);
  virtual bool IsDraining();
  virtual bool IsDrained();
  virtual void Flush();

  virtual float GetVolume()  override;
  virtual float GetReplayGain()  override;
  virtual float GetAmplification()  override;
  virtual void SetVolume(float volume)  override;
  virtual void SetReplayGain(float factor)  override;
  virtual void SetAmplification(float amplify)  override;
  virtual void SetFFmpegInfo(int profile, enum AVMatrixEncoding matrix_encoding, enum AVAudioServiceType audio_service_type) override;

  virtual const unsigned int GetFrameSize() const;
  virtual const unsigned int GetChannelCount() const;
  
  virtual const unsigned int GetSampleRate() const ;
  virtual const enum AEDataFormat GetDataFormat() const;
  
  virtual double GetResampleRatio();
  virtual void SetResampleRatio(double ratio);
  virtual void SetResampleMode(int mode);
  virtual void RegisterAudioCallback(IAudioCallback* pCallback);
  virtual void UnRegisterAudioCallback();
  virtual void FadeVolume(float from, float to, unsigned int time);
  virtual bool IsFading();
  virtual void RegisterSlave(IAEStream *stream);

protected:
  CActiveAE &m_activeAE;
  unsigned int m_id;
  AEAudioFormat m_format;
  float m_streamVolume;
  float m_streamRgain;
  float m_streamAmplify;
  double m_streamResampleRatio;
  int m_streamResampleMode;
  unsigned int m_streamSpace;
  bool m_streamDraining;
  bool m_streamDrained;
  bool m_streamFading;
  int m_streamFreeBuffers;
  bool m_streamIsBuffering;
  bool m_streamIsFlushed;
  IAEStream *m_streamSlave;
  CCriticalSection m_streamLock;
  CCriticalSection m_statsLock;
  uint8_t *m_leftoverBuffer;
  int m_leftoverBytes;
  CSampleBuffer *m_currentBuffer;
  CSoundPacket *m_remapBuffer;
  IAEResample *m_remapper;
  double m_lastPts;
  double m_lastPtsJump;
  std::atomic_int m_errorInterval;

  // only accessed by engine
  IActiveAEProcessingBuffer *m_processingBuffers;

  CActiveAEBufferPool *m_inputBuffers;
  std::deque<CSampleBuffer*> m_processingSamples;
  CActiveAEDataProtocol *m_streamPort;
  CEvent m_inMsgEvent;
  bool m_drain;
  bool m_paused;
  bool m_started;
  CAELimiter m_limiter;
  float m_volume;
  float m_rgain;
  float m_amplify;
  float m_bufferedTime;
  int m_fadingSamples;
  float m_fadingBase;
  float m_fadingTarget;
  int m_fadingTime;
  int m_profile;
  int m_resampleMode;
  double m_resampleIntegral;
  double m_clockSpeed;
  enum AVMatrixEncoding m_matrixEncoding;
  enum AVAudioServiceType m_audioServiceType;
  bool m_forceResampler;
  IAEClockCallback *m_pClock;
  CSyncError m_syncError;
  double m_lastSyncError;
  CAESyncInfo::AESyncState m_syncState;
};
}

