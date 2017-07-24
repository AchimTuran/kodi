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


#include "addons/kodi-addon-dev-kit/include/kodi/addon-instance/AudioDSP.h"
#include "cores/AudioEngine/Utils/AEAudioFormat.h"
#include "cores/AudioEngine/Utils/AEStreamProperties.h"


namespace ActiveAE
{
class CAudioDSPAddonUtil
{
public:
  // from AE to Add-On
  static bool AEAudioFormat_TO_AddonAudioFormat(const AEAudioFormat &AEFormat, AUDIODSP_ADDON_AUDIO_FORMAT &AddonAudioFormat);
  static bool AEDataFormat_TO_AddonAudioDataFormat(const AEDataFormat &DataFormat, AUDIODSP_ADDON_AUDIO_DATA_FORMAT &AddonDataFormat);
  static bool AEChannelInfo_TO_AddonChannelInfo(const CAEChannelInfo &ChannelInfo, AUDIODSP_ADDON_CHANNEL_INFO &AddonChannelInfo);
  static bool AEChannel_TO_AddonChannel(const AEChannel &Channel, AUDIODSP_ADDON_CHANNEL &AddonChannel);
  
  // from Add-On to AE
  static bool AddonAudioFormat_TO_AEAudioFormat(const AUDIODSP_ADDON_AUDIO_FORMAT &AddonAudioFormat, AEAudioFormat &AEFormat);
  static bool AddonAudioDataFormat_TO_AEDataFormat(const AUDIODSP_ADDON_AUDIO_DATA_FORMAT &AddonDataFormat, AEDataFormat &DataFormat);
  static bool AddonChannelInfo_TO_AEChannelInfo(const AUDIODSP_ADDON_CHANNEL_INFO &AddonChannelInfo, CAEChannelInfo &ChannelInfo);
  static bool AddonChannel_TO_AEChannel(const AUDIODSP_ADDON_CHANNEL &AddonChannel, AEChannel &Channel);

  static bool AEStreamProperties_TO_AddonStreamProperties(const AEStreamProperties &AEStreamProperties, AUDIODSP_ADDON_STREAM_PROPERTIES &AddonStreamProperties);
};
}
