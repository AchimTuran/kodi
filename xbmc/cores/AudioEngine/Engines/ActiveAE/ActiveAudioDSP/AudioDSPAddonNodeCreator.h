#pragma once


#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNodeCreator.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"

#include <map>

namespace ActiveAE
{
class CAudioDSPAddonNodeCreator : public DSP::IDSPNodeCreator, public DSP::IDSPNodeCreatorFactory
{
  typedef struct AddonStreamHandle_t
  {
    ADDON_HANDLE_STRUCT handle;
    std::map<unsigned int, unsigned int> modes; // first=mode number, second=mode count

    AddonStreamHandle_t()
    {
      memset(&handle, 0, sizeof(ADDON_HANDLE_STRUCT));
    }
  }AddonStreamHandle_t;
  typedef std::map<unsigned int, AddonStreamHandle_t> AudioDSPAddonStreamMap_t;

public:
  CAudioDSPAddonNodeCreator(const AE_DSP_ADDON &Addon);

  virtual DSP::AUDIO::IADSPNode* InstantiateNode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, const AEStreamProperties &StreamProperties, uint64_t ID) override;
  virtual DSPErrorCode_t DestroyNode(DSP::AUDIO::IADSPNode *&Node) override;

private:
  virtual IDSPNodeCreator* CreateCreator() override;

  AudioDSPAddonStreamMap_t m_addonModeMap;
  const AE_DSP_ADDON m_addon;
};
}
