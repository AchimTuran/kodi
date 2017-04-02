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

#pragma once

#include "cores/DSP/Models/Interfaces/IDSPNodeModel.h"
#include "cores/DSP/Models/Interfaces/IDSPNodeModelCallback.h"


namespace ActiveAE
{
class CAudioDSPController
{
public:
  CAudioDSPController(DSP::IDSPNodeModel &DSPNodeModel);

  // node infos
  DSP::IDSPNodeModel::CDSPNodeInfo GetNodeInfo(DSP::IDSPNodeModel::CDSPNodeInfoQuery &Node);
  DSPErrorCode_t GetNodeInfos(DSP::IDSPNodeModel::DSPNodeInfoVector_t &NodeInfos);
  DSPErrorCode_t GetActiveNodes(DSP::IDSPNodeModel::DSPNodeInfoVector_t &ActiveNodeInfos);

  // node managing
  DSPErrorCode_t DisableNode(uint64_t ID);
  // Position == 0 --> adds node to the end of active nodes
  DSPErrorCode_t EnableNode(uint64_t ID, uint32_t Position = 0);

private:
  DSP::IDSPNodeModel &m_DSPNodeModel;
};
}
