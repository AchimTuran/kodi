/*
 *      Copyright (C) 2010-2013 Team XBMC
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

#include "system.h"

#include <stdint.h>
#include <limits.h>

#include "AESourceNULL.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "utils/log.h"

CAESourceNULL::CAESourceNULL()
  : CThread("AESourceNull"),
    m_draining(false),
    m_source_frameSize(0),
    m_sourcebuffer_size(0),
    m_sourcebuffer_level(0),
    m_sourcebuffer_sec_per_byte(0)
{
}

CAESourceNULL::~CAESourceNULL()
{
}

bool CAESourceNULL::Initialize(AEAudioFormat &format, std::string &device)
{
  // setup for a 250ms source feed from SoftAE 
  format.m_dataFormat    = AE_IS_RAW(format.m_dataFormat) ? AE_FMT_S16NE : AE_FMT_FLOAT;
  format.m_frames        = format.m_sampleRate / 1000 * 250;
  format.m_frameSamples  = format.m_channelLayout.Count();
  format.m_frameSize     = format.m_frameSamples * (CAEUtil::DataFormatToBits(format.m_dataFormat) >> 3);
  m_format = format;

  // setup a pretend 500ms internal buffer
  m_source_frameSize = format.m_channelLayout.Count() * CAEUtil::DataFormatToBits(format.m_dataFormat) >> 3;
  m_sourcebuffer_size = m_source_frameSize * format.m_sampleRate / 2;
  m_sourcebuffer_sec_per_byte = 1.0 / (double)(m_source_frameSize * format.m_sampleRate);

  m_draining = false;
  m_wake.Reset();
  m_inited.Reset();
  Create();
  if (!m_inited.WaitMSec(100))
  {
    while(!m_inited.WaitMSec(1))
      Sleep(10);
  }

  return true;
}

void CAESourceNULL::Deinitialize()
{
  // force m_bStop and set m_wake, if might be sleeping.
  m_bStop = true;
  StopThread();
}

void CAESourceNULL::GetDelay(AEDelayStatus& status)
{
  double sourcebuffer_seconds_to_empty = m_sourcebuffer_sec_per_byte * (double)m_sourcebuffer_level;
  status.SetDelay(sourcebuffer_seconds_to_empty);
}

double CAESourceNULL::GetCacheTotal()
{
  return m_sourcebuffer_sec_per_byte * (double)m_sourcebuffer_size;
}

unsigned int CAESourceNULL::GetPackets(uint8_t **data, unsigned int frames, unsigned int offset)
{
  unsigned int max_frames = (m_sourcebuffer_size - m_sourcebuffer_level) / m_source_frameSize;
  if (frames > max_frames)
    frames = max_frames;

  if (frames)
  {
    m_sourcebuffer_level += frames * m_source_frameSize;
    m_wake.Set();
  }

  return frames;
}

void CAESourceNULL::Drain()
{
  m_draining = true;
  m_wake.Set();
}

void CAESourceNULL::EnumerateDevices (AEDeviceList &devices)
{
  // we never return any devices
}

void CAESourceNULL::Process()
{
  CLog::Log(LOGDEBUG, "CAESourceNULL::Process");

  // The object has been created and waiting to play,
  m_inited.Set();
  // yield to give other threads a chance to do some work.
  Sleep(0);

  SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
  while (!m_bStop)
  {
    if (m_draining)
    {
      // TODO: is it correct to not take data at the appropriate rate while draining?
      m_sourcebuffer_level = 0;
      m_draining = false;
    }

    // pretend we have a 64k audio buffer
    unsigned int min_buffer_size = 64 * 1024;
    unsigned int read_bytes = m_sourcebuffer_level;
    if (read_bytes > min_buffer_size)
      read_bytes = min_buffer_size;

    if (read_bytes > 0)
    {
      // drain it
      m_sourcebuffer_level -= read_bytes;

      // we MUST drain at the correct audio sample rate
      // or the NULL source will not work right. So calc
      // an approximate sleep time.
      int frames_written = read_bytes / m_source_frameSize;
      double empty_ms = 1000.0 * (double)frames_written / m_format.m_sampleRate;
      #if defined(TARGET_POSIX)
        usleep(empty_ms * 1000.0);
      #else
        Sleep((int)empty_ms);
      #endif
    }

    if (m_sourcebuffer_level == 0)
    {
      // sleep this audio thread, we will get woken when we have audio data.
      m_wake.WaitMSec(250);
    }
  }
  SetPriority(THREAD_PRIORITY_NORMAL);
}
