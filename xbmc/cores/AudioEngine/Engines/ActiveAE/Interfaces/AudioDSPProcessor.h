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

#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"
#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPTypes.h"

namespace DSP
{
namespace AUDIO
{
class IADSPProcessor
{
public:
  IADSPProcessor(std::string Name) : Name(Name) {}
  virtual ~IADSPProcessor() {}

  virtual DSPErrorCode_t Create(const AEAudioFormat *InFormat, AEAudioFormat *OutFormat) = 0;
  virtual bool ProcessBuffer() = 0;
  virtual DSPErrorCode_t Destroy() = 0;
  virtual float GetDelay() = 0;
  virtual bool HasInputLevel(int level) = 0;
  virtual bool HasWork() = 0;
  
  const std::string Name;

  std::deque<ActiveAE::CSampleBuffer*> m_outputSamples;
  std::deque<ActiveAE::CSampleBuffer*> m_inputSamples;
};
}
}
