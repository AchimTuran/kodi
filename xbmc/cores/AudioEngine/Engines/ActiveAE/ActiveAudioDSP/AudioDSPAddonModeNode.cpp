#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonModeNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"

using namespace DSP;
using namespace DSP::AUDIO;
using namespace ActiveAE;

AUDIODSP_ADDON_CHANNEL TranslateAEChannel(AEChannel Channel);
unsigned long GetPresentChannels(const CAEChannelInfo &ChannelLayout);

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

CAudioDSPAddonModeNode::CAudioDSPAddonModeNode(const AEAudioFormat & InputFormat, const AEAudioFormat & OutputFormat, ADDON_HANDLE_STRUCT & Handle, pAudioDSPAddon_t AddonInstance, uint64_t ID, int32_t AddonModeID) :
  m_addon(AddonInstance),
  IADSPBufferNode(AddonInstance->ID(), ID),
  m_handle(Handle)
{
  m_InputFormat = InputFormat;
  m_OutputFormat = OutputFormat;
}

DSPErrorCode_t CAudioDSPAddonModeNode::CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat)
{
  InputFormat.m_dataFormat = AE_FMT_FLOATP;
  OutputFormat.m_dataFormat = AE_FMT_FLOATP;

  //! @todo AudioDSP V2 simplify add-on mode creation API
  AUDIODSP_ADDON_SETTINGS addonSettings;
  addonSettings.StreamID = m_streamID;                /*!< @brief id of the audio stream packets */
  addonSettings.StreamType = AUDIODSP_ADDON_ASTREAM_AUTO; //! @todo AudioDSP V2 this should be set during mode creation              /*!< @brief the input stream type source eg, Movie or Music */
  //addonSettings.iInChannels = InputFormat.m_channelLayout.Count();              /*!< @brief the amount of input channels */
  //addonSettings.lInChannelPresentFlags = GetPresentChannels(InputFormat.m_channelLayout);
  //addonSettings.iInFrames = InputFormat.m_frames;                /*!< @brief the input frame size from KODI */
  //addonSettings.iInSamplerate = InputFormat.m_sampleRate;            /*!< @brief the basic sample rate of the audio packet */
  //addonSettings.iProcessFrames = InputFormat.m_frames;           /*!< @brief the processing frame size inside add-on's */
  //addonSettings.iProcessSamplerate = InputFormat.m_sampleRate;       /*!< @brief the sample rate after input resample present in master processing */
  //addonSettings.iOutChannels = OutputFormat.m_channelLayout.Count();             /*!< @brief the amount of output channels */
  //addonSettings.lOutChannelPresentFlags = GetPresentChannels(OutputFormat.m_channelLayout);
  //addonSettings.iOutFrames= OutputFormat.m_frames;               /*!< @brief the final out frame size for KODI */
  //addonSettings.iOutSamplerate = OutputFormat.m_sampleRate;           /*!< @brief the final sample rate of the audio packet */
  addonSettings.QualityLevel = AUDIODSP_ADDON_QUALITY_REALLYHIGH;            /*!< @brief the from KODI selected quality level for signal processing */

  //addonSettings.lInChannelPresentFlags;   /*!< @brief the exact channel mapping flags of input */
  //addonSettings.lOutChannelPresentFlags;  /*!< @brief the exact channel mapping flags for output */

  //if (m_addon->StreamInitialize(&m_handle, &addonSettings) != AUDIODSP_ADDON_ERROR_NO_ERROR)
  //{
  //  return DSP_ERR_FATAL_ERROR;
  //}

  AUDIODSP_ADDON_STREAMTYPE addonStreamType = AUDIODSP_ADDON_ASTREAM_AUTO;// StreamProperties.streamType; //! @todo AudioDSP V2 add translation method
  unsigned int addonModeID = 0; //! @todo AudioDSP V2 set correct addonModeID
  AUDIODSP_ADDON_ERROR dspErr = AUDIODSP_ADDON_ERROR_NO_ERROR;// m_addon->MasterProcessSetMode(&m_handle, addonStreamType, addonModeID, ID);
  if (dspErr != AUDIODSP_ADDON_ERROR_NO_ERROR && dspErr != AUDIODSP_ADDON_ERROR_NOT_IMPLEMENTED)
  {
    return DSP_ERR_FATAL_ERROR;
  }

  //unsigned long outChannelFlags = 0x0;
  //int outputChannelAmount = 0;
  //outputChannelAmount = 0;// m_addon->MasterProcessGetOutChannels(&m_handle, outChannelFlags);
  //OutputFormat.m_channelLayout.Reset();
  //for (unsigned int ch = 0; ch < AUDIODSP_ADDON_CH_MAX; ch++)
  //{
  //  if (outChannelFlags & 1 << ch)
  //  {
  //    OutputFormat.m_channelLayout += static_cast<AEChannel>(ch + 1); //! @todo AudioDSP V2 add conversion method
  //  }
  //}

  m_addon->CreateModeInstance(InputFormat, OutputFormat, nullptr);

  return DSP_ERR_NO_ERR;
}

int CAudioDSPAddonModeNode::ProcessInstance(const uint8_t **In, uint8_t **Out)
{
  return 0;// m_addon->MasterProcess(&m_handle, reinterpret_cast<const float**>(In), reinterpret_cast<float**>(Out), m_InputFormat.m_frames);
}

DSPErrorCode_t CAudioDSPAddonModeNode::DestroyInstance()
{
  return DSP_ERR_NO_ERR;
}
