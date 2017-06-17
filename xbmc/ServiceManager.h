/*
 *      Copyright (C) 2005-2016 Team XBMC
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

#pragma once

#include <memory>
#include "platform/Platform.h"

namespace ADDON {
class CAddonMgr;
class CBinaryAddonCache;
}

class IAE;

namespace ANNOUNCEMENT
{
class CAnnouncementManager;
}

namespace PVR
{
class CPVRManager;
}

namespace PLAYLIST
{
  class CPlayListPlayer;
}

class CContextMenuManager;
class XBPython;
class CDataCacheCore;
class IAE;

namespace GAME
{
  class CGameServices;
}

class CServiceManager
{
public:
  CServiceManager();
  ~CServiceManager();

  bool Init1();
  bool Init2();
  bool CreateAudioEngine();
  bool DestroyAudioEngine();
  bool StartAudioEngine();
  bool Init3();
  void Deinit();
  ADDON::CAddonMgr& GetAddonMgr();
  ADDON::CBinaryAddonCache& GetBinaryAddonCache();
  ANNOUNCEMENT::CAnnouncementManager& GetAnnouncementManager();
  XBPython& GetXBPython();
  PVR::CPVRManager& GetPVRManager();
  IAE& GetActiveAE();
  CContextMenuManager& GetContextMenuManager();
  CDataCacheCore& GetDataCacheCore();
  /**\brief Get the platform object. This is save to be called after Init1() was called
   */
  CPlatform& GetPlatform();
  GAME::CGameServices& GetGameServices();

  PLAYLIST::CPlayListPlayer& GetPlaylistPlayer();

protected:
  struct delete_dataCacheCore
  {
    void operator()(CDataCacheCore *p) const;
  };

  struct delete_contextMenuManager
  {
    void operator()(CContextMenuManager *p) const;
  };

  struct delete_activeAE
  {
    void operator()(IAE *p) const;
  };

  std::unique_ptr<ADDON::CAddonMgr> m_addonMgr;
  std::unique_ptr<ADDON::CBinaryAddonCache> m_binaryAddonCache;
  std::unique_ptr<ANNOUNCEMENT::CAnnouncementManager> m_announcementManager;
  std::unique_ptr<XBPython> m_XBPython;
  std::unique_ptr<PVR::CPVRManager> m_PVRManager;
  //! @todo use IAE instead of CActiveAE
  std::unique_ptr<IAE, delete_activeAE> m_ActiveAE;
  std::unique_ptr<CContextMenuManager, delete_contextMenuManager> m_contextMenuManager;
  std::unique_ptr<CDataCacheCore, delete_dataCacheCore> m_dataCacheCore;
  std::unique_ptr<CPlatform> m_Platform;
  std::unique_ptr<PLAYLIST::CPlayListPlayer> m_playlistPlayer;
  std::unique_ptr<GAME::CGameServices> m_gameServices;
};
