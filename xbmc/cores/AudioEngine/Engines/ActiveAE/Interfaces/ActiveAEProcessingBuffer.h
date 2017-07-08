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

#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#include <queue>

namespace ActiveAE
{
class CSampleBuffer;

class IActiveAEProcessingBuffer
{
public:
  IActiveAEProcessingBuffer(const AEAudioFormat &inputFormat, const AEAudioFormat &outputFormat) : m_inputFormat(inputFormat), m_outputFormat(outputFormat) {}

  virtual bool Create(unsigned int totaltime, bool ForceOutputFormat = false) = 0;
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

}
