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


#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioConverter/AudioConverterCreator.h"
#include "cores/AudioEngine/Engines/ActiveAE/ActiveAudioDSP/KodiModes/AudioConverter/AudioDSPConverterFFMPEG.h"

using namespace DSP;
using namespace DSP::AUDIO;
using namespace ActiveAE;

CAudioDSPAudioConverterCreator::CAudioDSPAudioConverterCreator(CAudioConverterModel &Model) :
  m_model(Model)
{
}

IADSPNode *CAudioDSPAudioConverterCreator::InstantiateNode(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat, const AEStreamProperties &StreamProperties, uint64_t ID)
{
  //! @todo add Raspberry PI resampler implementation
  CAudioDSPConverter *converter = new CAudioDSPConverter(InputFormat, OutputFormat, ID, m_model);
  m_model.Register(dynamic_cast<IAudioConverterNodeCallback*>(converter));
  IADSPNode *node = dynamic_cast<IADSPNode*>(converter);
  if (!node)
  {
    m_model.Deregister(converter);
    delete converter;
  }

  return node;
}

DSPErrorCode_t CAudioDSPAudioConverterCreator::DestroyNode(IADSPNode *&Node)
{
  DSPErrorCode_t err = DSP_ERR_INVALID_INPUT;
  if (Node)
  {
    m_model.Deregister(dynamic_cast<IAudioConverterNodeCallback*>(Node));
    err = Node->Destroy();

    delete Node;
    Node = nullptr;
  }

  return err;
}

IDSPNodeCreator* CAudioDSPAudioConverterCreator::CreateCreator()
{
  return dynamic_cast<IDSPNodeCreator*>(new CAudioDSPAudioConverterCreator(m_model));
}
