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


#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonNodeCreator.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPAddonModeNode.h"

using namespace DSP;
using namespace DSP::AUDIO;

namespace ActiveAE
{
CAudioDSPAddonNodeCreator::CAudioDSPAddonNodeCreator(const AE_DSP_ADDON &Addon) :
  m_addon(Addon)
{
}

DSPErrorCode_t CAudioDSPAddonNodeCreator::DestroyNode(IADSPNode *&Node)
{
  if (!Node)
  {
    return DSP_ERR_INVALID_INPUT;
  }

  DSPErrorCode_t err = Node->Destroy();

  delete Node;
  Node = nullptr;

  return err;
}

IDSPNodeCreator* CAudioDSPAddonNodeCreator::CreateCreator()
{
  return dynamic_cast<IDSPNodeCreator*>(new CAudioDSPAddonNodeCreator(m_addon));
}

IADSPNode* CAudioDSPAddonNodeCreator::InstantiateNode(uint64_t ID)
{
  IADSPNode *node = dynamic_cast<IADSPNode*>(new CAudioDSPAddonModeNode(m_addon, ID, 0)); //! @todo use <add-on name>::<mode name> as ID identifier generation and add-on mode ID
  if (!node)
  {
    return nullptr;
  }

  return node;
}
}
