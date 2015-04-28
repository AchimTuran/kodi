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

#include "AESourceFactory.h"
#include "Interfaces/AESource.h"
#if defined(TARGET_WINDOWS)
  #include "Sources/AESourceWASAPI.h"
  #include "Sources/AESourcekDirectSound.h"
//#elif defined(TARGET_ANDROID)
//  #include "Sources/AESourceAUDIOTRACK.h"
#elif defined(TARGET_RASPBERRY_PI)
  #include "Sources/AESourcePi.h"
  #include "Sources/AESourceALSA.h"
//#elif defined(TARGET_DARWIN_IOS)
//  #include "Sources/AESourceDARWINIOS.h"
//#elif defined(TARGET_DARWIN_OSX)
//  #include "Sources/AESourceDARWINOSX.h"
#elif defined(TARGET_LINUX) || defined(TARGET_FREEBSD)
  #if defined(HAS_ALSA)
    #include "Sources/AESourceALSA.h"
  #endif
  //#if defined(HAS_PULSEAUDIO)
  //  #include "Sources/AESourcePULSE.h"
  //#endif
  //#include "Sources/AESourceOSS.h"
#else
  #pragma message("NOTICE: No audio source for target platform.  Audio input will not be available.")
#endif
#include "Sources/AESourceNULL.h"

#include "settings/AdvancedSettings.h"
#include "utils/SystemInfo.h"
#include "utils/log.h"

#include <algorithm>

void CAESourceFactory::ParseDevice(std::string &device, std::string &driver)
{
  int pos = device.find_first_of(':');
  if (pos > 0)
  {
    driver = device.substr(0, pos);
    std::transform(driver.begin(), driver.end(), driver.begin(), ::toupper);

    // check that it is a valid driver name
    if (
#if defined(TARGET_WINDOWS)
        driver == "WASAPI"      ||
        driver == "DIRECTSOUND" ||
#elif defined(TARGET_ANDROID)
        driver == "AUDIOTRACK"  ||
#elif defined(TARGET_RASPBERRY_PI)
        driver == "PI"          ||
        driver == "ALSA"        ||
#elif defined(TARGET_DARWIN_IOS)
        driver == "DARWINIOS"  ||
#elif defined(TARGET_DARWIN_OSX)
        driver == "DARWINOSX"  ||
#elif defined(TARGET_LINUX) || defined(TARGET_FREEBSD)
  #if defined(HAS_ALSA)
        driver == "ALSA"        ||
  #endif
  #if defined(HAS_PULSEAUDIO)
        driver == "PULSE"       ||
  #endif
        driver == "OSS"         ||
#endif
        driver == "PROFILER"    ||
        driver == "NULL")
      device = device.substr(pos + 1, device.length() - pos - 1);
    else
      driver.clear();
  }
  else
    driver.clear();
}

IAESource *CAESourceFactory::TrySource(std::string &driver, std::string &device, AEAudioFormat &format)
{
  IAESource *source = NULL;

  if (driver == "NULL")
    source = new CAESourceNULL();
  else
  {
#if defined(TARGET_WINDOWS)
    if (driver == "WASAPI")
      source = new CAESourceWASAPI();
    if (driver == "DIRECTSOUND")
      source = new CAESourceDirectSound();
#elif defined(TARGET_ANDROID)
    source = new CAESourceAUDIOTRACK();
#elif defined(TARGET_RASPBERRY_PI)
  if (driver == "PI")
    source = new CAESourcePi();
  #if defined(HAS_ALSA)
  if (driver == "ALSA")
    source = new CAESourceALSA();
  #endif
#elif defined(TARGET_DARWIN_IOS)
    source = new CAESourceDARWINIOS();
#elif defined(TARGET_DARWIN_OSX)
    source = new CAESourceDARWINOSX();
#elif defined(TARGET_LINUX) || defined(TARGET_FREEBSD)
 #if defined(HAS_PULSEAUDIO)
    if (driver == "PULSE")
      source = new CAESourcePULSE();
 #endif
 #if defined(HAS_ALSA)
    if (driver == "ALSA")
      source = new CAESourceALSA();
 #endif
    if (driver == "OSS")
      source = new CAESourceOSS();
#endif
  }

  if (!source)
    return NULL;

  if (source->Initialize(format, device))
  {
    // do some sanity checks
    if (format.m_sampleRate == 0)
      CLog::Log(LOGERROR, "Source %s:%s returned invalid sample rate", driver.c_str(), device.c_str());
    else if (format.m_channelLayout.Count() == 0)
      CLog::Log(LOGERROR, "Source %s:%s returned invalid channel layout", driver.c_str(), device.c_str());
    else if (format.m_frames < 256)
      CLog::Log(LOGERROR, "Source %s:%s returned invalid buffer size: %d", driver.c_str(), device.c_str(), format.m_frames);
    else
      return source;
  }
  source->Deinitialize();
  delete source;
  return NULL;
}

