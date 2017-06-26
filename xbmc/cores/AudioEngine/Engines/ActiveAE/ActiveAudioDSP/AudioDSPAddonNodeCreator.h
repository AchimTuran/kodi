#pragma once


#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNodeCreator.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"

namespace ActiveAE
{
class CAudioDSPAddonNodeCreator : public DSP::IDSPNodeCreator, public DSP::IDSPNodeCreatorFactory
{
public:
  CAudioDSPAddonNodeCreator(const AE_DSP_ADDON &Addon);

  virtual DSP::AUDIO::IADSPNode* InstantiateNode(uint64_t ID) override;
  virtual DSPErrorCode_t DestroyNode(DSP::AUDIO::IADSPNode *&Node) override;

private:
  virtual IDSPNodeCreator* CreateCreator() override;

  const AE_DSP_ADDON m_addon;
};
}
