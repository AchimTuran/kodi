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

#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPProcessingBuffer.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPNodeModel.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

using namespace ActiveAE;
using namespace DSP;
using namespace DSP::AUDIO;


CAudioDSPProcessingBuffer::CAudioDSPProcessingBuffer(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, CAudioDSPController &Controller, IDSPNodeFactory &NodeFactory) :
  IActiveAEProcessingBuffer(InputFormat, OutputFormat),
  CActiveAEBufferPool(OutputFormat),
  m_AudioDSPController(Controller),
  m_NodeFactory(NodeFactory),
  m_conversionModeID({ "Kodi", "AudioConverter" })
{
}

bool CAudioDSPProcessingBuffer::Create(unsigned int totaltime, bool ForceOutputFormat)
{
  IDSPNodeModel::DSPNodeInfoVector_t nodeInfos;
  DSPErrorCode_t dspErr = m_AudioDSPController.GetActiveNodes(nodeInfos);
  if (dspErr != DSP_ERR_NO_ERR)
  {
    return false;
  }

  AEAudioFormat tmpParameters[2];
  AEAudioFormat *configInParameters = &tmpParameters[0];
  AEAudioFormat *configOutParameters = &tmpParameters[1];

  *configInParameters = m_inputFormat;
  *configOutParameters = m_outputFormat;

  // create node chain
  for (uint32_t ii = 0; ii < nodeInfos.size(); ii++)
  {
    IADSPNode *node = m_NodeFactory.InstantiateNode(nodeInfos.at(ii).ID);
    if (!node)
    {
      return DSP_ERR_FATAL_ERROR;
    }
    DSPErrorCode_t dspErr = node->Create(*configInParameters, *configOutParameters);
    if (dspErr != DSP_ERR_NO_ERR)
    {
      m_NodeFactory.DestroyNode(node);
      return dspErr;
    }

    IADSPNode *adspNode = dynamic_cast<IADSPNode*>(node);
    if (!adspNode)
    {
      m_NodeFactory.DestroyNode(node);
      return DSP_ERR_FATAL_ERROR;
    }

    // swap pointer for parameters
    AEAudioFormat *p = configInParameters;
    configInParameters = configOutParameters;
    configOutParameters = p;
    // the default behaviour is to set the same output as input configuration
    *configOutParameters = *configInParameters;

    m_DSPNodeChain.push_back(CAudioDSPModeHandle(adspNode, nullptr));
  }

  // configure buffers
  if (m_DSPNodeChain.size() == 0)
  {
    IDSPNodeModel::CDSPNodeInfoQuery query({ "Kodi", "AudioConverter" });
    IDSPNodeModel::CDSPNodeInfo audioConverterInfo = m_AudioDSPController.GetNodeInfo(query);
    IADSPNode *audioConverter = dynamic_cast<IADSPNode*>(m_NodeFactory.InstantiateNode(audioConverterInfo.ID));
    if (!audioConverter)
    {
      return DSP_ERR_INVALID_NODE_ID;
    }
    DSPErrorCode_t dspErr = audioConverter->Create(m_inputFormat, m_outputFormat);
    if (dspErr != DSP_ERR_NO_ERR)
    {
      IADSPNode *node = dynamic_cast<IADSPNode*>(audioConverter);
      m_NodeFactory.DestroyNode(node);
      return dspErr;
    }

    m_outputFormat = audioConverter->GetOutputFormat();

    m_DSPNodeChain.push_back(CAudioDSPModeHandle(audioConverter, nullptr));
  }
  else
  {
    if (m_DSPNodeChain.size() == 1)
    {
      AEAudioFormat inFmt = m_DSPNodeChain.at(0).m_mode->GetInputFormat();
      if (!(inFmt == m_inputFormat))
      { // create a output conversion buffer
        //! @todo add buffer
      }

      m_outputFormat = m_DSPNodeChain.at(0).m_mode->GetOutputFormat();
    }
    else
    {
      AEAudioFormat inFmt = m_inputFormat;
      for (uint32_t ii = 0; ii < m_DSPNodeChain.size(); ii++)
      {
        AEAudioFormat outFmt;
        outFmt = m_DSPNodeChain.at(ii).m_mode->GetInputFormat();

        if (!(inFmt == outFmt))
        { // create conversion buffer
          //! @todo add buffer
        }

        inFmt = m_DSPNodeChain.at(ii).m_mode->GetOutputFormat();
      }

      m_outputFormat = inFmt;
    }

    // add audio converter if the first mode needed a different input format
    AEAudioFormat firstModeInputFormat = m_DSPNodeChain.front().m_mode->GetInputFormat();
    if (!(firstModeInputFormat == m_inputFormat))
    {
      IDSPNodeModel::CDSPNodeInfoQuery query({ "Kodi", "AudioConverter" });
      IDSPNodeModel::CDSPNodeInfo audioConverterInfo = m_AudioDSPController.GetNodeInfo(query);
      IADSPNode *audioConverter = dynamic_cast<IADSPNode*>(m_NodeFactory.InstantiateNode(audioConverterInfo.ID));
      if (!audioConverter)
      {
        return DSP_ERR_INVALID_NODE_ID;
      }
      DSPErrorCode_t dspErr = audioConverter->Create(m_inputFormat, firstModeInputFormat);
      if (dspErr != DSP_ERR_NO_ERR)
      {
        IADSPNode *node = dynamic_cast<IADSPNode*>(audioConverter);
        m_NodeFactory.DestroyNode(node);
        return dspErr;
      }

      AudioDSPNodeChain_t::iterator nodeIter = m_DSPNodeChain.begin();
      m_DSPNodeChain.insert(nodeIter, CAudioDSPModeHandle(audioConverter, nullptr));
    }

    AEAudioFormat lastModeOutputFormat = m_DSPNodeChain.back().m_mode->GetOutputFormat();
    if (!(lastModeOutputFormat == m_outputFormat))
    {
      IDSPNodeModel::CDSPNodeInfoQuery query({ "Kodi", "AudioConverter" });
      IDSPNodeModel::CDSPNodeInfo audioConverterInfo = m_AudioDSPController.GetNodeInfo(query);
      IADSPNode *audioConverter = dynamic_cast<IADSPNode*>(m_NodeFactory.InstantiateNode(audioConverterInfo.ID));
      if (!audioConverter)
      {
        return DSP_ERR_INVALID_NODE_ID;
      }
      DSPErrorCode_t dspErr = audioConverter->Create(lastModeOutputFormat, m_outputFormat);
      if (dspErr != DSP_ERR_NO_ERR)
      {
        IADSPNode *node = dynamic_cast<IADSPNode*>(audioConverter);
        m_NodeFactory.DestroyNode(node);
        return dspErr;
      }

      m_DSPNodeChain.push_back(CAudioDSPModeHandle(audioConverter, nullptr));
    }
  }

  // initialize internal format with all available ActiveAE channels
  CAEChannelInfo audioDSPChLayout;
  //! @todo AudioDSP V3 add support for all AE channels, this requires to improve the implemented FFMPEG based AudioConversion Mode
  for (int ch = AE_CH_FL; ch < AE_CH_TBL; ch++)
  {
    if (m_inputFormat.m_channelLayout.HasChannel(static_cast<AEChannel>(ch)))
    {
      audioDSPChLayout += static_cast<AEChannel>(ch);
    }
  }

  // create buffers
  *configInParameters = m_inputFormat;
  for (unsigned int ii = 0; ii < m_DSPNodeChain.size(); ii++)
  {
    const AEAudioFormat &nodeOutFormat = m_DSPNodeChain.at(ii).m_mode->GetOutputFormat();

    AEAudioFormat bufferFormat = m_DSPNodeChain.at(ii).m_mode->GetInputFormat();
    bufferFormat.m_channelLayout = audioDSPChLayout;

    //! @todo AudioDSP V3 implement the class CAudioDSPConversionBufferPool
    m_DSPNodeChain.at(ii).m_buffer = new CActiveAEBufferPoolResample(*configInParameters, bufferFormat);
    m_DSPNodeChain.at(ii).m_buffer->Create(400); //! @todo AudioDSP V2 use define from AE

    if (!m_DSPNodeChain.at(ii).m_mode->m_processingBuffers)
    {
      bufferFormat = nodeOutFormat;
      bufferFormat.m_channelLayout = audioDSPChLayout;
      m_DSPNodeChain.at(ii).m_mode->m_processingBuffers = new CActiveAEBufferPool(nodeOutFormat);
      m_DSPNodeChain.at(ii).m_mode->m_processingBuffers->Create(400); //! @todo AudioDSP V2 use define from AE
    }

    //! @todo AudioDSP V3 add a conversion mode through m_conversionModeID
    //! @todo AudioDSP V3 also add a fallback to Kodi::AudioConverter mode if the configured default mode doesn't support the requested formats

    // set input format for the next node
    *configInParameters = nodeOutFormat;
  }

  if (!ForceOutputFormat)
  {
    m_nodeTimings.resize(m_DSPNodeChain.size()*2);
  }

  m_format = m_outputFormat;
  //! @todo AudioDSP V2 is this needed?
  if(!CActiveAEBufferPool::Create(totaltime))
  {
    CLog::Log(LOGERROR, "%s - failed to create AudioDSP buffer pool!", __FUNCTION__);

    return false;
  }

  return true;
}

