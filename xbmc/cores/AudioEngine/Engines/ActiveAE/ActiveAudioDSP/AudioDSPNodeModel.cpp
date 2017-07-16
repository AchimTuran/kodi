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

#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPNodeModel.h"
#include "utils/log.h"

using namespace std;
using namespace DSP;
using namespace DSP::AUDIO;

CDSPNodeModel::~CDSPNodeModel()
{
  CSingleLock lock(m_mutex);
  for (NodeInfoVector_t::iterator iter = m_Nodes.begin(); iter != m_Nodes.end(); ++iter)
  {
    if (iter->NodeCreator)
    {
      delete iter->NodeCreator;
      iter->NodeCreator = nullptr;
    }
  }

  m_Nodes.clear();
}

DSPErrorCode_t CDSPNodeModel::RegisterNode(const CDSPNodeInfoQuery &Node, IDSPNodeCreatorFactory &Factory)
{
  CSingleLock lock(m_mutex);

  if (Node.NameVector.size() < 2)
  {
    return DSP_ERR_INVALID_NODE_ID;
  }

  string addonName = Node.NameVector.at(0);
  string modeName = Node.NameVector.at(1);
  string instanceModeName = "";
  if (Node.NameVector.size() > 2)
  {
    instanceModeName = Node.NameVector.at(2);
  }


  if (GetNodeData(addonName, modeName, instanceModeName) != m_Nodes.end())
  {
    return DSP_ERR_NODE_ALREADY_REGISTERED;
  }

  CNodeStringInfo id(addonName, modeName, instanceModeName);
  if (id == 0x0)
  {
    return DSP_ERR_INVALID_INPUT;
  }

  IDSPNodeCreator *creator = Factory.CreateCreator();
  if (!creator)
  {
    return DSP_ERR_INVALID_INPUT;
  }

  m_Nodes.push_back(NodeInfo_t(id, false, creator, addonName, modeName, instanceModeName));
  CLog::Log(LOGDEBUG, "%s - successful registered signal processing node creator for mode %s from add-on %s with ModeID %i, ModeInstanceID %i and UniqueAddonID &i", __FUNCTION__, modeName.c_str(), addonName.c_str(), id.ModeID, id.ModeInstanceID, id.UniqueAddonID);

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CDSPNodeModel::DeregisterNode(uint64_t ID)
{
  CSingleLock lock(m_mutex);

  NodeInfoVector_t::iterator iter = GetNodeData(ID);
  if (iter == m_Nodes.end())
  {
    return DSP_ERR_NODE_NOT_FOUND;
  }

  if (iter->NodeCreator)
  {
    delete iter->NodeCreator;
    iter->NodeCreator = nullptr;
  }
  m_Nodes.erase(iter);

  return DSP_ERR_NO_ERR;
}

IDSPNodeModel::CDSPNodeInfo CDSPNodeModel::GetNodeInfo(const CDSPNodeInfoQuery &Node)
{
  CSingleLock lock(m_mutex);

  string nodeStr = GenerateNodeString(Node.NameVector);
  if (Node.NameVector.size() < 2 || Node.NameVector.size() > 3)
  {
    return CDSPNodeInfo();
  }

  string addonName = Node.NameVector.at(0);
  string modeName = Node.NameVector.at(1);
  string instanceModeName = Node.NameVector.size() == 3 ? instanceModeName = Node.NameVector.at(2) : "";
  NodeID_t id = 0x0;
  bool isActive = false;

  for (unsigned int ii = 0; ii < m_Nodes.size(); ii++)
  {
    if (addonName == m_Nodes.at(ii).AddonName && modeName == m_Nodes.at(ii).ModeName && instanceModeName == m_Nodes.at(ii).InstanceModeName)
    {
      nodeStr = GenerateNodeString(addonName, modeName, instanceModeName);
      id = m_Nodes.at(ii).ID;
      isActive = m_Nodes.at(ii).Active;
    }
  }

  if (nodeStr.empty() || id == 0x0)
  {
    return CDSPNodeInfo();
  }

  return CDSPNodeInfo(nodeStr, id, isActive);
}

DSPErrorCode_t CDSPNodeModel::GetNodeInfos(DSPNodeInfoVector_t &NodeInfos)
{
  NodeInfos.clear();
  string nodeStr;
  for (NodeInfoVector_t::iterator iter = m_Nodes.begin(); iter != m_Nodes.end(); ++iter)
  {
    nodeStr = GenerateNodeString(iter->AddonName, iter->ModeName, iter->InstanceModeName);
    NodeInfos.push_back(CDSPNodeInfo(nodeStr, iter->ID, iter->Active));
  }

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CDSPNodeModel::GetActiveNodes(DSPNodeInfoVector_t &ActiveNodeInfos)
{
  CSingleLock lock(m_mutex);

  ActiveNodeInfos.clear();
  string nodeStr;
  for (std::vector<uint64_t>::iterator iter = m_ActiveNodes.begin(); iter != m_ActiveNodes.end(); ++iter)
  {
    NodeInfoVector_t::iterator nodeIter = GetNodeData(*iter);
    if (nodeIter == m_Nodes.end())
    {
      return DSP_ERR_FATAL_ERROR;
    }

    nodeStr = nodeIter->AddonName;
    nodeStr += "::";
    nodeStr += nodeIter->ModeName;
    if (!nodeIter->InstanceModeName.empty())
    {
      nodeStr += "::";
      nodeStr += nodeIter->InstanceModeName;
    }

    ActiveNodeInfos.push_back(CDSPNodeInfo(nodeStr, nodeIter->ID, nodeIter->Active));
  }

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CDSPNodeModel::EnableNode(uint64_t ID, uint32_t Position)
{
  CSingleLock lock(m_mutex);

  NodeInfoVector_t::iterator nodeIter = GetNodeData(ID);
  if (nodeIter == m_Nodes.end())
  {
    return DSP_ERR_NODE_NOT_FOUND;
  }

  if (nodeIter->Active)
  {
    RemoveActiveNode(ID);
  }
  nodeIter->Active = true;
  m_ActiveNodes.insert(m_ActiveNodes.begin() + Position, ID);
  NotifyEnableNodeUpdate(ID, Position);

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CDSPNodeModel::DisableNode(uint64_t ID)
{
  CSingleLock lock(m_mutex);

  NodeInfoVector_t::iterator nodeIter = GetNodeData(ID);
  if (nodeIter == m_Nodes.end())
  {
    return DSP_ERR_NODE_NOT_FOUND;
  }

  nodeIter->Active = false;
  RemoveActiveNode(ID);
  NotifyDisableNodeUpdate(ID); //! @todo how to handle error code?

  return DSP_ERR_NO_ERR;
}


// factory interface
IADSPNode* CDSPNodeModel::InstantiateNode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, const AEStreamProperties &StreamProperties, uint64_t ID)
{
  CSingleLock lock(m_mutex);

  NodeInfoVector_t::iterator iter = GetNodeData(ID);
  if (iter == m_Nodes.end())
  {
    return nullptr;
  }

  return iter->NodeCreator->InstantiateNode(InputFormat, OutputFormat, StreamProperties, ID);
}

DSPErrorCode_t CDSPNodeModel::DestroyNode(IADSPNode *&Node)
{
  CSingleLock lock(m_mutex);

  NodeInfoVector_t::iterator iter = GetNodeData(Node->ID);
  if (iter == m_Nodes.end())
  {
    return DSP_ERR_CREATOR_ID_NOT_FOUND;
  }

  return iter->NodeCreator->DestroyNode(Node);
}

void CDSPNodeModel::NotifyEnableNodeUpdate(uint64_t ID, uint32_t Position)
{
}

void CDSPNodeModel::NotifyDisableNodeUpdate(uint64_t ID)
{
}
