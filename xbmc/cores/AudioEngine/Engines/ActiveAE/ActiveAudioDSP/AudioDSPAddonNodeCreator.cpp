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
AE_DSP_BASETYPE GetAEDSPBaseType(AESourceFormat SourceFormat);
AE_DSP_STREAMTYPE TranslateStreamType(AEStreamType StreamType);
AE_DSP_CHANNEL TranslateAEChannel(AEChannel Channel);
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
  AE_DSP_ERROR dspErr = AE_DSP_ERROR_NO_ERROR;
  

  AudioDSPAddonStreamMap_t::iterator iter = m_addonModeMap.find(StreamID);
  if (iter == m_addonModeMap.end())
  { // create new add-on stream
    m_addonModeMap[StreamID] = AddonStreamHandle_t();

    AE_DSP_SETTINGS addonSettings;
    addonSettings.iStreamID = StreamID;                /*!< @brief id of the audio stream packets */
    //! @todo AudioDSP V2 this should be set during mode creation
    addonSettings.iStreamType = TranslateStreamType(StreamProperties.streamType);             /*!< @brief the input stream type source eg, Movie or Music */
    addonSettings.iInChannels = InputFormat.m_channelLayout.Count();              /*!< @brief the amount of input channels */
    addonSettings.iInFrames = InputFormat.m_frames;                /*!< @brief the input frame size from KODI */
    addonSettings.iInSamplerate = InputFormat.m_sampleRate;            /*!< @brief the basic sample rate of the audio packet */
    addonSettings.iProcessFrames = InputFormat.m_frames;           /*!< @brief the processing frame size inside add-on's */
    addonSettings.iProcessSamplerate = InputFormat.m_sampleRate;       /*!< @brief the sample rate after input resample present in master processing */
    addonSettings.iOutChannels = OutputFormat.m_channelLayout.Count();             /*!< @brief the amount of output channels */
    addonSettings.iOutFrames = OutputFormat.m_frames;               /*!< @brief the final out frame size for KODI */
    addonSettings.iOutSamplerate = OutputFormat.m_sampleRate;           /*!< @brief the final sample rate of the audio packet */
    addonSettings.iQualityLevel = AE_DSP_QUALITY_REALLYHIGH;            /*!< @brief the from KODI selected quality level for signal processing */

    addonSettings.lInChannelPresentFlags = GetPresentChannels(InputFormat.m_channelLayout);   /*!< @brief the exact channel mapping flags of input */
    addonSettings.lOutChannelPresentFlags = GetPresentChannels(OutputFormat.m_channelLayout);  /*!< @brief the exact channel mapping flags for output */

    //! @todo AudioDSP V2 is this really needed?
    AE_DSP_STREAM_PROPERTIES streamProperties;
    streamProperties.StreamID = StreamID;
    //! @todo AudioDSP V2 this should be set during mode creation
    streamProperties.StreamType = TranslateStreamType(StreamProperties.streamType);
    streamProperties.BaseType = GetAEDSPBaseType(StreamProperties.sourceFormat);
    streamProperties.strName = ""; //! @todo AudioDSP V2 add stream name
    streamProperties.strCodecId = ""; //! @todo AudioDSP codec ID
    streamProperties.strLanguage = ""; //! @todo AudioDSP V2 stream language
    streamProperties.iIdentifier = 0; //! @todo AudioDSP V2 forward audio player stream ID
    streamProperties.iChannels = InputFormat.m_channelLayout.Count(); //! @todo AudioDSP V
    streamProperties.iSampleRate = InputFormat.m_sampleRate; //! @todo AudioDSP V2 forward source sample rate
    //! @todo AudioDSP V2 is the Profile really needed?
    memset(&streamProperties.Profile, 0, sizeof(AE_DSP_PROFILE));

    dspErr = m_addon->StreamCreate(&addonSettings, &streamProperties, &m_addonModeMap[StreamID].handle);
    if (dspErr != AE_DSP_ERROR_NO_ERROR)
    {
      return nullptr;
    }

    if (m_addon->StreamInitialize(&m_addonModeMap[StreamID].handle, &addonSettings) != AE_DSP_ERROR_NO_ERROR)
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

AE_DSP_BASETYPE GetAEDSPBaseType(AESourceFormat SourceFormat)
{
  switch (SourceFormat)
  {
    case AE_SOURCE_FORMAT_PCM:
      return AE_DSP_ABASE_PCM;
    case AE_SOURCE_FORMAT_ADPCM:
      return AE_DSP_ABASE_ADPCM;
    case AE_SOURCE_FORMAT_DPCM:
      return AE_DSP_ABASE_DPCM;
    case AE_SOURCE_FORMAT_MP2:
      return AE_DSP_ABASE_MP2;
    case AE_SOURCE_FORMAT_MP3:
      return AE_DSP_ABASE_MP3;
    case AE_SOURCE_FORMAT_AC3:
      return AE_DSP_ABASE_AC3;
    case AE_SOURCE_FORMAT_DTS:
      return AE_DSP_ABASE_DTS;
    case AE_SOURCE_FORMAT_VORBIS:
      return AE_DSP_ABASE_VORBIS;
    case AE_SOURCE_FORMAT_DVAUDIO:
      return AE_DSP_ABASE_DVAUDIO;
    case AE_SOURCE_FORMAT_WMAV1:
      return AE_DSP_ABASE_WMAV1;
    case AE_SOURCE_FORMAT_WMAV2:
      return AE_DSP_ABASE_WMAV2;
    case AE_SOURCE_FORMAT_MACE3:
      return AE_DSP_ABASE_MACE3;
    case AE_SOURCE_FORMAT_MACE6:
      return AE_DSP_ABASE_MACE6;
    case AE_SOURCE_FORMAT_VMDAUDIO:
      return AE_DSP_ABASE_VMDAUDIO;
    case AE_SOURCE_FORMAT_FLAC:
      return AE_DSP_ABASE_FLAC;
    case AE_SOURCE_FORMAT_MP3ADU:
      return AE_DSP_ABASE_MP3ADU;
    case AE_SOURCE_FORMAT_MP3ON4:
      return AE_DSP_ABASE_MP3ON4;
    case AE_SOURCE_FORMAT_SHORTEN:
      return AE_DSP_ABASE_SHORTEN;
    case AE_SOURCE_FORMAT_ALAC:
      return AE_DSP_ABASE_ALAC;
    case AE_SOURCE_FORMAT_WESTWOOD_SND1:
      return AE_DSP_ABASE_WESTWOOD_SND1;
    case AE_SOURCE_FORMAT_GSM: ///< as in Berlin toast format
      return AE_DSP_ABASE_GSM; ///< as in Berlin toast format
    case AE_SOURCE_FORMAT_QDM2:
      return AE_DSP_ABASE_QDM2;
    case AE_SOURCE_FORMAT_COOK:
      return AE_DSP_ABASE_COOK;
    case AE_SOURCE_FORMAT_TRUESPEECH:
      return AE_DSP_ABASE_TRUESPEECH;
    case AE_SOURCE_FORMAT_TTA:
      return AE_DSP_ABASE_TTA;
    case AE_SOURCE_FORMAT_SMACKAUDIO:
      return AE_DSP_ABASE_SMACKAUDIO;
    case AE_SOURCE_FORMAT_QCELP:
      return AE_DSP_ABASE_QCELP;
    case AE_SOURCE_FORMAT_WAVPACK:
      return AE_DSP_ABASE_WAVPACK;
    case AE_SOURCE_FORMAT_DSICINAUDIO:
      return AE_DSP_ABASE_DSICINAUDIO;
    case AE_SOURCE_FORMAT_IMC:
      return AE_DSP_ABASE_IMC;
    case AE_SOURCE_FORMAT_MUSEPACK7:
      return AE_DSP_ABASE_MUSEPACK7;
    case AE_SOURCE_FORMAT_MLP:
      return AE_DSP_ABASE_MLP;
    case AE_SOURCE_FORMAT_GSM_MS: ///< as found in WAV
      return AE_DSP_ABASE_GSM_MS; ///< as found in WAV
    case AE_SOURCE_FORMAT_ATRAC3:
      return AE_DSP_ABASE_ATRAC3;
    case AE_SOURCE_FORMAT_VOXWARE:
      return AE_DSP_ABASE_VOXWARE;
    case AE_SOURCE_FORMAT_APE:
      return AE_DSP_ABASE_APE;
    case AE_SOURCE_FORMAT_NELLYMOSER:
      return AE_DSP_ABASE_NELLYMOSER;
    case AE_SOURCE_FORMAT_MUSEPACK8:
      return AE_DSP_ABASE_MUSEPACK8;
    case AE_SOURCE_FORMAT_SPEEX:
      return AE_DSP_ABASE_SPEEX;
    case AE_SOURCE_FORMAT_WMAVOICE:
      return AE_DSP_ABASE_WMAVOICE;
    case AE_SOURCE_FORMAT_WMAPRO:
      return AE_DSP_ABASE_WMAPRO;
    case AE_SOURCE_FORMAT_WMALOSSLESS:
      return AE_DSP_ABASE_WMALOSSLESS;
    case AE_SOURCE_FORMAT_ATRAC3P:
      return AE_DSP_ABASE_ATRAC3P;
    case AE_SOURCE_FORMAT_EAC3:
      return AE_DSP_ABASE_EAC3;
    case AE_SOURCE_FORMAT_SIPR:
      return AE_DSP_ABASE_SIPR;
    case AE_SOURCE_FORMAT_MP1:
      return AE_DSP_ABASE_MP1;
    case AE_SOURCE_FORMAT_TWINVQ:
      return AE_DSP_ABASE_TWINVQ;
    case AE_SOURCE_FORMAT_TRUEHD:
      return AE_DSP_ABASE_TRUEHD;
    case AE_SOURCE_FORMAT_MP4ALS:
      return AE_DSP_ABASE_MP4ALS;
    case AE_SOURCE_FORMAT_ATRAC1:
      return AE_DSP_ABASE_ATRAC1;
    case AE_SOURCE_FORMAT_BINKAUDIO_RDFT:
      return AE_DSP_ABASE_BINKAUDIO_RDFT;
    case AE_SOURCE_FORMAT_BINKAUDIO_DCT:
      return AE_DSP_ABASE_BINKAUDIO_DCT;
    case AE_SOURCE_FORMAT_AAC_LATM:
      return AE_DSP_ABASE_AAC_LATM;
    case AE_SOURCE_FORMAT_QDMC:
      return AE_DSP_ABASE_QDMC;
    case AE_SOURCE_FORMAT_CELT:
      return AE_DSP_ABASE_CELT;
    case AE_SOURCE_FORMAT_G723_1:
      return AE_DSP_ABASE_G723_1;
    case AE_SOURCE_FORMAT_G729:
      return AE_DSP_ABASE_G729;
    case AE_SOURCE_FORMAT_8SVX_EXP:
      return AE_DSP_ABASE_8SVX_EXP;
    case AE_SOURCE_FORMAT_8SVX_FIB:
      return AE_DSP_ABASE_8SVX_FIB;
    case AE_SOURCE_FORMAT_BMV_AUDIO:
      return AE_DSP_ABASE_BMV_AUDIO;
    case AE_SOURCE_FORMAT_RALF:
      return AE_DSP_ABASE_RALF;
    case AE_SOURCE_FORMAT_IAC:
      return AE_DSP_ABASE_IAC;
    case AE_SOURCE_FORMAT_ILBC:
      return AE_DSP_ABASE_ILBC;
    case AE_SOURCE_FORMAT_OPUS:
      return AE_DSP_ABASE_OPUS;
    case AE_SOURCE_FORMAT_COMFORT_NOISE:
      return AE_DSP_ABASE_COMFORT_NOISE;
    case AE_SOURCE_FORMAT_TAK:
      return AE_DSP_ABASE_TAK;
    case AE_SOURCE_FORMAT_METASOUND:
      return AE_DSP_ABASE_METASOUND;
    case AE_SOURCE_FORMAT_PAF_AUDIO:
      return AE_DSP_ABASE_PAF_AUDIO;
    case AE_SOURCE_FORMAT_ON2AVC:
      return AE_DSP_ABASE_ON2AVC;
    case AE_SOURCE_FORMAT_DSS_SP:
      return AE_DSP_ABASE_DSS_SP;
    case AE_SOURCE_FORMAT_FFWAVESYNTH:
      return AE_DSP_ABASE_FFWAVESYNTH;
    case AE_SOURCE_FORMAT_SONIC:
      return AE_DSP_ABASE_SONIC;
    case AE_SOURCE_FORMAT_SONIC_LS:
      return AE_DSP_ABASE_SONIC_LS;
    case AE_SOURCE_FORMAT_EVRC:
      return AE_DSP_ABASE_EVRC;
    case AE_SOURCE_FORMAT_SMV:
      return AE_DSP_ABASE_SMV;
    case AE_SOURCE_FORMAT_DSD_LSBF:
      return AE_DSP_ABASE_DSD_LSBF;
    case AE_SOURCE_FORMAT_DSD_MSBF:
      return AE_DSP_ABASE_DSD_MSBF;
    case AE_SOURCE_FORMAT_DSD_LSBF_PLANAR:
      return AE_DSP_ABASE_DSD_LSBF_PLANAR;
    case AE_SOURCE_FORMAT_DSD_MSBF_PLANAR:
      return AE_DSP_ABASE_DSD_MSBF_PLANAR;
    case AE_SOURCE_FORMAT_4GV:
      return AE_DSP_ABASE_4GV;
    case AE_SOURCE_FORMAT_INTERPLAY_ACM:
      return AE_DSP_ABASE_INTERPLAY_ACM;
    case AE_SOURCE_FORMAT_XMA1:
      return AE_DSP_ABASE_XMA1;
    case AE_SOURCE_FORMAT_XMA2:
      return AE_DSP_ABASE_XMA2;
    case AE_SOURCE_FORMAT_DST:
      return AE_DSP_ABASE_DST;
    case AE_SOURCE_FORMAT_ATRAC3AL:
      return AE_DSP_ABASE_ATRAC3AL;
    case AE_SOURCE_FORMAT_ATRAC3PAL:
      return AE_DSP_ABASE_ATRAC3PAL;
    case AE_SOURCE_FORMAT_AAC:
      return AE_DSP_ABASE_AAC;
    default:
      return AE_DSP_ABASE_UNKNOWN;
  }

  //! @todo AudioDSP V2 add these enums
  //return AE_DSP_ABASE_STEREO;
  //return AE_DSP_ABASE_MONO;
  //return AE_DSP_ABASE_MULTICHANNEL;
  //return AE_DSP_ABASE_DTSHD_MA;
  //return AE_DSP_ABASE_DTSHD_HRA;
}

AE_DSP_CHANNEL TranslateAEChannel(AEChannel Channel)
{
  switch (Channel)
  {
    case AE_CH_FL:
      return AE_DSP_CH_FL;
    case AE_CH_FR:
      return AE_DSP_CH_FR;
    case AE_CH_FC:
      return AE_DSP_CH_FC;
    case AE_CH_LFE:
      return AE_DSP_CH_LFE;
    case AE_CH_BL:
      return AE_DSP_CH_BL;
    case AE_CH_BR:
      return AE_DSP_CH_BR;
    case AE_CH_FLOC:
      return AE_DSP_CH_FLOC;
    case AE_CH_FROC:
      return AE_DSP_CH_FROC;
    case AE_CH_BC:
      return AE_DSP_CH_BC;
    case AE_CH_SL:
      return AE_DSP_CH_SL;
    case AE_CH_SR:
      return AE_DSP_CH_SR;
    case AE_CH_TFL:
      return AE_DSP_CH_TFL;
    case AE_CH_TFR:
      return AE_DSP_CH_TFR;
    case AE_CH_TFC:
      return AE_DSP_CH_TFC;
    case AE_CH_TC:
      return AE_DSP_CH_TC;
    case AE_CH_TBL:
      return AE_DSP_CH_TBL;
    case AE_CH_TBR:
      return AE_DSP_CH_TBR;
    case AE_CH_TBC:
      return AE_DSP_CH_TBC;
    case AE_CH_BLOC:
      return AE_DSP_CH_BLOC;
    case AE_CH_BROC:
      return AE_DSP_CH_BROC;
    default:
      return AE_DSP_CH_INVALID;
  }
}

unsigned long GetPresentChannels(const CAEChannelInfo &ChannelLayout)
{
  unsigned long channelFlags = 0x0;
  for (int ch = 0; ch < ChannelLayout.Count(); ch++)
  {
    AE_DSP_CHANNEL addonChannel = TranslateAEChannel(ChannelLayout[ch]);
    if (addonChannel != AE_DSP_CH_INVALID && addonChannel != AE_DSP_CH_MAX)
    {
      channelFlags |= 1 << addonChannel;
    }
  }

  return channelFlags;
}

AE_DSP_STREAMTYPE TranslateStreamType(AEStreamType StreamType)
{
  switch (StreamType)
  {
    case AE_STREAM_MUSIC:
      return AE_DSP_ASTREAM_MUSIC;
    case AE_STREAM_MOVIE:
      return AE_DSP_ASTREAM_MOVIE;
    case AE_STREAM_GAME:
      return AE_DSP_ASTREAM_GAME;
    case AE_STREAM_APP:
      return AE_DSP_ASTREAM_APP;
    case AE_STREAM_PHONE:
      return AE_DSP_ASTREAM_PHONE;
    case AE_STREAM_MESSAGE:
      return AE_DSP_ASTREAM_MESSAGE;
    default:
      return AE_DSP_ASTREAM_INVALID;
  }
}

//! @todo AudioDSP V2 merge add-on and AE enums
//case AE_STREAM_TV:
//return AE_DSP_ASTREAM_BASIC;
}