void CAudioDSPProcessingBuffer::Destroy()
{
}

bool CAudioDSPProcessingBuffer::ProcessBuffer()
{
  int64_t startTime = 0;
  if (m_nodeTimings.size() > 0)
  {
    startTime = CurrentHostCounter();
  }
  bool busy = false;
  CSampleBuffer *buffer;

  static bool copyInput = false;
  if (m_DSPNodeChain.empty() || copyInput)
  {   
    while (!m_inputSamples.empty())
    {
      m_outputSamples.push_back(m_inputSamples.front());
      m_inputSamples.pop_front();
      busy = true;
    }
  }
  else
  {
    // send buffer to the next node
    CSampleBuffer *inBuf = nullptr; //In;
    CSampleBuffer *buf = nullptr; //In;
    std::deque<ActiveAE::CSampleBuffer*> *in = &m_inputSamples;
    std::deque<ActiveAE::CSampleBuffer*> *out = nullptr;
    CAudioDSPModeHandle *adspNodes = m_DSPNodeChain.data();
    bool busy = false;
    //for (AudioDSPNodeChain_t::iterator iter = m_DSPNodeChain.begin(); iter != m_DSPNodeChain.end(); ++iter)
    for (unsigned int ii = 0; ii < m_DSPNodeChain.size(); ii++)
    {
      if (ii + 1 < m_DSPNodeChain.size())
      {
        if (adspNodes[ii + 1].m_buffer)
        {
          //! @todo AudioDSP V2 get m_inputSamples from conversion buffer
          bool dummy = false;
        }
        else
        {
          out = &adspNodes[ii + 1].m_mode->m_inputSamples;
        }
      }
      else
      {
        out = &m_outputSamples;
      }

      // mode input buffers to node input buffers and do a conversion if needed
      while (!in->empty())
      {
        adspNodes[ii].m_buffer->m_inputSamples.push_back(in->front());
        in->pop_front();
        busy = true;
      }

      busy |= adspNodes[ii].m_buffer->ResampleBuffers();

      while (!adspNodes[ii].m_buffer->m_outputSamples.empty())
      {
        adspNodes[ii].m_mode->m_inputSamples.push_back(adspNodes[ii].m_buffer->m_outputSamples.front());
        adspNodes[ii].m_buffer->m_outputSamples.pop_front();
        busy = true;
      }

      if (m_nodeTimings.size() > 0)
      {
        m_nodeTimings[2*ii] = CurrentHostCounter();
      }
      busy |= adspNodes[ii].m_mode->Process();
      if (m_nodeTimings.size() > 0)
      {
        m_nodeTimings[2*ii + 1] = CurrentHostCounter();
      }

      // move output buffers to next node input buffers and do a conversion if needed
      while (!adspNodes[ii].m_mode->m_outputSamples.empty())
      {
        out->push_back(adspNodes[ii].m_mode->m_outputSamples.front());
        adspNodes[ii].m_mode->m_outputSamples.pop_front();
        busy = true;
      }

      // prepare for next node
      in = &adspNodes[ii].m_mode->m_outputSamples;
    }

    return busy;
  }


  if (m_nodeTimings.size() > 0)
  {
    int64_t endTime = CurrentHostCounter();
    m_AudioDSPController.SendTimings(m_nodeTimings, startTime, endTime);
  }

  return busy;
}

