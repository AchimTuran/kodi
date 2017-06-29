#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonModeNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"

using namespace DSP;
using namespace DSP::AUDIO;
using namespace ActiveAE;

CAudioDSPAddonModeNode::CAudioDSPAddonModeNode(ADDON_HANDLE_STRUCT &Handle, AE_DSP_ADDON Addon, uint64_t ID, int StreamID) :
  m_addon(Addon),
  IADSPBufferNode(Addon->ID(), ID),
  m_handle(Handle)
{
  memset(&m_handle, 0, sizeof(ADDON_HANDLE_STRUCT));
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
  addonSettings.iInFrames = InputFormat.m_frames;                /*!< @brief the input frame size from KODI */
  addonSettings.iInSamplerate = InputFormat.m_sampleRate;            /*!< @brief the basic sample rate of the audio packet */
  addonSettings.iProcessFrames = InputFormat.m_frames;           /*!< @brief the processing frame size inside add-on's */
  addonSettings.iProcessSamplerate = InputFormat.m_sampleRate;       /*!< @brief the sample rate after input resample present in master processing */
  addonSettings.iOutChannels = OutputFormat.m_channelLayout.Count();             /*!< @brief the amount of output channels */
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
  if (m_addon->MasterProcessSetMode(&m_handle, addonStreamType, addonModeID, ID) != AE_DSP_ERROR_NO_ERROR)
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
      OutputFormat.m_channelLayout += static_cast<AEChannel>(ch); //! @todo AudioDSP V2 add conversion method
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
