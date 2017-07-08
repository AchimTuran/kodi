#pragma once
/*
 *      Copyright (C) 2010-2014 Team KODI
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

#include <memory>
#include <string>
#include <vector>

#include "addons/binary-addons/AddonInstanceHandler.h"
#include "addons/kodi-addon-dev-kit/include/kodi/addon-instance/AudioDSP.h"

namespace ActiveAE
{
  class CActiveAEDSPAddon;

  typedef std::vector<AUDIODSP_MENU_HOOK>       AE_DSP_MENUHOOKS;
  typedef std::shared_ptr<ActiveAE::CActiveAEDSPAddon>    AE_DSP_ADDON;

  #define AE_DSP_INVALID_ADDON_ID (0)

  /*!
   * Interface from KODI to a Audio DSP add-on.
   *
   * Also translates KODI's C++ structures to the addon's C structures.
   */
  class CActiveAEDSPAddon : public ADDON::IAddonInstanceHandler
  {
  public:
    class CAudioDSPMode
    {
    public:
      CAudioDSPMode() 
      {
        uiUniqueDBModeId = AE_DSP_INVALID_ADDON_ID;  //! @todo AudioDSP V2 replace this with a new define which is not specific to ADDON_ID
        strModeName.clear();

        iModeNumber = AE_DSP_INVALID_ADDON_ID;  //! @todo AudioDSP V2 replace this with a new define which is not specific to ADDON_ID
        iModeSupportTypeFlags = 0;
        bHasSettingsDialog = false;
        bIsDisabled = true;

        iModeName = 0;
        iModeSetupName = 0;
        iModeDescription = 0;
        iModeHelp = 0;

        strOwnModeImage.clear();
        strOverrideModeImage.clear();
      }

      CAudioDSPMode(const AUDIODSP_ADDON_MODE_DATA *Mode)
      {
        *this = Mode;
      }

      unsigned int      uiUniqueDBModeId;                                 /*!< @brief (required) the inside add-on used identifier for the mode, set by KODI's audio DSP database */
      std::string       strModeName;                                      /*!< @brief (required) the addon name of the mode, used on KODI's logs  */

      unsigned int      iModeNumber;                                      /*!< @brief (required) number of this mode on the add-on, is used on process functions with value "mode_id" */
      unsigned int      iModeSupportTypeFlags;                            /*!< @brief (required) flags about supported input types for this mode, see AE_DSP_ASTREAM_PRESENT */
      bool              bHasSettingsDialog;                               /*!< @brief (required) if setting dialog(s) are available it must be set to true */
      bool              bIsDisabled;                                      /*!< @brief (optional) true if this mode is marked as disabled and not enabled default, only relevant for master processes, all other types always disabled as default */

      unsigned int      iModeName;                                        /*!< @brief (required) the name id of the mode for this hook in g_localizeStrings */
      unsigned int      iModeSetupName;                                   /*!< @brief (optional) the name id of the mode inside settings for this hook in g_localizeStrings */
      unsigned int      iModeDescription;                                 /*!< @brief (optional) the description id of the mode for this hook in g_localizeStrings */
      unsigned int      iModeHelp;                                        /*!< @brief (optional) help string id for inside DSP settings dialog of the mode for this hook in g_localizeStrings */

      std::string       strOwnModeImage;                                  /*!< @brief (optional) flag image for the mode */
      std::string       strOverrideModeImage;                             /*!< @brief (optional) image to override KODI Image for the mode, eg. Dolby Digital with Dolby Digital Ex (only used on master modes) */

      CAudioDSPMode& operator= (const AUDIODSP_ADDON_MODE_DATA *Mode)
      {
        uiUniqueDBModeId       = Mode->uiUniqueDBModeId;
        strModeName            = Mode->strModeName;
        bHasSettingsDialog     = Mode->bHasSettingsDialog;
        bIsDisabled            = Mode->bIsDisabled;
        iModeName              = Mode->iModeName;
        iModeSetupName         = Mode->iModeSetupName;
        iModeDescription       = Mode->iModeDescription;
        iModeHelp              = Mode->iModeHelp;
        strOwnModeImage        = Mode->strOwnModeImage;
        strOverrideModeImage   = Mode->strOverrideModeImage;

        return *this;
      }
    };

    class CAudioDSPDialogHook
    {
    public:
      unsigned int        iHookId;                /*!< @brief (required) this hook's identifier */
      unsigned int        iLocalizedStringId;     /*!< @brief (required) the id of the label for this hook in g_localizeStrings */
      AUDIODSP_MENU_HOOK_CAT category;               /*!< @brief (required) category of menu hook */
      unsigned int        iRelevantModeId;        /*!< @brief (required) except category AE_DSP_MENUHOOK_SETTING and AE_DSP_MENUHOOK_ALL must be the related mode id present here */
    };

    typedef std::vector<CAudioDSPMode>   CAudioDSPModeVector_t;

  public:
    explicit CActiveAEDSPAddon(ADDON::BinaryAddonBasePtr addonInfo);
    ~CActiveAEDSPAddon(void) override;

    /** @name Audio DSP add-on methods */
    //@{
    /*!
     * @brief Initialise the instance of this add-on.
     * @param iClientId The ID of this add-on.
     */
    bool Create(unsigned int iClientId);

    /*!
     * @brief Destroy the instance of this add-on.
     */
    void Destroy(void);

    /*!
     * @brief Destroy and recreate this add-on.
     */
    bool ReCreate(void);

    /*!
     * @return True if this instance is initialised, false otherwise.
     */
    bool ReadyToUse(void) const;

    /*!
     * @return The ID of this instance.
     */
    unsigned int GetAudioDSPID(void) const;
    //@}

    /** @name Audio DSP processing methods */
    //@{
    /*!
     * @return The name reported by the backend.
     */
    const std::string &GetAudioDSPName(void) const;

    /*!
     * @return The version string reported by the backend.
     */
    const std::string &GetAudioDSPVersion(void) const;

    /*!
     * @return A friendly name for this add-on that can be used in log messages.
     */
    const std::string &GetFriendlyName(void) const;

    /*!
     * @return True if this add-on has menu hooks, false otherwise.
     */
    bool HaveMenuHooks(AUDIODSP_MENU_HOOK_CAT cat) const;

    /*!
     * @return The menu hooks for this add-on.
     */
    AE_DSP_MENUHOOKS *GetMenuHooks(void);

    /*!
     * @brief Gets the registered AudioDSP modes.
     * @param AudioDSP mode vector.
     */
    void GetAudioDSPModes(CAudioDSPModeVector_t &Modes);

    /*!
     * @brief Call one of the menu hooks of this addon.
     * @param hook The hook to call.
     * @param item The selected file item for which the hook was called.
     */
    void CallMenuHook(const AUDIODSP_MENU_HOOK &hook);

    /*!
     * Set up Audio DSP with selected audio settings (detected on data of first present audio packet)
     * @param addonSettings The add-on's audio settings.
     * @return AUDIODSP_ADDON_ERROR_NO_ERROR if the properties were fetched successfully.
     */
    AUDIODSP_ADDON_ERROR StreamInitialize(const ADDON_HANDLE handle, const AUDIODSP_ADDON_SETTINGS *addonSettings);

    static const char *ErrorCodeToString(const AUDIODSP_ADDON_ERROR error);

  private:
    static unsigned int m_uniqueModeID;
    CAudioDSPModeVector_t m_registeredModes;
    unsigned int RegisterMode(const CAudioDSPMode &Mode);
    void DeregisterMode(const CAudioDSPMode &Mode);

    /*!
     * @brief Resets all class members to their defaults. Called by the constructors.
     */
    void ResetProperties(unsigned int uiClientId);

    void GetAddonProperties(void);

    bool LogError(const AUDIODSP_ADDON_ERROR error, const char *strMethod) const;

    /*!
     * @brief Callback functions from addon to kodi
     */
    //@{
    /*!
     * @brief Add or replace a menu hook for the menu for this add-on
     * @param kodiInstance A pointer to the add-on.
     * @param hook The hook to add.
     */
    static void cb_add_menu_hook(void* kodiInstance, AUDIODSP_MENU_HOOK* hook);

    /*!
    * @brief Remove a menu hook for the menu for this add-on
    * @param kodiInstance A pointer to the add-on.
    * @param hook The hook to remove.
    */
    static void cb_remove_menu_hook(void* kodiInstance, AUDIODSP_MENU_HOOK* hook);

    /*!
    * @brief Add or replace master mode information inside audio dsp database.
    * Becomes identifier written inside mode to iModeID if it was 0 (undefined)
    * @param kodiInstance A pointer to the add-on.
    * @param mode The master mode to add or update inside database
    */
    static void cb_register_mode(void* kodiInstance, AUDIODSP_ADDON_MODE_DATA* mode);

    /*!
    * @brief Remove a master mode from audio dsp database
    * @param kodiInstance A pointer to the add-on.
    * @param mode The Mode to remove
    */
    static void cb_unregister_mode(void* kodiInstance, AUDIODSP_ADDON_MODE_DATA* mode);
    //@}

    bool                      m_bReadyToUse;            /*!< true if this add-on is connected to the audio DSP, false otherwise */
    AE_DSP_MENUHOOKS          m_menuhooks;              /*!< the menu hooks for this add-on */
    unsigned int              m_uiClientId;             /*!< database ID of the audio DSP */

    /* cached data */
    std::string               m_strAudioDSPName;        /*!< the cached audio DSP name */
    std::string               m_strAudioDSPVersion;     /*!< the cached audio DSP version */
    std::string               m_strFriendlyName;        /*!< the cached friendly name */

    /* stored strings to make sure const char* members in AE_DSP_PROPERTIES stay valid */
    std::string               m_strUserPath;            /*!< @brief translated path to the user profile */
    std::string               m_strAddonPath ;          /*!< @brief translated path to this add-on */

    CCriticalSection          m_critSection;

    AddonInstance_AudioDSP m_struct; /*!< Interface table who contains function addresses and fixed values */
  };
}
