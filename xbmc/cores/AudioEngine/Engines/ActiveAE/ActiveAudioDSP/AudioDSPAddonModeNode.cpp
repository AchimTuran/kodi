#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonModeNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"

using namespace DSP;
using namespace DSP::AUDIO;
using namespace ActiveAE;

AE_DSP_CHANNEL TranslateAEChannel(AEChannel Channel);
unsigned long GetPresentChannels(const CAEChannelInfo &ChannelLayout);

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

CAudioDSPAddonModeNode::CAudioDSPAddonModeNode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, ADDON_HANDLE_STRUCT &Handle, AE_DSP_ADDON Addon, uint64_t ID, int StreamID) :
  m_addon(Addon),
  IADSPBufferNode(Addon->ID(), ID),
  m_handle(Handle)
{
  m_InputFormat = (InputFormat);
  m_OutputFormat = (OutputFormat);
}

DSPErrorCode_t CAudioDSPAddonModeNode::CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat)
{
  InputFormat.m_dataFormat = AE_FMT_FLOATP;
  OutputFormat.m_dataFormat = AE_FMT_FLOATP;

  //! @todo AudioDSP V2 simplify add-on mode creation API
  AE_DSP_SETTINGS addonSettings;
  addonSettings.iStreamID = m_streamID;                /*!< @brief id of the audio stream packets */
  addonSettings.iStreamType = AE_DSP_ASTREAM_AUTO; //! @todo AudioDSP V2 this should be set during mode creation              /*!< @brief the input stream type source eg, Movie or Music */
  addonSettings.iInChannels = InputFormat.m_channelLayout.Count();              /*!< @brief the amount of input channels */
  addonSettings.lInChannelPresentFlags = GetPresentChannels(InputFormat.m_channelLayout);
  addonSettings.iInFrames = InputFormat.m_frames;                /*!< @brief the input frame size from KODI */
  addonSettings.iInSamplerate = InputFormat.m_sampleRate;            /*!< @brief the basic sample rate of the audio packet */
  addonSettings.iProcessFrames = InputFormat.m_frames;           /*!< @brief the processing frame size inside add-on's */
  addonSettings.iProcessSamplerate = InputFormat.m_sampleRate;       /*!< @brief the sample rate after input resample present in master processing */
  addonSettings.iOutChannels = OutputFormat.m_channelLayout.Count();             /*!< @brief the amount of output channels */
  addonSettings.lOutChannelPresentFlags = GetPresentChannels(OutputFormat.m_channelLayout);
  addonSettings.iOutFrames= OutputFormat.m_frames;               /*!< @brief the final out frame size for KODI */
  addonSettings.iOutSamplerate = OutputFormat.m_sampleRate;           /*!< @brief the final sample rate of the audio packet */
  addonSettings.iQualityLevel = AE_DSP_QUALITY_REALLYHIGH;            /*!< @brief the from KODI selected quality level for signal processing */

  addonSettings.lInChannelPresentFlags;   /*!< @brief the exact channel mapping flags of input */
  addonSettings.lOutChannelPresentFlags;  /*!< @brief the exact channel mapping flags for output */

  if (m_addon->StreamInitialize(&m_handle, &addonSettings) != AE_DSP_ERROR_NO_ERROR)
  {
    return DSP_ERR_FATAL_ERROR;
  }

  AE_DSP_STREAMTYPE addonStreamType = AE_DSP_ASTREAM_BASIC;// StreamProperties.streamType; //! @todo AudioDSP V2 add translation method
  unsigned int addonModeID = 0; //! @todo AudioDSP V2 set correct addonModeID
  AE_DSP_ERROR dspErr = m_addon->MasterProcessSetMode(&m_handle, addonStreamType, addonModeID, ID);
  if (dspErr != AE_DSP_ERROR_NO_ERROR && dspErr != AE_DSP_ERROR_NOT_IMPLEMENTED)
  {
    return DSP_ERR_FATAL_ERROR;
  }

  unsigned long outChannelFlags = 0x0;
  int outputChannelAmount = 0;
  outputChannelAmount = m_addon->MasterProcessGetOutChannels(&m_handle, outChannelFlags);
  OutputFormat.m_channelLayout.Reset();
  for (unsigned int ch = 0; ch < AE_DSP_CH_MAX; ch++)
  {
    if (outChannelFlags & 1 << ch)
    {
      OutputFormat.m_channelLayout += static_cast<AEChannel>(ch + 1); //! @todo AudioDSP V2 add conversion method
    }
  }

  return DSP_ERR_NO_ERR;
}

int CAudioDSPAddonModeNode::ProcessInstance(const uint8_t **In, uint8_t **Out)
{
  return m_addon->MasterProcess(&m_handle, reinterpret_cast<const float**>(In), reinterpret_cast<float**>(Out), m_InputFormat.m_frames);
}

DSPErrorCode_t CAudioDSPAddonModeNode::DestroyInstance()
{
  return DSP_ERR_NO_ERR;
}
