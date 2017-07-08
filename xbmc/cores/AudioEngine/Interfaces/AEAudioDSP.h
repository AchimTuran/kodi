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
#include "addons/IAddon.h"
 
class IAEAudioDSP
{
public:
  IAEAudioDSP() {}
  virtual ~IAEAudioDSP() {}
  
  virtual void EnableAddon(const std::string& Id, bool Enable) = 0;
  virtual bool GetAddon(const std::string& Id, ADDON::AddonPtr &addon) = 0;

  virtual void RegisterAddon(const std::string& Id, bool restart = false, bool update = false) = 0;
  virtual void UnregisterAddon(const std::string& Id) = 0;
};
