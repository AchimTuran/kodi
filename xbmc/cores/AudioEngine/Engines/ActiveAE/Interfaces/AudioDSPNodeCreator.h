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


#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNode.h"

namespace DSP
{
class IDSPNodeCreator
{
public:
  virtual ~IDSPNodeCreator() {}
  typedef IDSPNodeCreator*(*NodeCreatorCallback)(void);

  virtual AUDIO::IADSPNode* InstantiateNode(uint64_t ID) = 0;
  virtual DSPErrorCode_t DestroyNode(AUDIO::IADSPNode *&Node) = 0;
};

class IDSPNodeCreatorFactory
{
public:
  virtual IDSPNodeCreator* CreateCreator() = 0;
};

template<class T>
class TDSPNodeCreator : public IDSPNodeCreator, public IDSPNodeCreatorFactory
{
  virtual IDSPNodeCreator* CreateCreator() override { return dynamic_cast<IDSPNodeCreator*>(new T); }
};
}
