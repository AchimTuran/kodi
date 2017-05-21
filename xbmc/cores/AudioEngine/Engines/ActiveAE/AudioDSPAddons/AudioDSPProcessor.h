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

#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/Interfaces/IADSPNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/Interfaces/IADSPProcessor.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/AudioDSPController.h"

#include "cores/DSP/Factory/Interfaces/IDSPNodeFactory.h"
#include "cores/DSP/Models/Interfaces/IDSPNodeModelCallback.h"

namespace ActiveAE
{
class CAudioDSPProcessor : public DSP::AUDIO::IADSPProcessor, public DSP::IDSPNodeModelCallback
{
  typedef struct NodeBuffer_t
  {
    uint8_t **buffer;
    int bytesPerSample;     // bytes per sample and per channel
    int planes;             // 1 for non planar formats, #channels for planar
    int samplesCount;       // number of frames used
    int maxSamplesCount;    // max number of frames this packet can hold
    int channels;

    NodeBuffer_t()
    {
      buffer = nullptr;
      bytesPerSample = 0;
      planes = 0;
      samplesCount = 0;
      maxSamplesCount = 0;
      channels = 0;
    }
  }NodeBuffer_t;
  typedef std::vector<DSP::AUDIO::IADSPNode*> AudioDSPNodeChain_t;
  typedef std::vector<NodeBuffer_t> AudioDSPBuffers_t;
public:
  CAudioDSPProcessor(CAudioDSPController &Controller, DSP::IDSPNodeFactory &NodeFactory);
  virtual ~CAudioDSPProcessor();

private: // private methods
  DSPErrorCode_t ReCreateNodeChain();

  //! @todo implement memory alignment (16 for SSE)
  void CreateBuffer(const AEAudioFormat& Format, NodeBuffer_t &Buffer);
  //! @todo implement memory alignment
  void FreeBuffer(NodeBuffer_t &Buffer);

private:
  // processor interface
  virtual DSPErrorCode_t Create(const AEAudioFormat *InFormat, AEAudioFormat *OutFormat) override;
  virtual DSPErrorCode_t Process(const ActiveAE::CSampleBuffer *In, ActiveAE::CSampleBuffer *Out) override;
  virtual DSPErrorCode_t Destroy() override;

  // node model callbacks
  virtual DSPErrorCode_t EnableNodeCallback(uint64_t ID, uint32_t Position = 0) override;
  virtual DSPErrorCode_t DisableNodeCallback(uint64_t ID) override;

  AudioDSPBuffers_t m_Buffers;
  AudioDSPNodeChain_t m_DSPNodeChain;
  AEAudioFormat m_InFormat;
  AEAudioFormat m_OutFormat;

  CAudioDSPController &m_AudioDSPController;
  DSP::IDSPNodeFactory &m_NodeFactory;
};
}
