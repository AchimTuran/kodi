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


#include "cores/AudioEngine/Engines/ActiveAE/ActiveAEBuffer.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/Interfaces/IADSPNode.h"
#include <deque>
#include <memory>
 
namespace ActiveAE
{
class CAudioDSPBufferPool : public IAEBufferPoolCallback
{
public:
  CAudioDSPBufferPool(const AEAudioFormat &InputFormat, const AEAudioFormat &OutputFormat);
  virtual ~CAudioDSPBufferPool();

  virtual bool Create(unsigned int totaltime);
  virtual bool AddInputBuffer(CSampleBuffer *Buffer);
  
  // IAEBufferPoolCallback interface implementation
  virtual void ReturnBuffer(CSampleBuffer *Buffer) override;

  virtual CSampleBuffer* GetBuffer();

  float GetDelay();
  
protected:
  virtual CSampleBuffer *GetFreeBuffer();

  bool m_needsConversion;

  AEAudioFormat m_inputFormat;
  std::deque<CSampleBuffer*> m_inputSamples;
  CSampleBuffer *m_inputBuffer;
  
  AEAudioFormat m_outputFormat;
  CSampleBuffer *m_processingBuffer;

  std::deque<CSampleBuffer*> m_allSamples;
  std::deque<CSampleBuffer*> m_freeSamples;

private:
  DSP::AUDIO::IADSPNode *m_audioConverter;
};
}
