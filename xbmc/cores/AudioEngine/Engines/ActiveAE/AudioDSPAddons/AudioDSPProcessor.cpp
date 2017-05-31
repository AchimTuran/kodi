/*
 *     Copyright (C) 2005-2017 Team Kodi
 *     http://kodi.tv
 *
 * This Program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kodi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/AudioDSPProcessor.h"
#include "cores/DSP/Models/DSPNodeModel.h"
#include "cores/AudioEngine/Utils/AEUtil.h"

using namespace ActiveAE;
using namespace DSP;
using namespace DSP::AUDIO;

CAudioDSPProcessor::CAudioDSPProcessor(CAudioDSPController &Controller, IDSPNodeFactory &NodeFactory) :
  IADSPProcessor("CAudioDSPProcessor"),
  m_AudioDSPController(Controller),
  m_NodeFactory(NodeFactory),
  m_conversionModeID({ "Kodi", "AudioConverter" })
{
}

CAudioDSPProcessor::~CAudioDSPProcessor()
{
}

DSPErrorCode_t CAudioDSPProcessor::ReCreateNodeChain()
{
  //if (!m_NeedsNodesUpdate)
  //{
  //  return DSP_ERR_NO_ERR;
  //}

  //ADSPChain_t tempNodeList;
  //ADSPChain_t destroyNodeList;
  //IDSPNodeModel::DSPNodeInfoVector_t activeNodes;
  //m_NodeModel->GetActiveNodes(activeNodes);

  //for (uint32_t ii = 0; ii < activeNodes.size(); ii++)
  //{
  //  IADSPChainNode *node = nullptr;
  //  for (ADSPChain_t::iterator iter = m_ActiveDSPChain.begin(); iter != m_ActiveDSPChain.end(); ++iter)
  //  {// search node
  //    if ((*iter)->ID == activeNodes.at(ii).ID)
  //    {
  //      node = *iter;
  //      break;
  //    }
  //  }

  //  if (activeNodes.at(ii).Active && !node)
  //  {// create and add new node
  //    node = dynamic_cast<IADSPChainNode*>(m_NodeFactory->InstantiateNode(activeNodes.at(ii).ID));
  //    if (node)
  //    {
  //      // todo save parameters into common memory
  //      void *InParameters = nullptr;
  //      void *OutParameters = nullptr;
  //      void *Options = nullptr;
  //      // todo save parameters into common memory
  //      node->Create(InParameters, OutParameters, Options);
  //    }
  //  }

  //  tempNodeList.push_back(node);
  //}

  //if (m_ActiveDSPChain.size() != tempNodeList.size())
  //{// disable nodes
  //  for (ADSPChain_t::iterator activeIter = m_ActiveDSPChain.begin(); activeIter != m_ActiveDSPChain.end(); ++activeIter)
  //  {
  //    bool found = false;
  //    for (ADSPChain_t::iterator tmpIter = tempNodeList.begin(); tmpIter != tempNodeList.end(); ++tmpIter)
  //    {// search node
  //      if ((*activeIter)->ID == (*tmpIter)->ID)
  //      {
  //        found = true;
  //        break;
  //      }
  //    }

  //    if (!found)
  //    {
  //      destroyNodeList.push_back(*activeIter);
  //    }
  //  }
  //}

  ////! @todo move this to an other thread
  //DSPErrorCode_t dspErr = DSP_ERR_NO_ERR;
  //if (destroyNodeList.size() > 0)
  //{
  //  for (ADSPChain_t::iterator tmpIter = destroyNodeList.begin(); tmpIter != destroyNodeList.end(); ++tmpIter)
  //  {
  //    IDSPNode *node = dynamic_cast<IDSPNode*>(*tmpIter);
  //    DSPErrorCode_t locErr = m_NodeFactory->DestroyNode(node);
  //    if (locErr != DSP_ERR_NO_ERR)
  //    {
  //      dspErr = locErr;
  //    }
  //  }
  //}

  //// set new DSP chain
  //m_ActiveDSPChain = tempNodeList;
  //m_NeedsNodesUpdate = false;

  //return dspErr;
  return DSP_ERR_NO_ERR;
}

void CAudioDSPProcessor::CreateBuffer(const AEAudioFormat &Format, NodeBuffer_t &Buffer)
{
  Buffer.planes = AE_IS_PLANAR(Format.m_dataFormat) ? Format.m_channelLayout.Count() : 1;
  Buffer.buffer = new uint8_t*[Buffer.planes];
  Buffer.bytesPerSample = CAEUtil::DataFormatToBits(Format.m_dataFormat) / 8;
  Buffer.maxSamplesCount = Format.m_frames;

  for(int ii = 0; ii < Buffer.planes; ii++)
  {
    Buffer.buffer[ii] = new uint8_t[Buffer.bytesPerSample * Buffer.maxSamplesCount];
    memset(Buffer.buffer[ii], 0, sizeof(uint8_t) * Buffer.bytesPerSample * Buffer.maxSamplesCount);
  }
}

void CAudioDSPProcessor::FreeBuffer(NodeBuffer_t &Buffer)
{
  for (int ii = 0; ii < Buffer.planes; ii++)
  {
    if (Buffer.buffer[ii])
    {
      delete [] Buffer.buffer[ii];
    }
    Buffer.buffer[ii] = nullptr;
  }
  delete [] Buffer.buffer;

  Buffer.buffer = nullptr;
  Buffer.bytesPerSample = 0;
  Buffer.planes = 0;
  Buffer.samplesCount = 0;
  Buffer.maxSamplesCount = 0;
  Buffer.channels = 0;
}

DSPErrorCode_t CAudioDSPProcessor::Create(const AEAudioFormat *InFormat, AEAudioFormat *OutFormat)
{
  IDSPNodeModel::DSPNodeInfoVector_t nodeInfos;
  DSPErrorCode_t dspErr = m_AudioDSPController.GetActiveNodes(nodeInfos);
  if (dspErr != DSP_ERR_NO_ERR)
  {
    return dspErr;
  }

  m_InFormat = *InFormat;
  m_OutFormat = *OutFormat;
  AEAudioFormat tmpParameters[2];
  AEAudioFormat *configInParameters = &tmpParameters[0];
  AEAudioFormat *configOutParameters = &tmpParameters[1];

  *configInParameters = m_InFormat;
  *configOutParameters = m_OutFormat;

  // create node chain
  for(uint32_t ii = 0; ii < nodeInfos.size(); ii++)
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
      DSPErrorCode_t dspErr = audioConverter->Create(m_InFormat, m_OutFormat);
      if (dspErr != DSP_ERR_NO_ERR)
      {
        IADSPNode *node = dynamic_cast<IADSPNode*>(audioConverter);
        m_NodeFactory.DestroyNode(node);
        return dspErr;
      }

      m_OutFormat = audioConverter->GetOutputFormat();

      m_DSPNodeChain.push_back(CAudioDSPModeHandle(audioConverter, nullptr));
  }
  else
  {
    if (m_DSPNodeChain.size() == 1)
    {
      AEAudioFormat inFmt = m_DSPNodeChain.at(0).m_mode->GetInputFormat();
      if (!(inFmt == m_InFormat))
      { // create a output conversion buffer
        //! @todo add buffer
      }

      m_OutFormat = m_DSPNodeChain.at(0).m_mode->GetOutputFormat();
    }
    else
    {
      AEAudioFormat inFmt = m_InFormat;
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

      m_OutFormat = inFmt;
    }

    // add audio converter if the first mode needed a different input format
    AEAudioFormat firstModeInputFormat = m_DSPNodeChain.front().m_mode->GetInputFormat();
    if (!(firstModeInputFormat == m_InFormat))
    {
      IDSPNodeModel::CDSPNodeInfoQuery query({ "Kodi", "AudioConverter" });
      IDSPNodeModel::CDSPNodeInfo audioConverterInfo = m_AudioDSPController.GetNodeInfo(query);
      IADSPNode *audioConverter = dynamic_cast<IADSPNode*>(m_NodeFactory.InstantiateNode(audioConverterInfo.ID));
      if (!audioConverter)
      {
        return DSP_ERR_INVALID_NODE_ID;
      }
      DSPErrorCode_t dspErr = audioConverter->Create(m_InFormat, firstModeInputFormat);
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
    if (!(lastModeOutputFormat == m_OutFormat))
    {
      IDSPNodeModel::CDSPNodeInfoQuery query({ "Kodi", "AudioConverter" });
      IDSPNodeModel::CDSPNodeInfo audioConverterInfo = m_AudioDSPController.GetNodeInfo(query);
      IADSPNode *audioConverter = dynamic_cast<IADSPNode*>(m_NodeFactory.InstantiateNode(audioConverterInfo.ID));
      if (!audioConverter)
      {
        return DSP_ERR_INVALID_NODE_ID;
      }
      DSPErrorCode_t dspErr = audioConverter->Create(lastModeOutputFormat, m_OutFormat);
      if (dspErr != DSP_ERR_NO_ERR)
      {
        IADSPNode *node = dynamic_cast<IADSPNode*>(audioConverter);
        m_NodeFactory.DestroyNode(node);
        return dspErr;
      }

      m_DSPNodeChain.push_back(CAudioDSPModeHandle(audioConverter, nullptr));
    }
  }

  *OutFormat = m_OutFormat;
  // initialize internal format with all available ActiveAE channels
  CAEChannelInfo audioDSPChLayout;
  //! @todo AudioDSP V3 add support for all AE channels, this requires to improve the implemented FFMPEG based AudioConversion Mode
  for(int ch = AE_CH_FL; ch < AE_CH_TBL; ch++)
  {
    audioDSPChLayout += static_cast<AEChannel>(ch);
  }

  // create buffers
  *configInParameters = m_InFormat;
  for(unsigned int ii = 0; ii < m_DSPNodeChain.size(); ii++)
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

  return DSP_ERR_NO_ERR;
}

bool CAudioDSPProcessor::ProcessBuffer()
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

    busy |= adspNodes[ii].m_mode->Process();

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

DSPErrorCode_t CAudioDSPProcessor::Destroy()
{
  for (AudioDSPNodeChain_t::iterator iter = m_DSPNodeChain.begin(); iter != m_DSPNodeChain.end(); ++iter)
  {
    if (iter->m_mode)
    {
      DSPErrorCode_t dspErr = m_NodeFactory.DestroyNode(iter->m_mode);
      if (dspErr != DSP_ERR_NO_ERR)
      { //! @todo handle error code
      }
    }

    // at first flush all buffers
    if (iter->m_buffer)
    {
      iter->m_buffer->Flush();
    }

    //! @todo AudioDSP V3 delete conversion mode.
  }

  // now delete all allocated buffers
  for (AudioDSPNodeChain_t::iterator iter = m_DSPNodeChain.begin(); iter != m_DSPNodeChain.end(); ++iter)
  {
    delete iter->m_buffer;
  }

  m_DSPNodeChain.clear();

  return DSP_ERR_NO_ERR;
}

float ActiveAE::CAudioDSPProcessor::GetDelay()
{
  float delay = 0.0f;
  std::deque<CSampleBuffer*>::iterator itBuf;

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

bool CAudioDSPProcessor::HasInputLevel(int level)
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
}

bool CAudioDSPProcessor::HasWork()
{
  if (!m_inputSamples.empty())
    return true;
  if (!m_outputSamples.empty())
    return true;
  if (m_DSPNodeChain[0].m_buffer->m_inputSamples.size())
    return true;

  return false;
}

DSPErrorCode_t CAudioDSPProcessor::EnableNodeCallback(uint64_t ID, uint32_t Position)
{
  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPProcessor::DisableNodeCallback(uint64_t ID)
{
  return DSP_ERR_NO_ERR;
}
