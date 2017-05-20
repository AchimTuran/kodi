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
  m_NodeFactory(NodeFactory)
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

void CAudioDSPProcessor::CreateBuffer(const AEAudioFormat &Format, NodeBuffer_t *Buffer)
{
  Buffer->planes = AE_IS_PLANAR(Format.m_dataFormat) ? Format.m_channelLayout.Count() : 1;
  Buffer->buffer = new uint8_t*[Buffer->planes];
  Buffer->bytesPerSample = CAEUtil::DataFormatToBits(Format.m_dataFormat) / 8;
  Buffer->maxSamplesCount = Format.m_frames / Format.m_channelLayout.Count();

  for(int ii = 0; ii < Buffer->planes; ii++)
  {
    Buffer->buffer[ii] = new uint8_t[Buffer->bytesPerSample * Buffer->maxSamplesCount];
  }
}

void CAudioDSPProcessor::FreeBuffer(NodeBuffer_t *Buffer)
{
  for (int ii = 0; ii < Buffer->planes; ii++)
  {
    if (Buffer->buffer[ii])
    {
      delete [] Buffer->buffer[ii];
    }
    Buffer->buffer[ii] = nullptr;
  }
  delete [] Buffer->buffer;

  Buffer->buffer = nullptr;
  Buffer->bytesPerSample = 0;
  Buffer->planes = 0;
  Buffer->samplesCount = 0;
  Buffer->maxSamplesCount = 0;
  Buffer->channels = 0;
}

