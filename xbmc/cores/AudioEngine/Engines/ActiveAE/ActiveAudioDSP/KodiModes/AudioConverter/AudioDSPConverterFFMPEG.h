#pragma once
/*
 *      Copyright (C) 2010-2017 Team Kodi
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

#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPTypes.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioConverter/AudioConverterModel.h"
#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPBufferNode.h"
#include "cores/AudioEngine/Interfaces/AEResample.h"


namespace ActiveAE
{
class CAudioDSPConverter : public DSP::AUDIO::IADSPNode, public IAudioConverterNodeCallback
{
public:
  CAudioDSPConverter(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, uint64_t ID, CAudioConverterModel &Model);
  virtual ~CAudioDSPConverter();

protected:
  virtual DSPErrorCode_t Create() override;
  virtual DSPErrorCode_t Destroy() override;

  virtual bool Process() override;

private:
  IAEResample *m_resampler;
  double m_resampleRatio;

  bool m_remapLayoutUsed;
  CAEChannelInfo m_remapLayout;
  bool m_forceResampling;

  int64_t m_lastSamplePts;
  
  // model and callback implementations
  bool m_needsSettingsUpdate;
  bool UpdateSettings();
  virtual void AudioConverterCallback() override;
  CAudioConverterModel &m_model;
  std::vector<uint8_t*> m_planes;
};

}
