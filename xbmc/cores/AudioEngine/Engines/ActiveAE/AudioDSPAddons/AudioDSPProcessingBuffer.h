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

#pragma once

#include "cores/AudioEngine/Interfaces/IActiveAEProcessingBuffer.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/Interfaces/IADSPProcessor.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"


namespace ActiveAE
{
class CActiveAudioDSP;

class CAudioDSPProcessingBuffer : public IActiveAEProcessingBuffer, private CActiveAEBufferPool
{
  friend class CActiveAudioDSP;
public:
  CAudioDSPProcessingBuffer(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat);

  virtual bool Create(unsigned int totaltime) override;
  virtual void Destroy() override;
  virtual bool ProcessBuffer() override;
  virtual bool HasInputLevel(int level) override;
  virtual float GetDelay() override;
  virtual void Flush() override;
  virtual void SetDrain(bool drain) override;
  virtual bool IsDrained() override;
  virtual void FillBuffer() override;
  virtual bool HasWork() override;
  virtual void SetOutputSampleRate(unsigned int OutputSampleRate) override;

private:
  void ChangeProcessor();

  int64_t m_lastSamplePts;
  bool m_fillPackets;
  bool m_empty;
  bool m_drain;
  CSampleBuffer *m_procSample;
  
  bool m_changeProcessor;
  DSP::AUDIO::IADSPProcessor *m_processor;
};
}