bool CAudioDSPProcessingBuffer::HasInputLevel(int level)
{
  //! @todo AudioDSP V2 also calculate delay from conversion buffers
  if (m_inputSamples.size() + m_DSPNodeChain[0].m_buffer->m_inputSamples.size() >= m_DSPNodeChain[0].m_buffer->m_allSamples.size() * level / 100)
  {
    return true;
  }
  else
  {
    return false;
  }
   
  return false;
}

float CAudioDSPProcessingBuffer::GetDelay()
{
  float delay = 0;
  std::deque<CSampleBuffer*>::iterator itBuf;

  for (itBuf = m_inputSamples.begin(); itBuf != m_inputSamples.end(); ++itBuf)
  {
    delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
  }

  for (itBuf = m_outputSamples.begin(); itBuf != m_outputSamples.end(); ++itBuf)
  {
    delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
  }

  for (unsigned int ii = 0; ii < m_DSPNodeChain.size(); ii++)
  {
    for (itBuf = m_DSPNodeChain[ii].m_mode->m_inputSamples.begin(); itBuf != m_DSPNodeChain[ii].m_mode->m_inputSamples.end(); ++itBuf)
    {
      delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
    }

    for (itBuf = m_DSPNodeChain[ii].m_mode->m_outputSamples.begin(); itBuf != m_DSPNodeChain[ii].m_mode->m_outputSamples.end(); ++itBuf)
    {
      delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
    }

    for (itBuf = m_DSPNodeChain[ii].m_buffer->m_inputSamples.begin(); itBuf != m_DSPNodeChain[ii].m_buffer->m_inputSamples.end(); ++itBuf)
    {
      delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
    }

    for (itBuf = m_DSPNodeChain[ii].m_buffer->m_outputSamples.begin(); itBuf != m_DSPNodeChain[ii].m_buffer->m_outputSamples.end(); ++itBuf)
    {
      delay += (float)(*itBuf)->pkt->nb_samples / (*itBuf)->pkt->config.sample_rate;
    }

    //! @todo AudioDSP V2 implement buffered samples from nodes
  }

  return delay;
}

