#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/KodiModes/AudioDSPCopyMode.h"
#include <string.h>

using namespace DSP;
using namespace DSP::AUDIO;


namespace ActiveAE
{
CAudioDSPCopyModeCreator::CAudioDSPCopyModeCreator()
{
}

IADSPNode *CAudioDSPCopyModeCreator::InstantiateNode(uint64_t ID)
{
  CAudioDSPCopyMode *copyMode = new CAudioDSPCopyMode(ID);
  IADSPNode *node = dynamic_cast<IADSPNode*>(copyMode);

  if (!node)
  {
    delete copyMode;
  }

  return node;
}

DSPErrorCode_t CAudioDSPCopyModeCreator::DestroyNode(IADSPNode *&Node)
{
  DSPErrorCode_t err = DSP_ERR_INVALID_INPUT;
  if (Node)
  {
    err = Node->DestroyInstance();

    delete Node;
    Node = nullptr;
  }

  return err;
}


CAudioDSPCopyMode::CAudioDSPCopyMode(uint64_t ID) :
  IADSPNode("CAudioDSPCopyMode", ID, ADSP_DataFormatFlagFloat) //! @todo set format flags with |
{
}

DSPErrorCode_t CAudioDSPCopyMode::CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat, void *Options/* = nullptr*/)
{
  InputFormat.m_dataFormat = AE_FMT_FLOATP;
  OutputFormat.m_dataFormat = AE_FMT_FLOATP;
  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPCopyMode::DestroyInstance()
{
  m_InputFormat.m_channelLayout.Reset();
  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPCopyMode::ProcessInstance(void *In, void *Out)
{
  uint8_t **in = reinterpret_cast<uint8_t**>(In);
  uint8_t **out = reinterpret_cast<uint8_t**>(Out);

  if (m_InputFormat.m_dataFormat == m_OutputFormat.m_dataFormat)
  {
    for (uint8_t ch = 0; ch < m_InputFormat.m_channelLayout.Count(); ch++)
    {
      for (uint32_t ii = 0; ii < m_InputFormat.m_frames * m_InputFormat.m_frameSize / m_InputFormat.m_channelLayout.Count(); ii++)
      {
        out[ch][ii] = in[ch][ii];
      }
    }
  }

  return DSP_ERR_NO_ERR;
}
}
