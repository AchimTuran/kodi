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



#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AddOns/AudioDSPAddonUtils.h"

namespace ActiveAE
{
bool CAudioDSPAddonUtil::AEAudioFormat_TO_AddonAudioFormat(const AEAudioFormat &AEFormat, AUDIODSP_ADDON_AUDIO_FORMAT &AddonAudioFormat)
{
  if (!AEDataFormat_TO_AddonAudioDataFormat(AEFormat.m_dataFormat, AddonAudioFormat.dataFormat))
  {
    return false;
  }

  if (!AEChannelInfo_TO_AddonChannelInfo(AEFormat.m_channelLayout, AddonAudioFormat.channelLayout))
  {
    return false;
  }

  AddonAudioFormat.sampleRate = AEFormat.m_sampleRate;
  AddonAudioFormat.frames = AEFormat.m_frames;
  AddonAudioFormat.frameSize = AEFormat.m_frameSize;

  return true;
}

bool CAudioDSPAddonUtil::AEDataFormat_TO_AddonAudioDataFormat(const AEDataFormat &DataFormat, AUDIODSP_ADDON_AUDIO_DATA_FORMAT &AddonDataFormat)
{
  switch (DataFormat)
  {
    case AE_FMT_U8:         AddonDataFormat = AUDIODSP_ADDON_FMT_U8;          break;
    case AE_FMT_S16BE:      AddonDataFormat = AUDIODSP_ADDON_FMT_S16BE;       break;
    case AE_FMT_S16LE:      AddonDataFormat = AUDIODSP_ADDON_FMT_S16LE;       break;
    case AE_FMT_S16NE:      AddonDataFormat = AUDIODSP_ADDON_FMT_S16NE;       break;
    case AE_FMT_S32BE:      AddonDataFormat = AUDIODSP_ADDON_FMT_S32BE;       break;
    case AE_FMT_S32LE:      AddonDataFormat = AUDIODSP_ADDON_FMT_S32LE;       break;
    case AE_FMT_S32NE:      AddonDataFormat = AUDIODSP_ADDON_FMT_S32NE;       break;
    case AE_FMT_S24BE4:     AddonDataFormat = AUDIODSP_ADDON_FMT_S24BE4;      break;
    case AE_FMT_S24LE4:     AddonDataFormat = AUDIODSP_ADDON_FMT_S24LE4;      break;
    case AE_FMT_S24NE4:     AddonDataFormat = AUDIODSP_ADDON_FMT_S24NE4;      break;
    case AE_FMT_S24NE4MSB:  AddonDataFormat = AUDIODSP_ADDON_FMT_S24NE4MSB;   break;
    case AE_FMT_S24BE3:     AddonDataFormat = AUDIODSP_ADDON_FMT_S24BE3;      break;
    case AE_FMT_S24LE3:     AddonDataFormat = AUDIODSP_ADDON_FMT_S24LE3;      break;
    case AE_FMT_S24NE3:     AddonDataFormat = AUDIODSP_ADDON_FMT_S24NE3;      break;
    case AE_FMT_DOUBLE:     AddonDataFormat = AUDIODSP_ADDON_FMT_DOUBLE;      break;
    case AE_FMT_FLOAT:      AddonDataFormat = AUDIODSP_ADDON_FMT_FLOAT;       break;
    case AE_FMT_U8P:        AddonDataFormat = AUDIODSP_ADDON_FMT_U8P;         break;
    case AE_FMT_S16NEP:     AddonDataFormat = AUDIODSP_ADDON_FMT_S16NEP;      break;
    case AE_FMT_S32NEP:     AddonDataFormat = AUDIODSP_ADDON_FMT_S32NEP;      break;
    case AE_FMT_S24NE4P:    AddonDataFormat = AUDIODSP_ADDON_FMT_S24NE4P;     break;
    case AE_FMT_S24NE4MSBP: AddonDataFormat = AUDIODSP_ADDON_FMT_S24NE4MSBP;  break;
    case AE_FMT_S24NE3P:    AddonDataFormat = AUDIODSP_ADDON_FMT_S24NE3P;     break;
    case AE_FMT_DOUBLEP:    AddonDataFormat = AUDIODSP_ADDON_FMT_DOUBLEP;     break;
    case AE_FMT_FLOATP:     AddonDataFormat = AUDIODSP_ADDON_FMT_FLOATP;      break;

    default:                AddonDataFormat = AUDIODSP_ADDON_FMT_INVALID;     return false;
  }

  return true;
}

bool CAudioDSPAddonUtil::AEChannelInfo_TO_AddonChannelInfo(const CAEChannelInfo &ChannelInfo, AUDIODSP_ADDON_CHANNEL_INFO &AddonChannelInfo)
{
  AddonChannelInfo.channelCount = ChannelInfo.Count();
  for (unsigned int ch = 0; ch < AUDIODSP_ADDON_CH_MAX + 1; ch++)
  {
    if (ch < AddonChannelInfo.channelCount)
    {
      if (!AEChannel_TO_AddonChannel(ChannelInfo[ch], AddonChannelInfo.channels[ch]))
      {
        return false;
      }
    }
    else
    {
      AddonChannelInfo.channels[ch] = AUDIODSP_ADDON_CH_INVALID;
    }
  }

  return true;
}

bool CAudioDSPAddonUtil::AEChannel_TO_AddonChannel(const AEChannel &Channel, AUDIODSP_ADDON_CHANNEL &AddonChannel)
{
  switch (Channel)
  {
    case AE_CH_FL:        AddonChannel = AUDIODSP_ADDON_CH_FL;        break;
    case AE_CH_FR:        AddonChannel = AUDIODSP_ADDON_CH_FR;        break;
    case AE_CH_FC:        AddonChannel = AUDIODSP_ADDON_CH_FC;        break;
    case AE_CH_LFE:       AddonChannel = AUDIODSP_ADDON_CH_LFE;       break;
    case AE_CH_BL:        AddonChannel = AUDIODSP_ADDON_CH_BL;        break;
    case AE_CH_BR:        AddonChannel = AUDIODSP_ADDON_CH_BR;        break;
    case AE_CH_FLOC:      AddonChannel = AUDIODSP_ADDON_CH_FLOC;      break;
    case AE_CH_FROC:      AddonChannel = AUDIODSP_ADDON_CH_FROC;      break;
    case AE_CH_BC:        AddonChannel = AUDIODSP_ADDON_CH_BC;        break;
    case AE_CH_SL:        AddonChannel = AUDIODSP_ADDON_CH_SL;        break;
    case AE_CH_SR:        AddonChannel = AUDIODSP_ADDON_CH_SR;        break;
    case AE_CH_TFL:       AddonChannel = AUDIODSP_ADDON_CH_TFL;       break;
    case AE_CH_TFR:       AddonChannel = AUDIODSP_ADDON_CH_TFR;       break;
    case AE_CH_TFC:       AddonChannel = AUDIODSP_ADDON_CH_TFC;       break;
    case AE_CH_TC:        AddonChannel = AUDIODSP_ADDON_CH_TC;        break;
    case AE_CH_TBL:       AddonChannel = AUDIODSP_ADDON_CH_TBL;       break;
    case AE_CH_TBR:       AddonChannel = AUDIODSP_ADDON_CH_TBR;       break;
    case AE_CH_TBC:       AddonChannel = AUDIODSP_ADDON_CH_TBC;       break;
    case AE_CH_BLOC:      AddonChannel = AUDIODSP_ADDON_CH_BLOC;      break;
    case AE_CH_BROC:      AddonChannel = AUDIODSP_ADDON_CH_BROC;      break;
    case AE_CH_UNKNOWN1:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN1;  break;
    case AE_CH_UNKNOWN2:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN2;  break;
    case AE_CH_UNKNOWN3:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN3;  break;
    case AE_CH_UNKNOWN4:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN4;  break;
    case AE_CH_UNKNOWN5:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN5;  break;
    case AE_CH_UNKNOWN6:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN6;  break;
    case AE_CH_UNKNOWN7:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN7;  break;
    case AE_CH_UNKNOWN8:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN8;  break;
    case AE_CH_UNKNOWN9:  AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN9;  break;
    case AE_CH_UNKNOWN10: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN10; break;
    case AE_CH_UNKNOWN11: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN11; break;
    case AE_CH_UNKNOWN12: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN12; break;
    case AE_CH_UNKNOWN13: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN13; break;
    case AE_CH_UNKNOWN14: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN14; break;
    case AE_CH_UNKNOWN15: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN15; break;
    case AE_CH_UNKNOWN16: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN16; break;
    case AE_CH_UNKNOWN17: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN17; break;
    case AE_CH_UNKNOWN18: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN18; break;
    case AE_CH_UNKNOWN19: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN19; break;
    case AE_CH_UNKNOWN20: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN20; break;
    case AE_CH_UNKNOWN21: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN21; break;
    case AE_CH_UNKNOWN22: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN22; break;
    case AE_CH_UNKNOWN23: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN23; break;
    case AE_CH_UNKNOWN24: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN24; break;
    case AE_CH_UNKNOWN25: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN25; break;
    case AE_CH_UNKNOWN26: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN26; break;
    case AE_CH_UNKNOWN27: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN27; break;
    case AE_CH_UNKNOWN28: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN28; break;
    case AE_CH_UNKNOWN29: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN29; break;
    case AE_CH_UNKNOWN30: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN30; break;
    case AE_CH_UNKNOWN31: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN31; break;
    case AE_CH_UNKNOWN32: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN32; break;
    case AE_CH_UNKNOWN33: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN33; break;
    case AE_CH_UNKNOWN34: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN34; break;
    case AE_CH_UNKNOWN35: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN35; break;
    case AE_CH_UNKNOWN36: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN36; break;
    case AE_CH_UNKNOWN37: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN37; break;
    case AE_CH_UNKNOWN38: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN38; break;
    case AE_CH_UNKNOWN39: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN39; break;
    case AE_CH_UNKNOWN40: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN40; break;
    case AE_CH_UNKNOWN41: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN41; break;
    case AE_CH_UNKNOWN42: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN42; break;
    case AE_CH_UNKNOWN43: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN43; break;
    case AE_CH_UNKNOWN44: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN44; break;
    case AE_CH_UNKNOWN45: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN45; break;
    case AE_CH_UNKNOWN46: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN46; break;
    case AE_CH_UNKNOWN47: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN47; break;
    case AE_CH_UNKNOWN48: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN48; break;
    case AE_CH_UNKNOWN49: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN49; break;
    case AE_CH_UNKNOWN50: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN50; break;
    case AE_CH_UNKNOWN51: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN51; break;
    case AE_CH_UNKNOWN52: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN52; break;
    case AE_CH_UNKNOWN53: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN53; break;
    case AE_CH_UNKNOWN54: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN54; break;
    case AE_CH_UNKNOWN55: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN55; break;
    case AE_CH_UNKNOWN56: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN56; break;
    case AE_CH_UNKNOWN57: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN57; break;
    case AE_CH_UNKNOWN58: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN58; break;
    case AE_CH_UNKNOWN59: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN59; break;
    case AE_CH_UNKNOWN60: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN60; break;
    case AE_CH_UNKNOWN61: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN61; break;
    case AE_CH_UNKNOWN62: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN62; break;
    case AE_CH_UNKNOWN63: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN63; break;
    case AE_CH_UNKNOWN64: AddonChannel = AUDIODSP_ADDON_CH_UNKNOWN64; break;

    default:              AddonChannel = AUDIODSP_ADDON_CH_INVALID;   return false;
  }

  return true;
}

bool CAudioDSPAddonUtil::AddonAudioFormat_TO_AEAudioFormat(const AUDIODSP_ADDON_AUDIO_FORMAT &AddonAudioFormat, AEAudioFormat &AEFormat)
{
  if (!AddonAudioDataFormat_TO_AEDataFormat(AddonAudioFormat.dataFormat, AEFormat.m_dataFormat))
  {
    return false;
  }

  if (!AddonChannelInfo_TO_AEChannelInfo(AddonAudioFormat.channelLayout, AEFormat.m_channelLayout))
  {
    return false;
  }

  AEFormat.m_sampleRate = AddonAudioFormat.sampleRate;
  AEFormat.m_frames = AddonAudioFormat.frames;
  AEFormat.m_frameSize = AddonAudioFormat.frameSize;

  return true;
}

bool CAudioDSPAddonUtil::AddonAudioDataFormat_TO_AEDataFormat(const AUDIODSP_ADDON_AUDIO_DATA_FORMAT &AddonDataFormat, AEDataFormat &DataFormat)
{
  switch (DataFormat)
  {
    case AUDIODSP_ADDON_FMT_U8:         DataFormat = AE_FMT_U8;          break;
    case AUDIODSP_ADDON_FMT_S16BE:      DataFormat = AE_FMT_S16BE;       break;
    case AUDIODSP_ADDON_FMT_S16LE:      DataFormat = AE_FMT_S16LE;       break;
    case AUDIODSP_ADDON_FMT_S16NE:      DataFormat = AE_FMT_S16NE;       break;
    case AUDIODSP_ADDON_FMT_S32BE:      DataFormat = AE_FMT_S32BE;       break;
    case AUDIODSP_ADDON_FMT_S32LE:      DataFormat = AE_FMT_S32LE;       break;
    case AUDIODSP_ADDON_FMT_S32NE:      DataFormat = AE_FMT_S32NE;       break;
    case AUDIODSP_ADDON_FMT_S24BE4:     DataFormat = AE_FMT_S24BE4;      break;
    case AUDIODSP_ADDON_FMT_S24LE4:     DataFormat = AE_FMT_S24LE4;      break;
    case AUDIODSP_ADDON_FMT_S24NE4:     DataFormat = AE_FMT_S24NE4;      break;
    case AUDIODSP_ADDON_FMT_S24NE4MSB:  DataFormat = AE_FMT_S24NE4MSB;   break;
    case AUDIODSP_ADDON_FMT_S24BE3:     DataFormat = AE_FMT_S24BE3;      break;
    case AUDIODSP_ADDON_FMT_S24LE3:     DataFormat = AE_FMT_S24LE3;      break;
    case AUDIODSP_ADDON_FMT_S24NE3:     DataFormat = AE_FMT_S24NE3;      break;
    case AUDIODSP_ADDON_FMT_DOUBLE:     DataFormat = AE_FMT_DOUBLE;      break;
    case AUDIODSP_ADDON_FMT_FLOAT:      DataFormat = AE_FMT_FLOAT;       break;
    case AUDIODSP_ADDON_FMT_U8P:        DataFormat = AE_FMT_U8P;         break;
    case AUDIODSP_ADDON_FMT_S16NEP:     DataFormat = AE_FMT_S16NEP;      break;
    case AUDIODSP_ADDON_FMT_S32NEP:     DataFormat = AE_FMT_S32NEP;      break;
    case AUDIODSP_ADDON_FMT_S24NE4P:    DataFormat = AE_FMT_S24NE4P;     break;
    case AUDIODSP_ADDON_FMT_S24NE4MSBP: DataFormat = AE_FMT_S24NE4MSBP;  break;
    case AUDIODSP_ADDON_FMT_S24NE3P:    DataFormat = AE_FMT_S24NE3P;     break;
    case AUDIODSP_ADDON_FMT_DOUBLEP:    DataFormat = AE_FMT_DOUBLEP;     break;
    case AUDIODSP_ADDON_FMT_FLOATP:     DataFormat = AE_FMT_FLOATP;      break;

    default:                            DataFormat = AE_FMT_INVALID;     return false;
  }

  return true;
}

bool CAudioDSPAddonUtil::AddonChannelInfo_TO_AEChannelInfo(const AUDIODSP_ADDON_CHANNEL_INFO &AddonChannelInfo, CAEChannelInfo &ChannelInfo)
{
  ChannelInfo.Reset();
  for (unsigned int ch = 0; ch < AddonChannelInfo.channelCount; ch++)
  {
    AEChannel channel;
    if (!AddonChannel_TO_AEChannel(AddonChannelInfo.channels[ch], channel))
    {
      return false;
    }

    ChannelInfo += channel;
  }

  return true;
}

bool CAudioDSPAddonUtil::AddonChannel_TO_AEChannel(const AUDIODSP_ADDON_CHANNEL &AddonChannel, AEChannel &Channel)
{
  switch (AddonChannel)
  {
    case AUDIODSP_ADDON_CH_FL:        Channel = AE_CH_FL;        break;
    case AUDIODSP_ADDON_CH_FR:        Channel = AE_CH_FR;        break;
    case AUDIODSP_ADDON_CH_FC:        Channel = AE_CH_FC;        break;
    case AUDIODSP_ADDON_CH_LFE:       Channel = AE_CH_LFE;       break;
    case AUDIODSP_ADDON_CH_BL:        Channel = AE_CH_BL;        break;
    case AUDIODSP_ADDON_CH_BR:        Channel = AE_CH_BR;        break;
    case AUDIODSP_ADDON_CH_FLOC:      Channel = AE_CH_FLOC;      break;
    case AUDIODSP_ADDON_CH_FROC:      Channel = AE_CH_FROC;      break;
    case AUDIODSP_ADDON_CH_BC:        Channel = AE_CH_BC;        break;
    case AUDIODSP_ADDON_CH_SL:        Channel = AE_CH_SL;        break;
    case AUDIODSP_ADDON_CH_SR:        Channel = AE_CH_SR;        break;
    case AUDIODSP_ADDON_CH_TFL:       Channel = AE_CH_TFL;       break;
    case AUDIODSP_ADDON_CH_TFR:       Channel = AE_CH_TFR;       break;
    case AUDIODSP_ADDON_CH_TFC:       Channel = AE_CH_TFC;       break;
    case AUDIODSP_ADDON_CH_TC:        Channel = AE_CH_TC;        break;
    case AUDIODSP_ADDON_CH_TBL:       Channel = AE_CH_TBL;       break;
    case AUDIODSP_ADDON_CH_TBR:       Channel = AE_CH_TBR;       break;
    case AUDIODSP_ADDON_CH_TBC:       Channel = AE_CH_TBC;       break;
    case AUDIODSP_ADDON_CH_BLOC:      Channel = AE_CH_BLOC;      break;
    case AUDIODSP_ADDON_CH_BROC:      Channel = AE_CH_BROC;      break;
    case AUDIODSP_ADDON_CH_UNKNOWN1:  Channel = AE_CH_UNKNOWN1;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN2:  Channel = AE_CH_UNKNOWN2;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN3:  Channel = AE_CH_UNKNOWN3;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN4:  Channel = AE_CH_UNKNOWN4;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN5:  Channel = AE_CH_UNKNOWN5;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN6:  Channel = AE_CH_UNKNOWN6;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN7:  Channel = AE_CH_UNKNOWN7;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN8:  Channel = AE_CH_UNKNOWN8;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN9:  Channel = AE_CH_UNKNOWN9;  break;
    case AUDIODSP_ADDON_CH_UNKNOWN10: Channel = AE_CH_UNKNOWN10; break;
    case AUDIODSP_ADDON_CH_UNKNOWN11: Channel = AE_CH_UNKNOWN11; break;
    case AUDIODSP_ADDON_CH_UNKNOWN12: Channel = AE_CH_UNKNOWN12; break;
    case AUDIODSP_ADDON_CH_UNKNOWN13: Channel = AE_CH_UNKNOWN13; break;
    case AUDIODSP_ADDON_CH_UNKNOWN14: Channel = AE_CH_UNKNOWN14; break;
    case AUDIODSP_ADDON_CH_UNKNOWN15: Channel = AE_CH_UNKNOWN15; break;
    case AUDIODSP_ADDON_CH_UNKNOWN16: Channel = AE_CH_UNKNOWN16; break;
    case AUDIODSP_ADDON_CH_UNKNOWN17: Channel = AE_CH_UNKNOWN17; break;
    case AUDIODSP_ADDON_CH_UNKNOWN18: Channel = AE_CH_UNKNOWN18; break;
    case AUDIODSP_ADDON_CH_UNKNOWN19: Channel = AE_CH_UNKNOWN19; break;
    case AUDIODSP_ADDON_CH_UNKNOWN20: Channel = AE_CH_UNKNOWN20; break;
    case AUDIODSP_ADDON_CH_UNKNOWN21: Channel = AE_CH_UNKNOWN21; break;
    case AUDIODSP_ADDON_CH_UNKNOWN22: Channel = AE_CH_UNKNOWN22; break;
    case AUDIODSP_ADDON_CH_UNKNOWN23: Channel = AE_CH_UNKNOWN23; break;
    case AUDIODSP_ADDON_CH_UNKNOWN24: Channel = AE_CH_UNKNOWN24; break;
    case AUDIODSP_ADDON_CH_UNKNOWN25: Channel = AE_CH_UNKNOWN25; break;
    case AUDIODSP_ADDON_CH_UNKNOWN26: Channel = AE_CH_UNKNOWN26; break;
    case AUDIODSP_ADDON_CH_UNKNOWN27: Channel = AE_CH_UNKNOWN27; break;
    case AUDIODSP_ADDON_CH_UNKNOWN28: Channel = AE_CH_UNKNOWN28; break;
    case AUDIODSP_ADDON_CH_UNKNOWN29: Channel = AE_CH_UNKNOWN29; break;
    case AUDIODSP_ADDON_CH_UNKNOWN30: Channel = AE_CH_UNKNOWN30; break;
    case AUDIODSP_ADDON_CH_UNKNOWN31: Channel = AE_CH_UNKNOWN31; break;
    case AUDIODSP_ADDON_CH_UNKNOWN32: Channel = AE_CH_UNKNOWN32; break;
    case AUDIODSP_ADDON_CH_UNKNOWN33: Channel = AE_CH_UNKNOWN33; break;
    case AUDIODSP_ADDON_CH_UNKNOWN34: Channel = AE_CH_UNKNOWN34; break;
    case AUDIODSP_ADDON_CH_UNKNOWN35: Channel = AE_CH_UNKNOWN35; break;
    case AUDIODSP_ADDON_CH_UNKNOWN36: Channel = AE_CH_UNKNOWN36; break;
    case AUDIODSP_ADDON_CH_UNKNOWN37: Channel = AE_CH_UNKNOWN37; break;
    case AUDIODSP_ADDON_CH_UNKNOWN38: Channel = AE_CH_UNKNOWN38; break;
    case AUDIODSP_ADDON_CH_UNKNOWN39: Channel = AE_CH_UNKNOWN39; break;
    case AUDIODSP_ADDON_CH_UNKNOWN40: Channel = AE_CH_UNKNOWN40; break;
    case AUDIODSP_ADDON_CH_UNKNOWN41: Channel = AE_CH_UNKNOWN41; break;
    case AUDIODSP_ADDON_CH_UNKNOWN42: Channel = AE_CH_UNKNOWN42; break;
    case AUDIODSP_ADDON_CH_UNKNOWN43: Channel = AE_CH_UNKNOWN43; break;
    case AUDIODSP_ADDON_CH_UNKNOWN44: Channel = AE_CH_UNKNOWN44; break;
    case AUDIODSP_ADDON_CH_UNKNOWN45: Channel = AE_CH_UNKNOWN45; break;
    case AUDIODSP_ADDON_CH_UNKNOWN46: Channel = AE_CH_UNKNOWN46; break;
    case AUDIODSP_ADDON_CH_UNKNOWN47: Channel = AE_CH_UNKNOWN47; break;
    case AUDIODSP_ADDON_CH_UNKNOWN48: Channel = AE_CH_UNKNOWN48; break;
    case AUDIODSP_ADDON_CH_UNKNOWN49: Channel = AE_CH_UNKNOWN49; break;
    case AUDIODSP_ADDON_CH_UNKNOWN50: Channel = AE_CH_UNKNOWN50; break;
    case AUDIODSP_ADDON_CH_UNKNOWN51: Channel = AE_CH_UNKNOWN51; break;
    case AUDIODSP_ADDON_CH_UNKNOWN52: Channel = AE_CH_UNKNOWN52; break;
    case AUDIODSP_ADDON_CH_UNKNOWN53: Channel = AE_CH_UNKNOWN53; break;
    case AUDIODSP_ADDON_CH_UNKNOWN54: Channel = AE_CH_UNKNOWN54; break;
    case AUDIODSP_ADDON_CH_UNKNOWN55: Channel = AE_CH_UNKNOWN55; break;
    case AUDIODSP_ADDON_CH_UNKNOWN56: Channel = AE_CH_UNKNOWN56; break;
    case AUDIODSP_ADDON_CH_UNKNOWN57: Channel = AE_CH_UNKNOWN57; break;
    case AUDIODSP_ADDON_CH_UNKNOWN58: Channel = AE_CH_UNKNOWN58; break;
    case AUDIODSP_ADDON_CH_UNKNOWN59: Channel = AE_CH_UNKNOWN59; break;
    case AUDIODSP_ADDON_CH_UNKNOWN60: Channel = AE_CH_UNKNOWN60; break;
    case AUDIODSP_ADDON_CH_UNKNOWN61: Channel = AE_CH_UNKNOWN61; break;
    case AUDIODSP_ADDON_CH_UNKNOWN62: Channel = AE_CH_UNKNOWN62; break;
    case AUDIODSP_ADDON_CH_UNKNOWN63: Channel = AE_CH_UNKNOWN63; break;
    case AUDIODSP_ADDON_CH_UNKNOWN64: Channel = AE_CH_UNKNOWN64; break;

    default:                          Channel = AE_CH_NULL;      return false;
  }

  return true;
}

bool CAudioDSPAddonUtil::AEStreamProperties_TO_AddonStreamProperties(const AEStreamProperties &AEStreamProperties, AUDIODSP_ADDON_STREAM_PROPERTIES &AddonStreamProperties)
{
  AddonStreamProperties.uiStreamID = AEStreamProperties.streamID;

  //! @todo AudioDSP V2 add conversion methods
  AEStreamProperties.matrixEncoding;
  AEStreamProperties.sourceFormat;
  AEStreamProperties.streamServiceType;
  memset(&AddonStreamProperties, 0, sizeof(AUDIODSP_ADDON_STREAM_PROPERTIES));
  AddonStreamProperties.eStreamType;// = AEStreamProperties.streamType;
  AddonStreamProperties.eBaseType;                  /*!< @brief the input stream base type source eg, Dolby Digital */
  AddonStreamProperties.strStreamName;              /*!< @brief the audio stream name */
  AddonStreamProperties.strStreamCodecId;           /*!< @brief codec id string of the audio stream */
  AddonStreamProperties.strStreamLanguage;          /*!< @brief language id of the audio stream */
  AddonStreamProperties.uProfile;// = AEStreamProperties.profile;

  return true;
}
}
