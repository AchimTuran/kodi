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
#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNodeCreator.h"
#include "addons/kodi-addon-dev-kit/include/kodi/addon-instance/AudioDSP.h"


namespace ActiveAE
{
class CAudioDSPCopyModeCreator : public DSP::TDSPNodeCreator<CAudioDSPCopyModeCreator>
{
public:
  CAudioDSPCopyModeCreator();

  virtual DSP::AUDIO::IADSPNode* InstantiateNode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, const AEStreamProperties &StreamProperties, uint64_t ID) override;
  virtual DSPErrorCode_t DestroyNode(DSP::AUDIO::IADSPNode *&Node) override;
};


class CAudioDSPCopyMode : public DSP::AUDIO::IADSPBufferNode
{
  CAudioDSPCopyMode() : IADSPBufferNode("CAudioDSPCopyMode", 0) {} // hide default constructor to prevent creation without a valid ID
public:
  CAudioDSPCopyMode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, uint64_t ID);

  virtual DSPErrorCode_t CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat) override;
  virtual int ProcessInstance(const uint8_t **In, uint8_t **Out) override;
  virtual DSPErrorCode_t DestroyInstance() override;
};
}
