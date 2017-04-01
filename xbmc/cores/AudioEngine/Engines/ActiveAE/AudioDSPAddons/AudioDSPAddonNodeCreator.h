#pragma once


#include "cores/DSP/Factory/Interfaces/IDSPNodeCreator.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"

namespace ActiveAE
{
class CAudioDSPAddonNodeCreator : public DSP::IDSPNodeCreator
{
public:
  CAudioDSPAddonNodeCreator(const AE_DSP_ADDON &Addon);

  virtual DSP::IDSPNode* InstantiateNode(uint64_t ID) override;
  virtual DSPErrorCode_t DestroyNode(DSP::IDSPNode *&Node) override;

private:
  const AE_DSP_ADDON m_Addon;
};
}
