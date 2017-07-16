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



#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/CopyMode/AudioDSPCopyMode.h"

using namespace DSP;
using namespace DSP::AUDIO;


namespace ActiveAE
{
CAudioDSPCopyModeCreator::CAudioDSPCopyModeCreator()
{
}

IADSPNode *CAudioDSPCopyModeCreator::InstantiateNode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, const AEStreamProperties &StreamProperties, unsigned int StreamID, uint64_t ID)
{
  CAudioDSPCopyMode *copyMode = new CAudioDSPCopyMode(InputFormat, OutputFormat, ID);
  IADSPNode *node = dynamic_cast<IADSPNode*>(copyMode);

  if (!node)
  {
    delete copyMode;
  }

  return node;
}

DSPErrorCode_t CAudioDSPCopyModeCreator::DestroyNode(IADSPNode *&Node)
{
  DSPErrorCode_t err = DSP_ERR_INVALID_INPUT;
  if (Node)
  {
    err = Node->Destroy();

    delete Node;
    Node = nullptr;
  }

  return err;
}


CAudioDSPCopyMode::CAudioDSPCopyMode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, uint64_t ID) :
  IADSPBufferNode("CAudioDSPCopyMode", ID)
{
  m_InputFormat = InputFormat;
  m_OutputFormat = OutputFormat;
}

DSPErrorCode_t CAudioDSPCopyMode::CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat)
{
  InputFormat.m_dataFormat = AE_FMT_FLOATP;
  OutputFormat.m_dataFormat = AE_FMT_FLOATP;
  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPCopyMode::DestroyInstance()
{
  m_InputFormat.m_channelLayout.Reset();
  return DSP_ERR_NO_ERR;
}

int CAudioDSPCopyMode::ProcessInstance(const uint8_t **In, uint8_t **Out)
{
  if (m_InputFormat.m_dataFormat == m_OutputFormat.m_dataFormat)
  {
    for (uint8_t ch = 0; ch < m_InputFormat.m_channelLayout.Count(); ch++)
    {
      for (uint32_t ii = 0; ii < m_InputFormat.m_frames * m_InputFormat.m_frameSize / m_InputFormat.m_channelLayout.Count(); ii++)
      {
        Out[ch][ii] = In[ch][ii];
      }
    }
  }

  return m_InputFormat.m_frames;
}
}
