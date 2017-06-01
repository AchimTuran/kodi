#pragma once
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


#include <string>
#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNode.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPAddon.h"


namespace ActiveAE
{
class CAudioDSPAddonModeNode : public DSP::AUDIO::IADSPNode
{
public:
  CAudioDSPAddonModeNode(AE_DSP_ADDON Addon, uint64_t ID, int32_t AddonModeID);

  virtual DSPErrorCode_t Create(const AEAudioFormat &InputProperties, const AEAudioFormat &OutputProperties) override;
  virtual bool Process() override;
  virtual DSPErrorCode_t Destroy() override;

private:
  AE_DSP_ADDON m_Addon; //! @todo m_Addon is easier, but call history is bigger
  AudioDSP m_DllFunctions; //! @todo m_DllFunctions is more complex, but call history is smaller
};
}
