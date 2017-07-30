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

#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPObject.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPTypes.h"

#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"

namespace DSP
{
namespace AUDIO
{
class IADSPNode : public DSPObject
{
public:
  IADSPNode(std::string Name, uint64_t ID) :
    DSPObject(Name, ID, DSP_CATEGORY_Audio)
  {
    m_InputFormat.m_dataFormat = AE_FMT_INVALID;
    m_InputFormat.m_channelLayout.Reset();

    m_OutputFormat.m_dataFormat = AE_FMT_INVALID;
    m_OutputFormat.m_channelLayout.Reset();

    m_processingBuffers = nullptr;
  }

  virtual DSPErrorCode_t Create() = 0;
  virtual bool Process() = 0;
  virtual DSPErrorCode_t Destroy() = 0;

  virtual const AEAudioFormat& GetInputFormat()  { return m_InputFormat;  }
  virtual const AEAudioFormat& GetOutputFormat() { return m_OutputFormat; }

  std::deque<ActiveAE::CSampleBuffer*> m_outputSamples;
  std::deque<ActiveAE::CSampleBuffer*> m_inputSamples;
  ActiveAE::CActiveAEBufferPool *m_processingBuffers;

protected:
  AEAudioFormat m_InputFormat;
  AEAudioFormat m_OutputFormat;
};
}
}
