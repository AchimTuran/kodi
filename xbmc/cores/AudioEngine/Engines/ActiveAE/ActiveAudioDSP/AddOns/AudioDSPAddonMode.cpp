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



#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AddOns/AudioDSPAddonMode.h"

using namespace DSP;
using namespace DSP::AUDIO;


namespace ActiveAE
{
  CAudioDSPAddonMode::CAudioDSPAddonMode( const AEAudioFormat &InputFormat,
                                          const AEAudioFormat &OutputFormat,
                                          const AEStreamProperties &StreamProperties,
                                          uint64_t ID,
                                          const AddonInstance_AudioDSP &Struct,
                                          const ADDON_HANDLE_STRUCT &ModeHandle) :
  IADSPBufferNode("CAudioDSPAddonMode", ID)
{
  m_InputFormat = InputFormat;
  m_OutputFormat = OutputFormat;
  m_streamProperties = StreamProperties;

  memset(&m_struct, 0, sizeof(AddonToKodiFuncTable_AudioDSP));
  m_struct = Struct;

  memset(&m_modeHandle, 0, sizeof(ADDON_HANDLE_STRUCT));
  m_modeHandle = ModeHandle;
}

DSPErrorCode_t CAudioDSPAddonMode::CreateInstance(AEAudioFormat &InputFormat, AEAudioFormat &OutputFormat)
{
  InputFormat.m_dataFormat = AE_FMT_FLOATP;
  OutputFormat.m_dataFormat = AE_FMT_FLOATP;
  
  AUDIODSP_ADDON_ERROR adspErr;
  
  adspErr = m_struct.toAddon.create_mode(&m_struct, &m_modeHandle);
  if (adspErr != AUDIODSP_ADDON_ERROR_NO_ERROR)
  {
    return DSP_ERR_FATAL_ERROR; //! @todo AudioDSP V2 translate add-on errors to core DSP errors
  }

  AUDIODSP_ADDON_AUDIO_FORMAT modeInputFormat = m_struct.toAddon.get_mode_input_format(&m_struct, &m_modeHandle);
  AUDIODSP_ADDON_AUDIO_FORMAT modeOutputFormat = m_struct.toAddon.get_mode_output_format(&m_struct, &m_modeHandle);
  //! @todo AudioDSP V2 use conversion functions for input- and output-format

  return DSP_ERR_NO_ERR;
}

DSPErrorCode_t CAudioDSPAddonMode::DestroyInstance()
{
  m_InputFormat.m_channelLayout.Reset();
  m_OutputFormat.m_channelLayout.Reset();

  AUDIODSP_ADDON_ERROR adspErr;
  adspErr = m_struct.toAddon.destroy_mode(&m_struct, &m_modeHandle);
  if (adspErr != AUDIODSP_ADDON_ERROR_NO_ERROR)
  {
    return DSP_ERR_FATAL_ERROR; //! @todo AudioDSP V2 translate add-on errors to core DSP errors
  }

  return DSP_ERR_NO_ERR;
}

//! @todo AudioDSP V2 why int and not unsigned int?
int CAudioDSPAddonMode::ProcessInstance(const uint8_t **In, uint8_t **Out)
{
  unsigned int processedSamples = 0;

  processedSamples = m_struct.toAddon.process_mode(&m_struct, &m_modeHandle, In, Out);
  return processedSamples;
}
}
