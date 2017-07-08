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
AUDIODSP_ADDON_BASETYPE GetAEDSPBaseType(AESourceFormat SourceFormat);
AUDIODSP_ADDON_STREAMTYPE TranslateStreamType(AEStreamType StreamType);
AUDIODSP_ADDON_CHANNEL TranslateAEChannel(AEChannel Channel);
unsigned long GetPresentChannels(const CAEChannelInfo &ChannelLayout);

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

  CAudioDSPAddonModeNode *addonMode = dynamic_cast<CAudioDSPAddonModeNode*>(Node);
  if (addonMode)
  {
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
  AUDIODSP_ADDON_ERROR dspErr = AUDIODSP_ADDON_ERROR_NO_ERROR;
  

  AudioDSPAddonStreamMap_t::iterator iter = m_addonModeMap.find(StreamID);
  if (iter == m_addonModeMap.end())
  { // create new add-on stream
    m_addonModeMap[StreamID] = AddonStreamHandle_t();

    AUDIODSP_ADDON_SETTINGS addonSettings;
    //addonSettings.iStreamID = StreamID;                /*!< @brief id of the audio stream packets */
    ////! @todo AudioDSP V2 this should be set during mode creation
    //addonSettings.iStreamType = TranslateStreamType(StreamProperties.streamType);             /*!< @brief the input stream type source eg, Movie or Music */
    //addonSettings.iInChannels = InputFormat.m_channelLayout.Count();              /*!< @brief the amount of input channels */
    //addonSettings.iInFrames = InputFormat.m_frames;                /*!< @brief the input frame size from KODI */
    //addonSettings.iInSamplerate = InputFormat.m_sampleRate;            /*!< @brief the basic sample rate of the audio packet */
    //addonSettings.iProcessFrames = InputFormat.m_frames;           /*!< @brief the processing frame size inside add-on's */
    //addonSettings.iProcessSamplerate = InputFormat.m_sampleRate;       /*!< @brief the sample rate after input resample present in master processing */
    //addonSettings.iOutChannels = OutputFormat.m_channelLayout.Count();             /*!< @brief the amount of output channels */
    //addonSettings.iOutFrames = OutputFormat.m_frames;               /*!< @brief the final out frame size for KODI */
    //addonSettings.iOutSamplerate = OutputFormat.m_sampleRate;           /*!< @brief the final sample rate of the audio packet */
    //addonSettings.iQualityLevel = AUDIODSP_ADDON_QUALITY_REALLYHIGH;            /*!< @brief the from KODI selected quality level for signal processing */

    //addonSettings.lInChannelPresentFlags = GetPresentChannels(InputFormat.m_channelLayout);   /*!< @brief the exact channel mapping flags of input */
    //addonSettings.lOutChannelPresentFlags = GetPresentChannels(OutputFormat.m_channelLayout);  /*!< @brief the exact channel mapping flags for output */

    //! @todo AudioDSP V2 is this really needed?
    AUDIODSP_ADDON_STREAM_PROPERTIES streamProperties;
    //streamProperties.StreamID = StreamID;
    ////! @todo AudioDSP V2 this should be set during mode creation
    //streamProperties.StreamType = TranslateStreamType(StreamProperties.streamType);
    //streamProperties.BaseType = GetAEDSPBaseType(StreamProperties.sourceFormat);
    //streamProperties.strName = ""; //! @todo AudioDSP V2 add stream name
    //streamProperties.strCodecId = ""; //! @todo AudioDSP codec ID
    //streamProperties.strLanguage = ""; //! @todo AudioDSP V2 stream language
    //streamProperties.iIdentifier = 0; //! @todo AudioDSP V2 forward audio player stream ID
    //streamProperties.iChannels = InputFormat.m_channelLayout.Count(); //! @todo AudioDSP V
    //streamProperties.iSampleRate = InputFormat.m_sampleRate; //! @todo AudioDSP V2 forward source sample rate
    ////! @todo AudioDSP V2 is the Profile really needed?
    //memset(&streamProperties.Profile, 0, sizeof(AE_DSP_PROFILE));

    //dspErr = m_addon->StreamCreate(&addonSettings, &streamProperties, &m_addonModeMap[StreamID].handle);
    if (dspErr != AUDIODSP_ADDON_ERROR_NO_ERROR)
    {
      return nullptr;
    }

    if (m_addon->StreamInitialize(&m_addonModeMap[StreamID].handle, &addonSettings) != AUDIODSP_ADDON_ERROR_NO_ERROR)
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

  IADSPNode *node = dynamic_cast<IADSPNode*>(new CAudioDSPAddonModeNode(InputFormat, OutputFormat, m_addonModeMap[StreamID].handle, m_addon, ID, 0)); //! @todo use <add-on name>::<mode name> as ID identifier generation and add-on mode ID
  if (!node)
  {
    return nullptr;
  }

  return node;
}

AUDIODSP_ADDON_BASETYPE GetAEDSPBaseType(AESourceFormat SourceFormat)
{
  switch (SourceFormat)
  {
    case AE_SOURCE_FORMAT_PCM:
      return AUDIODSP_ADDON_ABASE_PCM;
    case AE_SOURCE_FORMAT_ADPCM:
      return AUDIODSP_ADDON_ABASE_ADPCM;
    case AE_SOURCE_FORMAT_DPCM:
      return AUDIODSP_ADDON_ABASE_DPCM;
    case AE_SOURCE_FORMAT_MP2:
      return AUDIODSP_ADDON_ABASE_MP2;
    case AE_SOURCE_FORMAT_MP3:
      return AUDIODSP_ADDON_ABASE_MP3;
    case AE_SOURCE_FORMAT_AC3:
      return AUDIODSP_ADDON_ABASE_AC3;
    case AE_SOURCE_FORMAT_DTS:
      return AUDIODSP_ADDON_ABASE_DTS;
    case AE_SOURCE_FORMAT_VORBIS:
      return AUDIODSP_ADDON_ABASE_VORBIS;
    case AE_SOURCE_FORMAT_DVAUDIO:
      return AUDIODSP_ADDON_ABASE_DVAUDIO;
    case AE_SOURCE_FORMAT_WMAV1:
      return AUDIODSP_ADDON_ABASE_WMAV1;
    case AE_SOURCE_FORMAT_WMAV2:
      return AUDIODSP_ADDON_ABASE_WMAV2;
    case AE_SOURCE_FORMAT_MACE3:
      return AUDIODSP_ADDON_ABASE_MACE3;
    case AE_SOURCE_FORMAT_MACE6:
      return AUDIODSP_ADDON_ABASE_MACE6;
    case AE_SOURCE_FORMAT_VMDAUDIO:
      return AUDIODSP_ADDON_ABASE_VMDAUDIO;
    case AE_SOURCE_FORMAT_FLAC:
      return AUDIODSP_ADDON_ABASE_FLAC;
    case AE_SOURCE_FORMAT_MP3ADU:
      return AUDIODSP_ADDON_ABASE_MP3ADU;
    case AE_SOURCE_FORMAT_MP3ON4:
      return AUDIODSP_ADDON_ABASE_MP3ON4;
    case AE_SOURCE_FORMAT_SHORTEN:
      return AUDIODSP_ADDON_ABASE_SHORTEN;
    case AE_SOURCE_FORMAT_ALAC:
      return AUDIODSP_ADDON_ABASE_ALAC;
    case AE_SOURCE_FORMAT_WESTWOOD_SND1:
      return AUDIODSP_ADDON_ABASE_WESTWOOD_SND1;
    case AE_SOURCE_FORMAT_GSM: ///< as in Berlin toast format
      return AUDIODSP_ADDON_ABASE_GSM; ///< as in Berlin toast format
    case AE_SOURCE_FORMAT_QDM2:
      return AUDIODSP_ADDON_ABASE_QDM2;
    case AE_SOURCE_FORMAT_COOK:
      return AUDIODSP_ADDON_ABASE_COOK;
    case AE_SOURCE_FORMAT_TRUESPEECH:
      return AUDIODSP_ADDON_ABASE_TRUESPEECH;
    case AE_SOURCE_FORMAT_TTA:
      return AUDIODSP_ADDON_ABASE_TTA;
    case AE_SOURCE_FORMAT_SMACKAUDIO:
      return AUDIODSP_ADDON_ABASE_SMACKAUDIO;
    case AE_SOURCE_FORMAT_QCELP:
      return AUDIODSP_ADDON_ABASE_QCELP;
    case AE_SOURCE_FORMAT_WAVPACK:
      return AUDIODSP_ADDON_ABASE_WAVPACK;
    case AE_SOURCE_FORMAT_DSICINAUDIO:
      return AUDIODSP_ADDON_ABASE_DSICINAUDIO;
    case AE_SOURCE_FORMAT_IMC:
      return AUDIODSP_ADDON_ABASE_IMC;
    case AE_SOURCE_FORMAT_MUSEPACK7:
      return AUDIODSP_ADDON_ABASE_MUSEPACK7;
    case AE_SOURCE_FORMAT_MLP:
      return AUDIODSP_ADDON_ABASE_MLP;
    case AE_SOURCE_FORMAT_GSM_MS: ///< as found in WAV
      return AUDIODSP_ADDON_ABASE_GSM_MS; ///< as found in WAV
    case AE_SOURCE_FORMAT_ATRAC3:
      return AUDIODSP_ADDON_ABASE_ATRAC3;
    case AE_SOURCE_FORMAT_VOXWARE:
      return AUDIODSP_ADDON_ABASE_VOXWARE;
    case AE_SOURCE_FORMAT_APE:
      return AUDIODSP_ADDON_ABASE_APE;
    case AE_SOURCE_FORMAT_NELLYMOSER:
      return AUDIODSP_ADDON_ABASE_NELLYMOSER;
    case AE_SOURCE_FORMAT_MUSEPACK8:
      return AUDIODSP_ADDON_ABASE_MUSEPACK8;
    case AE_SOURCE_FORMAT_SPEEX:
      return AUDIODSP_ADDON_ABASE_SPEEX;
    case AE_SOURCE_FORMAT_WMAVOICE:
      return AUDIODSP_ADDON_ABASE_WMAVOICE;
    case AE_SOURCE_FORMAT_WMAPRO:
      return AUDIODSP_ADDON_ABASE_WMAPRO;
    case AE_SOURCE_FORMAT_WMALOSSLESS:
      return AUDIODSP_ADDON_ABASE_WMALOSSLESS;
    case AE_SOURCE_FORMAT_ATRAC3P:
      return AUDIODSP_ADDON_ABASE_ATRAC3P;
    case AE_SOURCE_FORMAT_EAC3:
      return AUDIODSP_ADDON_ABASE_EAC3;
    case AE_SOURCE_FORMAT_SIPR:
      return AUDIODSP_ADDON_ABASE_SIPR;
    case AE_SOURCE_FORMAT_MP1:
      return AUDIODSP_ADDON_ABASE_MP1;
    case AE_SOURCE_FORMAT_TWINVQ:
      return AUDIODSP_ADDON_ABASE_TWINVQ;
    case AE_SOURCE_FORMAT_TRUEHD:
      return AUDIODSP_ADDON_ABASE_TRUEHD;
    case AE_SOURCE_FORMAT_MP4ALS:
      return AUDIODSP_ADDON_ABASE_MP4ALS;
    case AE_SOURCE_FORMAT_ATRAC1:
      return AUDIODSP_ADDON_ABASE_ATRAC1;
    case AE_SOURCE_FORMAT_BINKAUDIO_RDFT:
      return AUDIODSP_ADDON_ABASE_BINKAUDIO_RDFT;
    case AE_SOURCE_FORMAT_BINKAUDIO_DCT:
      return AUDIODSP_ADDON_ABASE_BINKAUDIO_DCT;
    case AE_SOURCE_FORMAT_AAC_LATM:
      return AUDIODSP_ADDON_ABASE_AAC_LATM;
    case AE_SOURCE_FORMAT_QDMC:
      return AUDIODSP_ADDON_ABASE_QDMC;
    case AE_SOURCE_FORMAT_CELT:
      return AUDIODSP_ADDON_ABASE_CELT;
    case AE_SOURCE_FORMAT_G723_1:
      return AUDIODSP_ADDON_ABASE_G723_1;
    case AE_SOURCE_FORMAT_G729:
      return AUDIODSP_ADDON_ABASE_G729;
    case AE_SOURCE_FORMAT_8SVX_EXP:
      return AUDIODSP_ADDON_ABASE_8SVX_EXP;
    case AE_SOURCE_FORMAT_8SVX_FIB:
      return AUDIODSP_ADDON_ABASE_8SVX_FIB;
    case AE_SOURCE_FORMAT_BMV_AUDIO:
      return AUDIODSP_ADDON_ABASE_BMV_AUDIO;
    case AE_SOURCE_FORMAT_RALF:
      return AUDIODSP_ADDON_ABASE_RALF;
    case AE_SOURCE_FORMAT_IAC:
      return AUDIODSP_ADDON_ABASE_IAC;
    case AE_SOURCE_FORMAT_ILBC:
      return AUDIODSP_ADDON_ABASE_ILBC;
    case AE_SOURCE_FORMAT_OPUS:
      return AUDIODSP_ADDON_ABASE_OPUS;
    case AE_SOURCE_FORMAT_COMFORT_NOISE:
      return AUDIODSP_ADDON_ABASE_COMFORT_NOISE;
    case AE_SOURCE_FORMAT_TAK:
      return AUDIODSP_ADDON_ABASE_TAK;
    case AE_SOURCE_FORMAT_METASOUND:
      return AUDIODSP_ADDON_ABASE_METASOUND;
    case AE_SOURCE_FORMAT_PAF_AUDIO:
      return AUDIODSP_ADDON_ABASE_PAF_AUDIO;
    case AE_SOURCE_FORMAT_ON2AVC:
      return AUDIODSP_ADDON_ABASE_ON2AVC;
    case AE_SOURCE_FORMAT_DSS_SP:
      return AUDIODSP_ADDON_ABASE_DSS_SP;
    case AE_SOURCE_FORMAT_FFWAVESYNTH:
      return AUDIODSP_ADDON_ABASE_FFWAVESYNTH;
    case AE_SOURCE_FORMAT_SONIC:
      return AUDIODSP_ADDON_ABASE_SONIC;
    case AE_SOURCE_FORMAT_SONIC_LS:
      return AUDIODSP_ADDON_ABASE_SONIC_LS;
    case AE_SOURCE_FORMAT_EVRC:
      return AUDIODSP_ADDON_ABASE_EVRC;
    case AE_SOURCE_FORMAT_SMV:
      return AUDIODSP_ADDON_ABASE_SMV;
    case AE_SOURCE_FORMAT_DSD_LSBF:
      return AUDIODSP_ADDON_ABASE_DSD_LSBF;
    case AE_SOURCE_FORMAT_DSD_MSBF:
      return AUDIODSP_ADDON_ABASE_DSD_MSBF;
    case AE_SOURCE_FORMAT_DSD_LSBF_PLANAR:
      return AUDIODSP_ADDON_ABASE_DSD_LSBF_PLANAR;
    case AE_SOURCE_FORMAT_DSD_MSBF_PLANAR:
      return AUDIODSP_ADDON_ABASE_DSD_MSBF_PLANAR;
    case AE_SOURCE_FORMAT_4GV:
      return AUDIODSP_ADDON_ABASE_4GV;
    case AE_SOURCE_FORMAT_INTERPLAY_ACM:
      return AUDIODSP_ADDON_ABASE_INTERPLAY_ACM;
    case AE_SOURCE_FORMAT_XMA1:
      return AUDIODSP_ADDON_ABASE_XMA1;
    case AE_SOURCE_FORMAT_XMA2:
      return AUDIODSP_ADDON_ABASE_XMA2;
    case AE_SOURCE_FORMAT_DST:
      return AUDIODSP_ADDON_ABASE_DST;
    case AE_SOURCE_FORMAT_ATRAC3AL:
      return AUDIODSP_ADDON_ABASE_ATRAC3AL;
    case AE_SOURCE_FORMAT_ATRAC3PAL:
      return AUDIODSP_ADDON_ABASE_ATRAC3PAL;
    case AE_SOURCE_FORMAT_AAC:
      return AUDIODSP_ADDON_ABASE_AAC;
    default:
      return AUDIODSP_ADDON_ABASE_UNKNOWN;
  }

  //! @todo AudioDSP V2 add these enums
  //return AUDIODSP_ADDON_ABASE_STEREO;
  //return AUDIODSP_ADDON_ABASE_MONO;
  //return AUDIODSP_ADDON_ABASE_MULTICHANNEL;
  //return AUDIODSP_ADDON_ABASE_DTSHD_MA;
  //return AUDIODSP_ADDON_ABASE_DTSHD_HRA;
}

AUDIODSP_ADDON_CHANNEL TranslateAEChannel(AEChannel Channel)
{
  switch (Channel)
  {
    case AE_CH_FL:
      return AUDIODSP_ADDON_CH_FL;
    case AE_CH_FR:
      return AUDIODSP_ADDON_CH_FR;
    case AE_CH_FC:
      return AUDIODSP_ADDON_CH_FC;
    case AE_CH_LFE:
      return AUDIODSP_ADDON_CH_LFE;
    case AE_CH_BL:
      return AUDIODSP_ADDON_CH_BL;
    case AE_CH_BR:
      return AUDIODSP_ADDON_CH_BR;
    case AE_CH_FLOC:
      return AUDIODSP_ADDON_CH_FLOC;
    case AE_CH_FROC:
      return AUDIODSP_ADDON_CH_FROC;
    case AE_CH_BC:
      return AUDIODSP_ADDON_CH_BC;
    case AE_CH_SL:
      return AUDIODSP_ADDON_CH_SL;
    case AE_CH_SR:
      return AUDIODSP_ADDON_CH_SR;
    case AE_CH_TFL:
      return AUDIODSP_ADDON_CH_TFL;
    case AE_CH_TFR:
      return AUDIODSP_ADDON_CH_TFR;
    case AE_CH_TFC:
      return AUDIODSP_ADDON_CH_TFC;
    case AE_CH_TC:
      return AUDIODSP_ADDON_CH_TC;
    case AE_CH_TBL:
      return AUDIODSP_ADDON_CH_TBL;
    case AE_CH_TBR:
      return AUDIODSP_ADDON_CH_TBR;
    case AE_CH_TBC:
      return AUDIODSP_ADDON_CH_TBC;
    case AE_CH_BLOC:
      return AUDIODSP_ADDON_CH_BLOC;
    case AE_CH_BROC:
      return AUDIODSP_ADDON_CH_BROC;
    default:
      return AUDIODSP_ADDON_CH_INVALID;
  }
}

unsigned long GetPresentChannels(const CAEChannelInfo &ChannelLayout)
{
  unsigned long channelFlags = 0x0;
  for (int ch = 0; ch < ChannelLayout.Count(); ch++)
  {
    AUDIODSP_ADDON_CHANNEL addonChannel = TranslateAEChannel(ChannelLayout[ch]);
    if (addonChannel != AUDIODSP_ADDON_CH_INVALID && addonChannel != AUDIODSP_ADDON_CH_MAX)
    {
      channelFlags |= 1 << addonChannel;
    }
  }

  return channelFlags;
}

AUDIODSP_ADDON_STREAMTYPE TranslateStreamType(AEStreamType StreamType)
{
  switch (StreamType)
  {
    case AE_STREAM_MUSIC:
      return AUDIODSP_ADDON_ASTREAM_MUSIC;
    case AE_STREAM_MOVIE:
      return AUDIODSP_ADDON_ASTREAM_MOVIE;
    case AE_STREAM_GAME:
      return AUDIODSP_ADDON_ASTREAM_GAME;
    case AE_STREAM_APP:
      return AUDIODSP_ADDON_ASTREAM_APP;
    case AE_STREAM_PHONE:
      return AUDIODSP_ADDON_ASTREAM_PHONE;
    case AE_STREAM_MESSAGE:
      return AUDIODSP_ADDON_ASTREAM_MESSAGE;
    default:
      return AUDIODSP_ADDON_ASTREAM_INVALID;
  }
}

//! @todo AudioDSP V2 merge add-on and AE enums
//case AE_STREAM_TV:
//return AUDIODSP_ADDON_ASTREAM_BASIC;
}
