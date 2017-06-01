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
 

typedef enum
{
  DSP_ERR_NODE_ID_ALREADY_REGISTERED    = -10,
  DSP_ERR_CREATOR_ID_NOT_FOUND          = -9,
  DSP_ERR_ALREADY_REGISTERED            = -8,
  DSP_ERR_NODE_NOT_FOUND                = -7,
  DSP_ERR_NODE_ALREADY_REGISTERED       = -6,
  DSP_ERR_INVALID_NODE_ID               = -5,
  DSP_ERR_FATAL_ERROR                   = -4,
  DSP_ERR_INVALID_DATA_FORMAT           = -3,
  DSP_ERR_NOT_IMPLEMENTED               = -2,
  DSP_ERR_INVALID_INPUT                 = -1,
  DSP_ERR_NO_ERR                        = 0
}DSPErrorCode_t;

typedef enum
{
  DSP_CATEGORY_UNKNOWN,

  DSP_CATEGORY_Audio = 0,
  DSP_CATEGORY_Image,

  DSP_CATEGORY_MAX
}DSPCategory_t;

typedef struct NodeID_t
{
  uint32_t UniqueAddonID;
  uint16_t ModeID;
  uint16_t ModeInstanceID;

  //NodeID_t &operator=(IDSPChainModel::DSPNodeInfo_t &NodeInfo)
  //{
  //  UniqueAddonID   = (uint32_t)(0xFFFFFFFF  &  NodeInfo.ID);
  //  ModeID          = (uint16_t)(0xFFFF      & (NodeInfo.ID >> 32));
  //  ModeInstanceID  = (uint16_t)(0xFFFF      & (NodeInfo.ID >> 48));
  //  return *this;
  //}

  NodeID_t &operator=(const NodeID_t &ID)
  {
    UniqueAddonID   = ID.UniqueAddonID;
    ModeID          = ID.ModeID;
    ModeInstanceID  = ID.ModeInstanceID;

    return *this;
  }

  operator uint64_t()
  {
    return (uint64_t)ModeInstanceID << 48 | (uint64_t)ModeID << 32 | (uint64_t)UniqueAddonID;
  }
  
  NodeID_t(uint64_t ID)
  {
    UniqueAddonID   = (uint32_t)(0xFFFFFFFF  &  ID);
    ModeID          = (uint16_t)(0xFFFF      & (ID >> 32));
    ModeInstanceID  = (uint16_t)(0xFFFF      & (ID >> 48));
  }

  NodeID_t(uint32_t UniqueAddonID, uint16_t ModeID, uint16_t ModeInstanceID) :
    UniqueAddonID(UniqueAddonID),
    ModeID(ModeID),
    ModeInstanceID(ModeInstanceID)
  {
  }

  NodeID_t()
  {
    UniqueAddonID   = 0;
    ModeID          = 0;
    ModeInstanceID  = 0;
  }
}NodeID_t;  
