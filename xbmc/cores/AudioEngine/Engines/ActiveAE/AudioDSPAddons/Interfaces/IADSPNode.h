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

#include "cores/DSP/DSPObject.h"
#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ADSPTypedefs.h"
#include "cores/AudioEngine/Utils/AEAudioFormat.h"

namespace DSP
{
namespace AUDIO
{
class IADSPNode : public DSPObject
{
public:
  IADSPNode(std::string Name, uint64_t ID, ADSPDataFormatFlags_t FormatFlags) :
    DSPObject(Name, ID, DSP_CATEGORY_Audio),
    FormatFlags(FormatFlags)
  {
    m_InputFormat.m_dataFormat = AE_FMT_INVALID;
    m_InputFormat.m_channelLayout.Reset();

    m_OutputFormat.m_dataFormat = AE_FMT_INVALID;
    m_OutputFormat.m_channelLayout.Reset();
  }

  const ADSPDataFormatFlags_t FormatFlags;

  virtual DSPErrorCode_t Create(const AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat, void *Options = nullptr)
  {
    if (InputFormat.m_dataFormat == AE_FMT_INVALID || InputFormat.m_dataFormat == AE_FMT_MAX || InputFormat.m_dataFormat == AE_FMT_RAW)
    {
      return DSP_ERR_INVALID_DATA_FORMAT;
    }

    // the default behavoiur is that the node uses the same output parameters as the input parameters
    m_InputFormat = InputFormat;
    m_OutputFormat = OutputFormat;

    DSPErrorCode_t err = CreateInstance(m_InputFormat, m_OutputFormat, Options);

    return err;
  }

  virtual DSPErrorCode_t ProcessInstance(void *In, void *Out) = 0;
  virtual DSPErrorCode_t DestroyInstance() = 0;

  virtual const AEAudioFormat& GetInputFormat()  { return m_InputFormat;  }
  virtual const AEAudioFormat& GetOutputFormat() { return m_OutputFormat; }

protected:
  virtual DSPErrorCode_t CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat, void *Options = nullptr) = 0;

  AEAudioFormat m_InputFormat;
  AEAudioFormat m_OutputFormat;
};
}
}
