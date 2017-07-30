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


#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/AudioDSPNodeModel.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioConverter/AudioConverterModel.h"

 // mode includes
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/CopyMode/AudioDSPCopyMode.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioConverter/AudioConverterCreator.h"

#include <vector>

namespace ActiveAE
{
class CAudioDSPKodiModes
{
public:
  CAudioDSPKodiModes();

  void PrepareModes(DSP::CDSPNodeModel &Model);
  void ReleaseAllModes(DSP::CDSPNodeModel &Model);

  CAudioConverterModel m_audioConverterModel;

private:
  CAudioDSPCopyModeCreator m_copyModeCreator;
  CAudioDSPAudioConverterCreator m_audioConverterCreator;
};
}