DSPErrorCode_t CAudioDSPProcessor::Create(const AEAudioFormat *InFormat, AEAudioFormat *OutFormat)
{
  IDSPNodeModel::DSPNodeInfoVector_t nodeInfos;
  DSPErrorCode_t dspErr = m_AudioDSPController.GetActiveNodes(nodeInfos);
  if (dspErr != DSP_ERR_NO_ERR)
  {
    return dspErr;
  }

  OutFormat->m_dataFormat = AE_FMT_FLOATP;
  m_InFormat = *InFormat;
  m_OutFormat = *OutFormat;
  AEAudioFormat tmpParameters[2];
  AEAudioFormat &configInParameters = tmpParameters[0];
  AEAudioFormat &configOutParameters = tmpParameters[1];

  configInParameters = m_InFormat;
  configOutParameters = m_InFormat;

  // create node chain
  for(uint32_t ii = 0; ii < nodeInfos.size(); ii++)
  {
    IADSPNode *node = m_NodeFactory.InstantiateNode(nodeInfos.at(ii).ID);
    if (!node)
    {
      return DSP_ERR_FATAL_ERROR;
    }
    DSPErrorCode_t dspErr = node->Create(configInParameters, configOutParameters);
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
    AEAudioFormat &p = configInParameters;
    configInParameters = configOutParameters;
    configOutParameters = p;

    m_DSPNodeChain.push_back(adspNode);
  }

  // configure buffers
  //! @todo implement buffer configuration
  if (m_DSPNodeChain.size() == 0)
  {
      IDSPNodeModel::CDSPNodeInfoQuery query({ "Kodi", "AudioConverter" });
      IDSPNodeModel::CDSPNodeInfo audioConverterInfo = m_AudioDSPController.GetNodeInfo(query);
      IADSPNode *audioConverter = dynamic_cast<IADSPNode*>(m_NodeFactory.InstantiateNode(audioConverterInfo.ID));
      if (!audioConverter)
      {
        return DSP_ERR_INVALID_NODE_ID;
      }
      DSPErrorCode_t dspErr = audioConverter->Create(configInParameters, configOutParameters);
      if (dspErr != DSP_ERR_NO_ERR)
      {
        IADSPNode *node = dynamic_cast<IADSPNode*>(audioConverter);
        m_NodeFactory.DestroyNode(node);
        return dspErr;
      }

      m_DSPNodeChain.push_back(audioConverter);
  }
  else
  {
    if (m_DSPNodeChain.size() == 1)
    {
      AEAudioFormat inFmt = m_DSPNodeChain.at(0)->GetInputFormat();
      if (!(inFmt == m_InFormat))
      { // create a output conversion buffer
        //! @todo add buffer
      }

      m_OutFormat = m_DSPNodeChain.at(0)->GetOutputFormat();
    }
    else
    {
      AEAudioFormat inFmt = m_InFormat;
      for (uint32_t ii = 0; ii < m_DSPNodeChain.size(); ii++)
      {
        AEAudioFormat outFmt;
        outFmt = m_DSPNodeChain.at(ii)->GetInputFormat();

        if (!(inFmt == outFmt))
        { // create conversion buffer
          //! @todo add buffer
        }

        inFmt = m_DSPNodeChain.at(ii)->GetOutputFormat();
      }

      m_OutFormat = inFmt;
    }

    // add audio converter if the first mode needed a different input format
    AudioDSPNodeChain_t::iterator nodeIter = m_DSPNodeChain.begin();
    AEAudioFormat firstModeInputFormat = (*nodeIter)->GetInputFormat();
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

      m_DSPNodeChain.insert(nodeIter, audioConverter);
    }

    nodeIter = m_DSPNodeChain.end();
    AEAudioFormat lastModeOutputFormat = (*nodeIter)->GetOutputFormat();
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

      m_DSPNodeChain.push_back(audioConverter);
    }
  }

  *OutFormat = m_OutFormat;
  // initialize internal format with all available ActiveAE channels
  CAEChannelInfo audioDSPChLayout;
  for(int ch = AE_CH_FL; ch <= AE_CH_BROC; ch++)
  {
    audioDSPChLayout += static_cast<AEChannel>(ch);
  }

  // create buffers
  for(unsigned int ii = 0; ii < m_DSPNodeChain.size(); ii++)
  {
    const AEAudioFormat &nodeOutFormat = m_DSPNodeChain.at(ii)->GetOutputFormat();
    unsigned int channelFrameSize = nodeOutFormat.m_frames / nodeOutFormat.m_channelLayout.Count();
    unsigned int channelSampleSize = nodeOutFormat.m_frameSize / nodeOutFormat.m_channelLayout.Count();
    NodeBuffer_t nodeBuffer;
    AEAudioFormat bufferFormat = nodeOutFormat;
    bufferFormat.m_channelLayout = audioDSPChLayout;
    bufferFormat.m_frames = bufferFormat.m_channelLayout.Count() * channelFrameSize;
    bufferFormat.m_frameSize = bufferFormat.m_channelLayout.Count() * channelSampleSize;
    CreateBuffer(bufferFormat, &nodeBuffer);

    m_Buffers.push_back(nodeBuffer);
  }

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPProcessor::Process(const CSampleBuffer *In, CSampleBuffer *Out)
{
  if (!In || !Out)
  {
    return DSP_ERR_INVALID_INPUT;
  }

//  DSPErrorCode_t dspErr = ReCreateNodeChain();
//  if (dspErr != DSP_ERR_NO_ERR)
//  {
//    return dspErr;
//  }

  //if (m_planes.size() < m_procSample->pkt->planes)
  //{
  //  for (int i = m_planes.size(); i < m_procSample->pkt->planes; i++)
  //  {
  //    m_planes.push_back(nullptr);
  //  }
  //}


  //int start = m_procSample->pkt->nb_samples *
  //            m_procSample->pkt->bytes_per_sample *
  //            m_procSample->pkt->config.channels /
  //            m_procSample->pkt->planes;

  //for (int i = 0; i < m_procSample->pkt->planes; i++)
  //{
  //  m_planes[i] = m_procSample->pkt->data[i] + start;
  //}

  uint8_t **in = In->pkt->data;
  AudioDSPBuffers_t::iterator bufferIter = m_Buffers.begin();
  NodeBuffer_t &outBuffer =  *bufferIter;
  uint8_t **out = outBuffer.buffer;
  DSPErrorCode_t dspErr = DSP_ERR_NO_ERR;
  for (AudioDSPNodeChain_t::iterator iter = m_DSPNodeChain.begin(); iter != m_DSPNodeChain.end() && bufferIter != m_Buffers.end(); ++iter)
  {
    dspErr = (*iter)->ProcessInstance(in, out);
    if (dspErr != DSP_ERR_NO_ERR)
    {
      return dspErr;
    }
    bufferIter->samplesCount += (*iter)->GetOutputFormat().m_frames;
    in = out;

    ++bufferIter;
    if (bufferIter != m_Buffers.end())
    {
      outBuffer =  *bufferIter;
      out = outBuffer.buffer;
    }
  }

  for (int ch = 0; ch < Out->pkt->planes; ch++)
  {
    memcpy(Out->pkt->data[ch], outBuffer.buffer[ch], sizeof(outBuffer.bytesPerSample * outBuffer.maxSamplesCount));
    Out->pkt->nb_samples += outBuffer.maxSamplesCount;
  }

  if (In->pkt->nb_samples != Out->pkt->nb_samples)
  {
    return DSP_ERR_FATAL_ERROR;
  }

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPProcessor::Destroy()
{
  for (AudioDSPNodeChain_t::iterator iter = m_DSPNodeChain.begin(); iter != m_DSPNodeChain.end(); ++iter)
  {
    if (*iter)
    {
      IADSPNode *node = *iter;
      
      DSPErrorCode_t dspErr = m_NodeFactory.DestroyNode(node);
      if (dspErr != DSP_ERR_NO_ERR)
      { //! @todo handle error code
      }
    }
  }

  m_DSPNodeChain.clear();

  for(AudioDSPBuffers_t::iterator iter = m_Buffers.begin(); iter != m_Buffers.end(); ++iter)
  {
    FreeBuffer(&*iter);
  }

  m_Buffers.clear();

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPProcessor::EnableNodeCallback(uint64_t ID, uint32_t Position)
{
  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPProcessor::DisableNodeCallback(uint64_t ID)
{
  return DSP_ERR_NO_ERR;
}
