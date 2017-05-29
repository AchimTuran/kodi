#include "AudioDSPAddonModeNode.h"

using namespace DSP;
using namespace DSP::AUDIO;


namespace ActiveAE
{
CAudioDSPAddonModeNode::CAudioDSPAddonModeNode(AE_DSP_ADDON Addon, uint64_t ID, int32_t AddonModeID) :
  m_Addon(Addon),
  IADSPNode(Name, ID, ADSP_DataFormatFlagFloat)
{
  memset(&m_DllFunctions, 0, sizeof(m_DllFunctions));
}

DSPErrorCode_t CAudioDSPAddonModeNode::Create(const AEAudioFormat &InputProperties, const AEAudioFormat &OutputProperties)
{
  if (!m_Addon->GetAddonProcessingCallbacks(m_DllFunctions))
  {
    return DSP_ERR_FATAL_ERROR;
  }

  //! @todo AudioDSP V2 simplify add-on mode creation API

  return DSP_ERR_NO_ERR;
}

bool CAudioDSPAddonModeNode::Process()
{
  if (!m_DllFunctions.PostProcess)
  {
    return false;
  }

  //m_DllFunctions.PostProcess(nullptr, 0, In, Out, 0); //! @todo AudioDSP V2 change API to PostProcess(HANDLE, In, Out), size and ID is set during creation

  return true;
}

DSPErrorCode_t CAudioDSPAddonModeNode::Destroy()
{
  return DSP_ERR_NO_ERR;
}
}
