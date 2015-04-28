#pragma once
/*
 *      Copyright (C) 2010-2015 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <stdint.h>
#include <string>
#include <vector>

#include "cores/AudioEngine/Utils/AEDeviceInfo.h"

class IAESource;

typedef struct
{
  std::string      m_sourceName;
  AEDeviceInfoList m_deviceInfoList;
} AESourceInfo;

typedef std::vector<AESourceInfo> AESourceInfoList;

class CAESourceFactory
{
public:
  static void       ParseDevice(std::string &device, std::string &driver);
  static IAESource  *Create(std::string &device, AEAudioFormat &desiredFormat, bool rawPassthrough);
  static void       EnumerateEx(AESourceInfoList &list, bool force = false); /* The force flag can be used to indicate the rescan devices */

protected:
  static IAESource *TrySource(std::string &driver, std::string &device, AEAudioFormat &format);
};
