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


#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/ActiveAEProcessingBuffer.h"
#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPProcessor.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"

#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPController.h"

#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNodeFactory.h"
#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNodeModelCallback.h"
#include "cores/AudioEngine/Utils/AEStreamProperties.h"


namespace ActiveAE
{
class CActiveAudioDSP;

class CAudioDSPProcessingBuffer : public IActiveAEProcessingBuffer, private CActiveAEBufferPool, public DSP::IDSPNodeModelCallback
{
  friend class CActiveAudioDSP;

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

  class CAudioDSPModeHandle
  {
    CAudioDSPModeHandle() { m_mode = nullptr; m_buffer = nullptr; }
  public:
    CAudioDSPModeHandle(DSP::AUDIO::IADSPNode *Mode, CActiveAEBufferPoolResample *Buffer)
    {
      m_mode = Mode;
      m_buffer = Buffer;
    }

    CAudioDSPModeHandle(DSP::AUDIO::IADSPNode *Mode)
    {
      m_mode = Mode;
      m_buffer = nullptr;
    }

    DSP::AUDIO::IADSPNode *m_mode;
    CActiveAEBufferPoolResample *m_buffer;
  };

  typedef std::vector<CAudioDSPModeHandle> AudioDSPNodeChain_t;
  typedef std::vector<NodeBuffer_t> AudioDSPBuffers_t;


public:
  CAudioDSPProcessingBuffer(unsigned int ID, 
                            const AEStreamProperties &StreamProperties,
                            const AEAudioFormat &InputFormat, 
                            const AEAudioFormat &OutputFormat, 
                            CAudioDSPController &Controller,
                            DSP::IDSPNodeFactory &NodeFactory);

  virtual bool Create(unsigned int totaltime, bool ForceOutputFormat = false) override;
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

  const unsigned int m_streamID;

private:
  // node model callbacks
  virtual DSPErrorCode_t EnableNodeCallback(uint64_t ID, uint32_t Position = 0) override;
  virtual DSPErrorCode_t DisableNodeCallback(uint64_t ID) override;

  int64_t m_lastSamplePts;
  bool m_fillPackets;
  bool m_empty;
  bool m_drain;
  
  std::vector<int64_t> m_nodeTimings;

  AudioDSPNodeChain_t m_DSPNodeChain;

  CAudioDSPController &m_audioDSPController;
  DSP::IDSPNodeFactory &m_nodeFactory;
  
  AEStreamProperties m_streamProperties;

  DSP::IDSPNodeModel::CDSPNodeInfoQuery m_conversionModeID;
};
}
