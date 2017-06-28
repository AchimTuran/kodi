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


#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonNodeCreator.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonModeNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPNodeModel.h"

using namespace DSP;
using namespace DSP::AUDIO;

namespace ActiveAE
{
CAudioDSPAddonNodeCreator::CAudioDSPAddonNodeCreator(const AE_DSP_ADDON &Addon) :
  m_addon(Addon)
{
}

DSPErrorCode_t CAudioDSPAddonNodeCreator::DestroyNode(IADSPNode *&Node)
{
  if (!Node)
  {
    return DSP_ERR_INVALID_INPUT;
  }

  DSPErrorCode_t err = Node->Destroy();

  delete Node;
  Node = nullptr;

  return err;
}

IDSPNodeCreator* CAudioDSPAddonNodeCreator::CreateCreator()
{
  return dynamic_cast<IDSPNodeCreator*>(new CAudioDSPAddonNodeCreator(m_addon));
}

IADSPNode* CAudioDSPAddonNodeCreator::InstantiateNode(const AEAudioFormat &InputFormat, 
                                                      const AEAudioFormat &OutputFormat, 
                                                      const AEStreamProperties &StreamProperties, 
                                                      unsigned int StreamID, 
                                                      uint64_t ID)
{
  AE_DSP_ERROR dspErr = AE_DSP_ERROR_NO_ERROR;
  AudioDSPAddonStreamMap_t::iterator iter = m_addonModeMap.find(StreamID);
  if (iter == m_addonModeMap.end())
  { // create new add-on stream
    m_addonModeMap[StreamID] = AddonStreamHandle_t();

    //! @todo AudioDSP V2 translate variables
    AE_DSP_SETTINGS addonSettings;
    addonSettings.iStreamID = StreamID;
    addonSettings.iStreamType;
    addonSettings.iInChannels;
    addonSettings.lInChannelPresentFlags;
    addonSettings.iInFrames;
    addonSettings.iInSamplerate;
    addonSettings.iProcessFrames;
    addonSettings.iProcessSamplerate;
    addonSettings.iOutChannels;
    addonSettings.lOutChannelPresentFlags;
    addonSettings.iOutFrames;
    addonSettings.iOutSamplerate;
    addonSettings.bInputResamplingActive;
    addonSettings.bStereoUpmix;
    addonSettings.iQualityLevel;

    AE_DSP_STREAM_PROPERTIES streamProperties;
    streamProperties.iStreamID;
    streamProperties.iStreamType;
    streamProperties.iBaseType;
    streamProperties.strName;
    streamProperties.strCodecId;
    streamProperties.strLanguage;
    streamProperties.iIdentifier;
    streamProperties.iChannels;
    streamProperties.iSampleRate;
    streamProperties.Profile;

    dspErr = m_addon->StreamCreate(&addonSettings, &streamProperties, &m_addonModeMap[StreamID].handle);
    if (dspErr != AE_DSP_ERROR_NO_ERROR)
    {
      return nullptr;
    }
  }

  NodeID_t nodeID(ID);
  //! @todo AudioDSP V3 add support for multiple mode instance support
  std::map<unsigned int, unsigned int>::iterator modeIter = m_addonModeMap[StreamID].modes.find(nodeID.ModeID);
  if (modeIter != m_addonModeMap[StreamID].modes.end())
  {// current implementation only supports one mode instance
    return nullptr;
  }

  IADSPNode *node = dynamic_cast<IADSPNode*>(new CAudioDSPAddonModeNode(m_addonModeMap[StreamID].handle, m_addon, ID, 0)); //! @todo use <add-on name>::<mode name> as ID identifier generation and add-on mode ID
  if (!node)
  {
    return nullptr;
  }

  return node;
}
}