IAESource *CAESourceFactory::Create(std::string &device, AEAudioFormat &desiredFormat, bool rawPassthrough)
{
  // extract the driver from the device string if it exists
  std::string driver;
  ParseDevice(device, driver);

  AEAudioFormat  tmpFormat = desiredFormat;
  IAESource       *source;
  std::string    tmpDevice = device;

  source = TrySource(driver, tmpDevice, tmpFormat);
  if (source)
  {
    desiredFormat = tmpFormat;
    return source;
  }

  return NULL;
}

void CAESourceFactory::EnumerateEx(AESourceInfoList &list, bool force)
{
  AESourceInfo info;
#if defined(TARGET_WINDOWS)

  info.m_deviceInfoList.clear();
  info.m_sourceName = "DIRECTSOUND";
  CAESourceDirectSound::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
    list.push_back(info);

  info.m_deviceInfoList.clear();
  info.m_sourceName = "WASAPI";
  CAESourceWASAPI::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
    list.push_back(info);

#elif defined(TARGET_ANDROID)

  info.m_deviceInfoList.clear();
  info.m_sourceName = "AUDIOTRACK";
  CAESourceAUDIOTRACK::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
    list.push_back(info);

#elif defined(TARGET_RASPBERRY_PI)

  info.m_deviceInfoList.clear();
  info.m_sourceName = "PI";
  CAESourcePi::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
    list.push_back(info);
  #if defined(HAS_ALSA)
  info.m_deviceInfoList.clear();
  info.m_sourceName = "ALSA";
  CAESourceALSA::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
    list.push_back(info);
  #endif
#elif defined(TARGET_DARWIN_IOS)

  info.m_deviceInfoList.clear();
  info.m_sourceName = "DARWINIOS";
  CAESourceDARWINIOS::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
    list.push_back(info);

#elif defined(TARGET_DARWIN_OSX)

  info.m_deviceInfoList.clear();
  info.m_sourceName = "DARWINOSX";
  CAESourceDARWINOSX::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
    list.push_back(info);

#elif defined(TARGET_LINUX) || defined(TARGET_FREEBSD)
  // check if user wants us to do something specific
  if (getenv("AE_SOURCE"))
  {
    std::string envSource = (std::string)getenv("AE_SOURCE");
    std::transform(envSource.begin(), envSource.end(), envSource.begin(), ::toupper);
    info.m_deviceInfoList.clear();
    #if defined(HAS_PULSEAUDIO)
    if (envSource == "PULSE")
      CAESourcePULSE::EnumerateDevicesEx(info.m_deviceInfoList, force);
    #endif
    #if defined(HAS_ALSA)
    if (envSource == "ALSA")
      CAESourceALSA::EnumerateDevicesEx(info.m_deviceInfoList, force);
    #endif
    if (envSource == "OSS")
      CAESourceOSS::EnumerateDevicesEx(info.m_deviceInfoList, force);

    if(!info.m_deviceInfoList.empty())
    {
      info.m_sourceName = envSource;
      list.push_back(info);
      return;
    }
    else
      CLog::Log(LOGNOTICE, "User specified Source %s could not be enumerated", envSource.c_str());
  }

  #if defined(HAS_PULSEAUDIO)
  info.m_deviceInfoList.clear();
  info.m_sourceName = "PULSE";
  CAESourcePULSE::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
  {
    list.push_back(info);
    return;
  }
  #endif

  #if defined(HAS_ALSA)
  info.m_deviceInfoList.clear();
  info.m_sourceName = "ALSA";
  CAESourceALSA::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
  {
    list.push_back(info);
    return;
  }
  #endif

  info.m_deviceInfoList.clear();
  info.m_sourceName = "OSS";
  CAESourceOSS::EnumerateDevicesEx(info.m_deviceInfoList, force);
  if(!info.m_deviceInfoList.empty())
    list.push_back(info);

#endif

}
