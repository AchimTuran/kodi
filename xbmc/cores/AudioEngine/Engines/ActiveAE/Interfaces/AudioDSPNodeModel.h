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

#include <stdint.h>
#include <vector>

#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNodeCreator.h"


namespace DSP
{
class IDSPNodeModel
{
public:
  class CDSPNodeInfo
  {
  public:
    CDSPNodeInfo(std::string Name, bool Active) :
      ID(0),
      Name(Name),
      Active(Active)
    {
    }

    CDSPNodeInfo(std::string Name, uint64_t ID, bool Active) :
      ID(ID),
      Name(Name),
      Active(Active)
    {
    }

    CDSPNodeInfo(const CDSPNodeInfo &RHS) :
      ID(RHS.ID),
      Name(RHS.Name),
      Active(RHS.Active)
    {
    }
    
    CDSPNodeInfo() :
      ID(0),
      Name(""),
      Active(false)
    {
    }

    const uint64_t    ID;
    const std::string Name;
    const bool        Active;
  };

  class CDSPNodeInfoQuery
  {
  public:
    CDSPNodeInfoQuery(std::vector<std::string> NameStrings) :
      NameVector(NameStrings)
    {
    }

    CDSPNodeInfoQuery(const CDSPNodeInfoQuery &RHS) :
      NameVector(RHS.NameVector)
    {
    }

    const std::vector<std::string> NameVector;
  };

  typedef std::vector<CDSPNodeInfo> DSPNodeInfoVector_t;

  // model task
  // - ordered node chain
  // - enabled nodes are at the beginning of the chain
  // - node creators management
  // - enable/disable nodes
  // - node position

  // node registering
  virtual DSPErrorCode_t RegisterNode(const CDSPNodeInfoQuery &Node, IDSPNodeCreatorFactory &Factory) = 0;
  virtual DSPErrorCode_t DeregisterNode(uint64_t ID) = 0;
  
  // node infos
  virtual CDSPNodeInfo GetNodeInfo(const CDSPNodeInfoQuery &Node) = 0;
  virtual DSPErrorCode_t GetNodeInfos(DSPNodeInfoVector_t &NodeInfos) = 0;
  virtual DSPErrorCode_t GetActiveNodes(DSPNodeInfoVector_t &ActiveNodeInfos) = 0;
  
  // node managing
  virtual DSPErrorCode_t DisableNode(uint64_t ID) = 0;
  // Position == 0 --> adds node to the end of active nodes
  virtual DSPErrorCode_t EnableNode(uint64_t ID, uint32_t Position = 0) = 0;
};
}
