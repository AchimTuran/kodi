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

#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioDSPKodiModes.h"

// mode includes
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/CopyMode/AudioDSPCopyMode.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioConverter/AudioConverterCreator.h"

#include "utils/log.h"

using namespace DSP;
using namespace std;

namespace ActiveAE
{
CAudioDSPKodiModes::CAudioDSPKodiModes()
{
}

void CAudioDSPKodiModes::PrepareModes(DSP::CDSPNodeModel &Model)
{
  CAudioDSPCopyModeCreator copyModeCreator;
  DSPErrorCode_t err = Model.RegisterNode(IDSPNodeModel::CDSPNodeInfoQuery({ "Kodi", "AudioDSPCopyMode" }), copyModeCreator);
  if (err != DSP_ERR_NO_ERR)
  {
    CLog::Log(LOGERROR, "%s failed to register Kodi::AudioDSPCopyMode!", __FUNCTION__);
  }
  IDSPNodeModel::CDSPNodeInfo copyModeNodeInfo = Model.GetNodeInfo(IDSPNodeModel::CDSPNodeInfoQuery({ "Kodi", "AudioDSPCopyMode" }));
  //Model.EnableNode(copyModeNodeInfo.ID); //! @todo AudioDSP V2 fix this node, as it currently crashes Kodi

  CAudioDSPAudioConverterCreator audioConverterCreator(m_audioConverterModel);
  err = Model.RegisterNode(IDSPNodeModel::CDSPNodeInfoQuery({ "Kodi", "AudioConverter" }), audioConverterCreator);
  if (err != DSP_ERR_NO_ERR)
  {
    CLog::Log(LOGERROR, "%s failed to register Kodi::AudioConverter!", __FUNCTION__);
  }  
}

void CAudioDSPKodiModes::ReleaseAllModes(DSP::CDSPNodeModel &Model)
{
  // release all internal Kodi AudioDSP modes
  DSPErrorCode_t err = Model.DeregisterNode(Model.GetNodeInfo(IDSPNodeModel::CDSPNodeInfoQuery({ "Kodi", "AudioDSPCopyMode" })).ID);
  if (err != DSP_ERR_NO_ERR)
  {
    CLog::Log(LOGERROR, "%s failed to deregister Kodi::AudioDSPCopyMode", __FUNCTION__);
  }

  err = Model.DeregisterNode(Model.GetNodeInfo(IDSPNodeModel::CDSPNodeInfoQuery({ "Kodi", "AudioConverter" })).ID);
  if (err != DSP_ERR_NO_ERR)
  {
    CLog::Log(LOGERROR, "%s failed to deregister Kodi::AudioConverter", __FUNCTION__);
  }
}
}
