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

  typedef std::vector<AE_DSP_MENUHOOK>                    AE_DSP_MENUHOOKS;
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
        iModeType = AE_DSP_MODE_TYPE_UNDEFINED;
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

      CAudioDSPMode(const AE_DSP_MODES::AE_DSP_MODE *Mode)
      {
        *this = Mode;
      }

      unsigned int      uiUniqueDBModeId;                                 /*!< @brief (required) the inside add-on used identifier for the mode, set by KODI's audio DSP database */
      AE_DSP_MODE_TYPE  iModeType;                                        /*!< @brief (required) the processong mode type, see AE_DSP_MODE_TYPE */
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

      CAudioDSPMode& operator= (const AE_DSP_MODES::AE_DSP_MODE *Mode)
      {
        uiUniqueDBModeId       = Mode->uiUniqueDBModeId;
        iModeType              = Mode->iModeType;
        strModeName            = Mode->strModeName;
        iModeNumber            = Mode->iModeNumber;
        iModeSupportTypeFlags  = Mode->iModeSupportTypeFlags;
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
      AE_DSP_MENUHOOK_CAT category;               /*!< @brief (required) category of menu hook */
      unsigned int        iRelevantModeId;        /*!< @brief (required) except category AE_DSP_MENUHOOK_SETTING and AE_DSP_MENUHOOK_ALL must be the related mode id present here */
      bool                bNeedPlayback;          /*!< @brief (required) set to true if menu hook need playback and active processing */
    };

    typedef std::vector<CAudioDSPMode>   CAudioDSPModeVector_t;

  public:
    explicit CActiveAEDSPAddon(ADDON::BinaryAddonBasePtr addonInfo);
    ~CActiveAEDSPAddon(void);

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

    /*!
     * @return The false if this addon is currently not used.
     */
    virtual bool IsInUse() const;
    //@}

    /** @name Audio DSP processing methods */
    //@{
    /*!
     * @brief Query this add-on's capabilities.
     * @return pCapabilities The add-on's capabilities.
     */
    AE_DSP_ADDON_CAPABILITIES GetAddonCapabilities(void) const;

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
    bool HaveMenuHooks(AE_DSP_MENUHOOK_CAT cat) const;

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
    void CallMenuHook(const AE_DSP_MENUHOOK &hook, AE_DSP_MENUHOOK_DATA &hookData);

    /*!
     * Set up Audio DSP with selected audio settings (use the basic present audio stream data format)
     * Used to detect available addons for present stream, as example stereo surround upmix not needed on 5.1 audio stream
     * @param addonSettings The add-on's audio settings.
     * @param pProperties The properties of the currently playing stream.
     * @return AE_DSP_ERROR_NO_ERROR if the properties were fetched successfully and data can be performed. AE_DSP_ERROR_IGNORE_ME if format is not supported, but without fault.
     */
    AE_DSP_ERROR StreamCreate(const AE_DSP_SETTINGS *addonSettings, const AE_DSP_STREAM_PROPERTIES* pProperties, ADDON_HANDLE handle);

    /*!
     * Remove the selected id from currently used dsp processes
     * @param id The stream id
     */
    void StreamDestroy(const ADDON_HANDLE handle);

    /*!
     * @brief Ask the addon about a requested processing mode that it is supported on the current
     * stream. Is called about every addon mode after successed StreamCreate.
     * @param id The stream id
     * @param addon_mode_id The mode inside addon which must be performed on call. Id is set from addon by iModeNumber on AE_DSP_MODE structure during RegisterMode callback,
     * @param unique_mode_id The Mode unique id generated from dsp database.
     * @return true if supported
     */
    bool StreamIsModeSupported(const ADDON_HANDLE handle, AE_DSP_MODE_TYPE type, unsigned int addon_mode_id, int unique_db_mode_id);

    /*!
     * Set up Audio DSP with selected audio settings (detected on data of first present audio packet)
     * @param addonSettings The add-on's audio settings.
     * @return AE_DSP_ERROR_NO_ERROR if the properties were fetched successfully.
     */
    AE_DSP_ERROR StreamInitialize(const ADDON_HANDLE handle, const AE_DSP_SETTINGS *addonSettings);

    /*!
     * @brief DSP input processing
     * Can be used to have unchanged stream..
     * All DSP add-ons allowed to-do this.
     * @param id The stream id
     * @param array_in Pointer to data memory
     * @param samples Amount of samples inside array_in
     * @return true if work was OK
     * @remarks Optional. Is set by AE_DSP_ADDON_CAPABILITIES and asked with GetAddonCapabilities
     */
    bool InputProcess(const ADDON_HANDLE handle, const float **array_in, unsigned int samples);

    /*!
     * If the add-on operate with buffered arrays and the output size can be higher as the input
     * it becomes asked about needed size before any InputResampleProcess call.
     * @param id The stream id
     * @return The needed size of output array or 0 if no changes within it
     */
    unsigned int InputResampleProcessNeededSamplesize(const ADDON_HANDLE handle);

    /*!
     * @brief DSP re sample processing before master Here a high quality resample can be performed.
     * Only one DSP add-on is allowed to-do this!
     * @param id The stream id
     * @param array_in Pointer to input data memory
     * @param array_out Pointer to output data memory
     * @param samples Amount of samples inside array_in
     * @return Amount of samples processed
     */
    unsigned int InputResampleProcess(const ADDON_HANDLE handle, const float **array_in, float **array_out, unsigned int samples);

    /*!
     * Returns the re-sampling generated new sample rate used before the master process
     * @param id The stream id
     * @return The new sample rate
     */
    int InputResampleSampleRate(const ADDON_HANDLE handle);

    /*!
     * Returns the time in seconds that it will take
     * for the next added packet to be heard from the speakers.
     * @param id The stream id
     * @return the delay in seconds
     */
    float InputResampleGetDelay(const ADDON_HANDLE handle);

    /*!
     * If the addon operate with buffered arrays and the output size can be higher as the input
     * it becomes asked about needed size before any PreProcess call.
     * @param id The stream id
     * @param mode_id The mode inside add-on which must be performed on call. Id is set from add-on by iModeNumber on AE_DSP_MODE structure during RegisterMode callback,
     * and can be defined from add-on as a structure pointer or anything else what is needed to find it.
     * @return The needed size of output array or 0 if no changes within it
     */
    unsigned int PreProcessNeededSamplesize(const ADDON_HANDLE handle, unsigned int mode_id);

    /*!
     * Returns the time in seconds that it will take
     * for the next added packet to be heard from the speakers.
     * @param id The stream id
     * @param mode_id The mode inside add-on which must be performed on call. Id is set from add-on by iModeNumber on AE_DSP_MODE structure during RegisterMode callback,
     * and can be defined from add-on as a structure pointer or anything else what is needed to find it.
     * @return the delay in seconds
     */
    float PreProcessGetDelay(const ADDON_HANDLE handle, unsigned int mode_id);

    /*!
     * @brief DSP preprocessing
     * All DSP add-ons allowed to-do this.
     * @param id The stream id
     * @param mode_id The mode inside add-on which must be performed on call. Id is set from add-on by iModeNumber on AE_DSP_MODE structure during RegisterMode callback,
     * and can be defined from add-on as a structure pointer or anything else what is needed to find it.
     * @param array_in Pointer to input data memory
     * @param array_out Pointer to output data memory
     * @param samples Amount of samples inside array_in
     * @return Amount of samples processed
     */
    unsigned int PreProcess(const ADDON_HANDLE handle, unsigned int mode_id, const float **array_in, float **array_out, unsigned int samples);

    /*!
     * @brief Set the active master process mode
     * @param id The stream id
     * @param type Requested stream type for the selected master mode
     * @param mode_id The Mode identifier.
     * @param unique_mode_id The Mode unique id generated from DSP database.
     * @return AE_DSP_ERROR_NO_ERROR if the setup was successful
     */
    AE_DSP_ERROR MasterProcessSetMode(const ADDON_HANDLE handle, AE_DSP_STREAMTYPE type, unsigned int mode_id, int unique_db_mode_id);

    /*!
     * @brief If the add-on operate with buffered arrays and the output size can be higher as the input
     * it becomes asked about needed size before any MasterProcess call.
     * @param id The stream id
     * @return The needed size of output array or 0 if no changes within it
     */
    unsigned int MasterProcessNeededSamplesize(const ADDON_HANDLE handle);

    /*!
     * @brief Returns the time in seconds that it will take
     * for the next added packet to be heard from the speakers.
     * @param id The stream id
     * @return the delay in seconds
     */
    float MasterProcessGetDelay(const ADDON_HANDLE handle);

    /*!
     * @brief Returns the from selected master mode performed channel alignment
     * @param id The stream id
     * @retval out_channel_present_flags the exact channel present flags after performed up-/downmix
     * @return the amount channels
     */
    int MasterProcessGetOutChannels(const ADDON_HANDLE handle, unsigned long &out_channel_present_flags);

    /*!
     * @brief Master processing becomes performed with it
     * Here a channel up-mix/down-mix for stereo surround sound can be performed
     * Only one DSP add-on is allowed to-do this!
     * @param id The stream id
     * @param array_in Pointer to input data memory
     * @param array_out Pointer to output data memory
     * @param samples Amount of samples inside array_in
     * @return Amount of samples processed
     */
    unsigned int MasterProcess(const ADDON_HANDLE handle, const float **array_in, float **array_out, unsigned int samples);

    /*!
     * @brief Get a from addon generated information string
     * @param id The stream id
     * @return Info string
     */
    std::string MasterProcessGetStreamInfoString(const ADDON_HANDLE handle);

    /*!
     * If the add-on operate with buffered arrays and the output size can be higher as the input
     * it becomes asked about needed size before any PostProcess call.
     * @param id The stream id
     * @param mode_id The mode inside add-on which must be performed on call. Id is set from add-on by iModeNumber on AE_DSP_MODE structure during RegisterMode callback,
     * and can be defined from add-on as a structure pointer or anything else what is needed to find it.
     * @return The needed size of output array or 0 if no changes within it
     */
    unsigned int PostProcessNeededSamplesize(const ADDON_HANDLE handle, unsigned int mode_id);

    /*!
     * Returns the time in seconds that it will take
     * for the next added packet to be heard from the speakers.
     * @param id The stream id
     * @param mode_id The mode inside add-on which must be performed on call. Id is set from add-on by iModeNumber on AE_DSP_MODE structure during RegisterMode callback,
     * and can be defined from add-on as a structure pointer or anything else what is needed to find it.
     * @return the delay in seconds
     */
    float PostProcessGetDelay(const ADDON_HANDLE handle, unsigned int mode_id);

    /*!
     * @brief DSP post processing
     * @param id The stream id
     * @param mode_id The mode inside add-on which must be performed on call. Id is set from add-on by iModeNumber on AE_DSP_MODE structure during RegisterMode callback,
     * and can be defined from add-on as a structure pointer or anything else what is needed to find it.
     * @param array_in Pointer to input data memory
     * @param array_out Pointer to output data memory
     * @param samples Amount of samples inside array_in
     * @return Amount of samples processed
     */
    unsigned int PostProcess(const ADDON_HANDLE handle, unsigned int mode_id, const float **array_in, float **array_out, unsigned int samples);

    /*!
     * @brief If the add-on operate with buffered arrays and the output size can be higher as the input
     * it becomes asked about needed size before any OutputResampleProcess call.
     * @param id The stream id
     * @return The needed size of output array or 0 if no changes within it
     */
    unsigned int OutputResampleProcessNeededSamplesize(const ADDON_HANDLE handle);

    /*!
     * @brief Re-sampling after master processing becomes performed with it if needed, only
     * one add-on can perform it.
     * @param id The stream id
     * @param array_in Pointer to input data memory
     * @param array_out Pointer to output data memory
     * @param samples Amount of samples inside array_in
     * @return Amount of samples processed
     */
    unsigned int OutputResampleProcess(const ADDON_HANDLE handle, const float **array_in, float **array_out, unsigned int samples);

    /*!
     * Returns the re-sampling generated new sample rate used after the master process
     * @param id The stream id
     * @return The new sample rate
     */
    int OutputResampleSampleRate(const ADDON_HANDLE handle);

    /*!
     * Returns the time in seconds that it will take
     * for the next added packet to be heard from the speakers.
     * @param id The stream id
     * @return the delay in seconds
     */
    float OutputResampleGetDelay(const ADDON_HANDLE handle);

    bool SupportsInputInfoProcess(void) const;
    bool SupportsPreProcess(void) const;
    bool SupportsInputResample(void) const;
    bool SupportsMasterProcess(void) const;
    bool SupportsPostProcess(void) const;
    bool SupportsOutputResample(void) const;

    static const char *ToString(const AE_DSP_ERROR error);

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

    bool LogError(const AE_DSP_ERROR error, const char *strMethod) const;

    /*!
     * @brief Callback functions from addon to kodi
     */
    //@{
    /*!
     * @brief Add or replace a menu hook for the menu for this add-on
     * @param kodiInstance A pointer to the add-on.
     * @param hook The hook to add.
     */
    static void cb_add_menu_hook(void* kodiInstance, AE_DSP_MENUHOOK* hook);

    /*!
    * @brief Remove a menu hook for the menu for this add-on
    * @param kodiInstance A pointer to the add-on.
    * @param hook The hook to remove.
    */
    static void cb_remove_menu_hook(void* kodiInstance, AE_DSP_MENUHOOK* hook);

    /*!
    * @brief Add or replace master mode information inside audio dsp database.
    * Becomes identifier written inside mode to iModeID if it was 0 (undefined)
    * @param kodiInstance A pointer to the add-on.
    * @param mode The master mode to add or update inside database
    */
    static void cb_register_mode(void* kodiInstance, AE_DSP_MODES::AE_DSP_MODE* mode);

    /*!
    * @brief Remove a master mode from audio dsp database
    * @param kodiInstance A pointer to the add-on.
    * @param mode The Mode to remove
    */
    static void cb_unregister_mode(void* kodiInstance, AE_DSP_MODES::AE_DSP_MODE* mode);
    //@}

    bool                      m_bReadyToUse;            /*!< true if this add-on is connected to the audio DSP, false otherwise */
    bool                      m_isInUse;                /*!< true if this add-on currently processing data */
    AE_DSP_MENUHOOKS          m_menuhooks;              /*!< the menu hooks for this add-on */
    unsigned int              m_uiClientId;             /*!< database ID of the audio DSP */

    /* cached data */
    std::string               m_strAudioDSPName;        /*!< the cached audio DSP name */
    std::string               m_strAudioDSPVersion;     /*!< the cached audio DSP version */
    std::string               m_strFriendlyName;        /*!< the cached friendly name */
    AE_DSP_ADDON_CAPABILITIES m_addonCapabilities;      /*!< the cached add-on capabilities */

    /* stored strings to make sure const char* members in AE_DSP_PROPERTIES stay valid */
    std::string               m_strUserPath;            /*!< @brief translated path to the user profile */
    std::string               m_strAddonPath ;          /*!< @brief translated path to this add-on */

    CCriticalSection          m_critSection;

    AddonInstance_AudioDSP m_struct; /*!< Interface table who contains function addresses and fixed values */
  };
}
