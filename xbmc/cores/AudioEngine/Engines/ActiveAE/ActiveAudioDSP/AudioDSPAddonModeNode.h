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
#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPBufferNode.h"


namespace ActiveAE
{
class CActiveAEDSPAddon;
typedef std::shared_ptr<ActiveAE::CActiveAEDSPAddon>    AE_DSP_ADDON;

class CAudioDSPAddonModeNode : public DSP::AUDIO::IADSPBufferNode
{
public:
  CAudioDSPAddonModeNode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, ADDON_HANDLE_STRUCT &Handle, AE_DSP_ADDON Addon, uint64_t ID, int32_t AddonModeID);

  virtual DSPErrorCode_t CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat) override;
  virtual int ProcessInstance(const uint8_t **In, uint8_t **Out) override;
  virtual DSPErrorCode_t DestroyInstance() override;

  ADDON_HANDLE_STRUCT &m_handle;

private:
  AE_DSP_ADDON m_addon; //! @todo m_Addon is easier, but call history is bigger
  int m_streamID;
};
}