void CAudioDSPProcessingBuffer::Flush()
{  
  while (!m_inputSamples.empty())
  {
    m_inputSamples.front()->Return();
    m_inputSamples.pop_front();
  }
  
  while (!m_outputSamples.empty())
  {
    m_outputSamples.front()->Return();
    m_outputSamples.pop_front();
  }

  for (int ii = 0; ii < m_DSPNodeChain.size(); ii++)
  {
    if (m_DSPNodeChain[ii].m_buffer)
    {
      while (!m_DSPNodeChain[ii].m_buffer->m_inputSamples.empty())
      {
        m_DSPNodeChain[ii].m_buffer->m_inputSamples.front()->Return();
        m_DSPNodeChain[ii].m_buffer->m_inputSamples.pop_front();
      }

      while (!m_DSPNodeChain[ii].m_buffer->m_outputSamples.empty())
      {
        m_DSPNodeChain[ii].m_buffer->m_outputSamples.front()->Return();
        m_DSPNodeChain[ii].m_buffer->m_outputSamples.pop_front();
      }

      m_DSPNodeChain[ii].m_buffer->Flush();
    }

    if (m_DSPNodeChain[ii].m_mode)
    {
      while (!m_DSPNodeChain[ii].m_mode->m_inputSamples.empty())
      {
        m_DSPNodeChain[ii].m_mode->m_inputSamples.front()->Return();
        m_DSPNodeChain[ii].m_mode->m_inputSamples.pop_front();
      }

      while (!m_DSPNodeChain[ii].m_mode->m_outputSamples.empty())
      {
        m_DSPNodeChain[ii].m_mode->m_outputSamples.front()->Return();
        m_DSPNodeChain[ii].m_mode->m_outputSamples.pop_front();
      }
    }
  }
}

void CAudioDSPProcessingBuffer::SetDrain(bool drain)
{
  m_drain = drain;
}

bool CAudioDSPProcessingBuffer::IsDrained()
{
  if (//! @todo AudioDSP V2 implement this /*m_processor->m_inputSamples.empty() &&*/
    !m_inputSamples.empty() ||
    !m_outputSamples.empty())

  {
    return false;
  }

  return true;
}

void CAudioDSPProcessingBuffer::FillBuffer()
{
  m_fillPackets = true;
}

bool CAudioDSPProcessingBuffer::HasWork()
{
  if (!m_inputSamples.empty())
    return true;
  if (!m_outputSamples.empty())
    return true;
  if (m_DSPNodeChain.size() > 0 && m_DSPNodeChain[0].m_buffer->m_inputSamples.size())
    return true;

  return false;
}

void CAudioDSPProcessingBuffer::SetOutputSampleRate(unsigned int OutputSampleRate)
{
  //! @todo AudioDSP V2 implement this
  //m_resampleRatio = (double)m_inputFormat.m_sampleRate / OutputSampleRate;
  m_outputFormat.m_sampleRate = OutputSampleRate;
}

DSPErrorCode_t CAudioDSPProcessingBuffer::EnableNodeCallback(uint64_t ID, uint32_t Position)
{
  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPProcessingBuffer::DisableNodeCallback(uint64_t ID)
{
  return DSP_ERR_NO_ERR;
}

