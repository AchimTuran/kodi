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

#include "AudioDSPController.h"

using namespace ActiveAE;
using namespace DSP;


CAudioDSPController::CAudioDSPController(IDSPNodeModel &DSPNodeModel) :
  m_DSPNodeModel(DSPNodeModel)
{
}

IDSPNodeModel::CDSPNodeInfo CAudioDSPController::GetNodeInfo(IDSPNodeModel::CDSPNodeInfoQuery &Node)
{
  return m_DSPNodeModel.GetNodeInfo(Node);
}

DSPErrorCode_t CAudioDSPController::GetNodeInfos(IDSPNodeModel::DSPNodeInfoVector_t &NodeInfos)
{
  return m_DSPNodeModel.GetNodeInfos(NodeInfos);
}

DSPErrorCode_t CAudioDSPController::GetActiveNodes(IDSPNodeModel::DSPNodeInfoVector_t &ActiveNodeInfos)
{
  return m_DSPNodeModel.GetActiveNodes(ActiveNodeInfos);
}

DSPErrorCode_t CAudioDSPController::DisableNode(uint64_t ID)
{
  return m_DSPNodeModel.DisableNode(ID);
}

DSPErrorCode_t CAudioDSPController::EnableNode(uint64_t ID, uint32_t Position)
{
  return m_DSPNodeModel.EnableNode(ID, Position);
}

DSPErrorCode_t CAudioDSPController::EnableNodeCallback(uint64_t ID, uint32_t Position)
{
  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPController::DisableNodeCallback(uint64_t ID)
{
  return DSP_ERR_NO_ERR;
}
