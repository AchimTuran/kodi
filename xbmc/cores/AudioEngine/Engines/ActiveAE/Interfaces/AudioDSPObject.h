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
#include <stdint.h>

#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPTypes.h"


namespace DSP
{
class DSPObject
{
public:
  DSPObject(const std::string &Name, uint64_t ID, DSPCategory_t DSPCategory) :
    Name(Name), 
    ID(ID),
    DSPCategory(DSPCategory)
  {
  }
  
  const std::string     Name;
  const uint64_t        ID;
  const DSPCategory_t   DSPCategory;
};
}
