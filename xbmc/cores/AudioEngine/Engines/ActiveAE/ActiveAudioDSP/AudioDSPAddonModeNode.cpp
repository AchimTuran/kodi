#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonModeNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"

using namespace DSP;
using namespace DSP::AUDIO;
using namespace ActiveAE;

CAudioDSPAddonModeNode::CAudioDSPAddonModeNode(AE_DSP_ADDON Addon, uint64_t ID, int32_t AddonModeID) :
  m_addon(Addon),
  IADSPBufferNode(Addon->ID(), ID)
{
  memset(&m_handle, 0, sizeof(ADDON_HANDLE_STRUCT));
  //memset(&m_DllFunctions, 0, sizeof(m_DllFunctions));
}

DSPErrorCode_t CAudioDSPAddonModeNode::CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat)
{
  InputFormat.m_dataFormat = AE_FMT_FLOATP;
  OutputFormat.m_dataFormat = AE_FMT_FLOATP;
  //if (!m_Addon->GetAddonProcessingCallbacks(m_DllFunctions))
  //{
  //  return DSP_ERR_FATAL_ERROR;
  //}

  //! @todo AudioDSP V2 simplify add-on mode creation API
  AE_DSP_SETTINGS addonSettings;
  addonSettings.iStreamID;                /*!< @brief id of the audio stream packets */
  addonSettings.iStreamType;              /*!< @brief the input stream type source eg, Movie or Music */
  addonSettings.iInChannels;              /*!< @brief the amount of input channels */
  addonSettings.lInChannelPresentFlags;   /*!< @brief the exact channel mapping flags of input */
  addonSettings.iInFrames;                /*!< @brief the input frame size from KODI */
  addonSettings.iInSamplerate;            /*!< @brief the basic sample rate of the audio packet */
  addonSettings.iProcessFrames;           /*!< @brief the processing frame size inside add-on's */
  addonSettings.iProcessSamplerate;       /*!< @brief the sample rate after input resample present in master processing */
  addonSettings.iOutChannels;             /*!< @brief the amount of output channels */
  addonSettings.lOutChannelPresentFlags;  /*!< @brief the exact channel mapping flags for output */
  addonSettings.iOutFrames;               /*!< @brief the final out frame size for KODI */
  addonSettings.iOutSamplerate;           /*!< @brief the final sample rate of the audio packet */
  addonSettings.bInputResamplingActive;   /*!< @brief if a re-sampling is performed before master processing this flag is set to true */
  addonSettings.bStereoUpmix;             /*!< @brief true if the stereo upmix setting on kodi is set */
  addonSettings.iQualityLevel;            /*!< @brief the from KODI selected quality level for signal processing */

  AE_DSP_STREAM_PROPERTIES pProperties;
  pProperties.iStreamID;
  pProperties.iStreamType;
  pProperties.iBaseType;
  pProperties.strName;
  pProperties.strCodecId;
  pProperties.strLanguage;
  pProperties.iIdentifier;
  pProperties.iChannels;
  pProperties.iSampleRate;
  pProperties.Profile;

  DSPErrorCode_t dspErr = DSP_ERR_NO_ERR;
  if (m_addon->StreamInitialize(&m_handle, &addonSettings) != AE_DSP_ERROR_NO_ERROR)
  {
    dspErr = DSP_ERR_FATAL_ERROR;
  }

  if (m_addon->StreamCreate(&addonSettings, &pProperties, &m_handle) != AE_DSP_ERROR_NO_ERROR)
  {
    dspErr = DSP_ERR_FATAL_ERROR;
  }

  return dspErr;
}

int CAudioDSPAddonModeNode::ProcessInstance(const uint8_t **In, uint8_t **Out)
{
  return m_addon->MasterProcess(&m_handle, reinterpret_cast<const float**>(In), reinterpret_cast<float**>(Out), m_InputFormat.m_frames);
}

DSPErrorCode_t CAudioDSPAddonModeNode::DestroyInstance()
{
  m_addon->StreamDestroy(&m_handle);
  return DSP_ERR_NO_ERR;
}
