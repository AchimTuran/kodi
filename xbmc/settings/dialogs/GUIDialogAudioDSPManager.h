#pragma once
/*
 *      Copyright (C) 2012-2015 Team KODI
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
 *  along with KODI; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "addons/kodi-addon-dev-kit/include/kodi/addon-instance/AudioDSP.h"
#include "cores/AudioEngine/Engines/ActiveAE/AudioDSPAddons/ActiveAEDSPMode.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "guilib/GUIDialog.h"
#include "view/GUIViewControl.h"
#include "cores/AudioEngine/Engines/ActiveAE/Interfaces/AudioDSPNodeModelCallback.h"

class CGUIDialogBusy;

class CGUIDialogAudioDSPManager : public CGUIDialog, public DSP::IDSPNodeModelCallback
{
public:
  CGUIDialogAudioDSPManager(void);
  virtual ~CGUIDialogAudioDSPManager(void);
  bool OnMessage(CGUIMessage& message) override;
  bool OnAction(const CAction& action) override;
  void OnWindowLoaded(void) override;
  void OnWindowUnload(void) override;
  bool HasListItems() const override { return true; };
  virtual void FrameMove();

protected:
  void OnInitWindow() override;
  void OnDeinitWindow(int nextWindowID) override;

  virtual bool OnPopupMenu(int iItem, int listType);
  virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button, int listType);

  virtual bool OnActionMove(const CAction &action);

  virtual bool OnMessageClick(CGUIMessage &message);

  bool OnClickListAvailable(CGUIMessage &message);
  bool OnClickListActive(CGUIMessage &message);
  bool OnClickRadioContinousSaving(CGUIMessage &message);
  bool OnClickApplyChanges(CGUIMessage &message);
  bool OnClickClearActiveModes(CGUIMessage &message);

  void SetItemsUnchanged(void);

private:
  // AudioDSP model callbacks
  virtual DSPErrorCode_t EnableNodeCallback(uint64_t ID, uint32_t Position = 0) override;
  virtual DSPErrorCode_t DisableNodeCallback(uint64_t ID) override;

  void Clear(void);
  void Update(void);
  void SaveList(void);
  void Renumber(void);
  bool UpdateDatabase(CGUIDialogBusy* pDlgBusy);
  void SetSelectedModeType(void);

  //! helper function prototypes
  static void                 helper_LogError(const char *function);
  static int                  helper_TranslateModeType(std::string ModeString);
  static CFileItem           *helper_CreateModeListItem(ActiveAE::CActiveAEDSPModePtr &ModePointer, AE_DSP_MENUHOOK_CAT &MenuHook, int *ContinuesNo);
  static int                  helper_GetDialogId(ActiveAE::CActiveAEDSPModePtr &ModePointer, AE_DSP_MENUHOOK_CAT &MenuHook, ActiveAE::AE_DSP_ADDON &Addon, std::string AddonName);
  static AE_DSP_MENUHOOK_CAT  helper_GetMenuHookCategory(int CurrentType);

  bool m_bMovingMode;
  bool m_bContainsChanges;
  bool m_bContinousSaving;    // if true, all settings are directly saved

  int m_iCurrentType;
  int m_iSelected[AE_DSP_MODE_TYPE_MAX];

  CFileItemList* m_activeItems[AE_DSP_MODE_TYPE_MAX];
  CFileItemList* m_availableItems[AE_DSP_MODE_TYPE_MAX];

  CGUIViewControl m_availableViewControl;
  CGUIViewControl m_activeViewControl;

  void* m_AudioDSPController;
  CCriticalSection m_nodeCallbackLock;
};
