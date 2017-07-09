#pragma once

/*
 *      Copyright (C) 2005-2015 Team Kodi
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

#include "AddonBase.h"

/*!
 * @file Addon.h
 * @section sec1 Basic audio dsp addon interface description
 * @author Team Kodi
 * @date 10. May 2014
 * @version 0.1.5
 *
 * @subsection sec1_1 General
 * @li The basic support on the addon is supplied with the
 * AUDIODSP_ADDON_ADDON_CAPABILITIES data which becomes asked over
 * GetCapabilities(...), further the addon must register his available
 * modes on startup with the RegisterMode(...) callback function.
 * If one of this two points is not set the addon becomes
 * ignored for the chain step.
 *
 * @subsection sec1_2 Processing
 * @li On start of new stream the addon becomes called with StreamCreate(...)
 * to check about given values that it support it basically and can create
 * his structure, if nothing is supported it can return AUDIODSP_ADDON_ERROR_IGNORE_ME.
 *
 * @li As next step StreamIsModeSupported(...) becomes called for every
 * available and enabled modes, is separated due to more as one available mode
 * on the addon is possible, if the mode is not supported it can also be return
 * AUDIODSP_ADDON_ERROR_IGNORE_ME.
 *   - If mode is a resample mode and returns no error it becomes asked with
 *     InputResampleSampleRate(...) or OutputResampleSampleRate(...) (relevant
 *     to his type) about his given sample rate.
 *   - About the from user selected master processing mode the related addon
 *     becomes called now with MasterProcessSetMode(...) to handle it's
 *     selectionon the addon given by the own addon type identifier or by
 *     KODI's useddatabase id, also the currently used stream type (e.g.
 *     Music or Video) is send.
 *     - If the addon supports only one master mode it can ignore this function
 *       and return always AUDIODSP_ADDON_ERROR_NO_ERROR.
 *     - If the master mode is set the addon becomes asked about the from him
 *       given output channel layout related to up- or downmix modes, if
 *       nothing becomes changed on the layout it can return -1.
 *     - The MasterProcessSetMode(...) is also called if from user a another
 *       mode becomes selected.
 *
 * @li Then as last step shortly before the first process call becomes executed
 * the addon is called one time with StreamInitialize(...) to inform that
 * processing is started on the given settings.
 *   - This function becomes also called on all add-ons if the master process
 *     becomes changed.
 *   - Also every process after StreamInitialize on the addon mode becomes asked
 *     with _..._ProcessNeededSamplesize(...) about required memory size for the
 *     output of his data, if no other size is required it can return 0.
 *
 * @li From now the processing becomes handled for the different steps with
 * _..._Process(...).
 *   - Further it becomes asked with _..._GetDelay(...) about his processing
 *     time as float value in seconds, needed for video and audio alignment.
 *
 * @li On the end of the processing if the source becomes stopped the
 * StreamDestroy(...) function becomes called on all active processing add-ons.
 *
 * @note
 * The StreamCreate(...) can be becomes called for a new stream before the
 * previous was closed with StreamDestroy(...) ! To have a speed improve.
 */

namespace kodi { namespace addon { class CInstanceAudioDSP; }}

extern "C" {

  typedef void* AUDIODSP_HANDLE;

  /*!
   * @brief Audio DSP add-on error codes
   */
  typedef enum
  {
    AUDIODSP_ADDON_ERROR_FAILED               = -8,     /*!< @brief the command failed */
    AUDIODSP_ADDON_ERROR_INVALID_OUT_CHANNELS = -7,     /*!< @brief the processed output channel format is not supported */
    AUDIODSP_ADDON_ERROR_INVALID_IN_CHANNELS  = -6,     /*!< @brief the processed input channel format is not supported */
    AUDIODSP_ADDON_ERROR_INVALID_SAMPLERATE   = -5,     /*!< @brief the processed samplerate is not supported */
    AUDIODSP_ADDON_ERROR_INVALID_PARAMETERS   = -4,     /*!< @brief the parameters of the method that was called are invalid for this operation */
    AUDIODSP_ADDON_ERROR_REJECTED             = -3,     /*!< @brief the command was rejected by the DSP */
    AUDIODSP_ADDON_ERROR_NOT_IMPLEMENTED      = -2,     /*!< @brief the method that KODI called is not implemented by the add-on */
    AUDIODSP_ADDON_ERROR_UNKNOWN              = -1,     /*!< @brief an unknown error occurred */
    AUDIODSP_ADDON_ERROR_NO_ERROR             = 0,      /*!< @brief no error occurred */
    AUDIODSP_ADDON_ERROR_IGNORE_ME            = 1,      /*!< @brief the used input stream can not processed and add-on want to ignore */
  } AUDIODSP_ADDON_ERROR;

  /*!
   * @brief The possible DSP channels (used as pointer inside arrays)
   */
  typedef enum
  {
    AUDIODSP_ADDON_CH_INVALID = -1,
    AUDIODSP_ADDON_CH_FL,
    AUDIODSP_ADDON_CH_FR,
    AUDIODSP_ADDON_CH_FC,
    AUDIODSP_ADDON_CH_LFE,
    AUDIODSP_ADDON_CH_BL,
    AUDIODSP_ADDON_CH_BR,
    AUDIODSP_ADDON_CH_FLOC,
    AUDIODSP_ADDON_CH_FROC,
    AUDIODSP_ADDON_CH_BC,
    AUDIODSP_ADDON_CH_SL,
    AUDIODSP_ADDON_CH_SR,
    AUDIODSP_ADDON_CH_TFL,
    AUDIODSP_ADDON_CH_TFR,
    AUDIODSP_ADDON_CH_TFC,
    AUDIODSP_ADDON_CH_TC,
    AUDIODSP_ADDON_CH_TBL,
    AUDIODSP_ADDON_CH_TBR,
    AUDIODSP_ADDON_CH_TBC,
    AUDIODSP_ADDON_CH_BLOC,
    AUDIODSP_ADDON_CH_BROC,

    /* p16v devices */
    AUDIODSP_ADDON_CH_UNKNOWN1,
    AUDIODSP_ADDON_CH_UNKNOWN2,
    AUDIODSP_ADDON_CH_UNKNOWN3,
    AUDIODSP_ADDON_CH_UNKNOWN4,
    AUDIODSP_ADDON_CH_UNKNOWN5,
    AUDIODSP_ADDON_CH_UNKNOWN6,
    AUDIODSP_ADDON_CH_UNKNOWN7,
    AUDIODSP_ADDON_CH_UNKNOWN8,
    AUDIODSP_ADDON_CH_UNKNOWN9,
    AUDIODSP_ADDON_CH_UNKNOWN10,
    AUDIODSP_ADDON_CH_UNKNOWN11,
    AUDIODSP_ADDON_CH_UNKNOWN12,
    AUDIODSP_ADDON_CH_UNKNOWN13,
    AUDIODSP_ADDON_CH_UNKNOWN14,
    AUDIODSP_ADDON_CH_UNKNOWN15,
    AUDIODSP_ADDON_CH_UNKNOWN16,
    AUDIODSP_ADDON_CH_UNKNOWN17,
    AUDIODSP_ADDON_CH_UNKNOWN18,
    AUDIODSP_ADDON_CH_UNKNOWN19,
    AUDIODSP_ADDON_CH_UNKNOWN20,
    AUDIODSP_ADDON_CH_UNKNOWN21,
    AUDIODSP_ADDON_CH_UNKNOWN22,
    AUDIODSP_ADDON_CH_UNKNOWN23,
    AUDIODSP_ADDON_CH_UNKNOWN24,
    AUDIODSP_ADDON_CH_UNKNOWN25,
    AUDIODSP_ADDON_CH_UNKNOWN26,
    AUDIODSP_ADDON_CH_UNKNOWN27,
    AUDIODSP_ADDON_CH_UNKNOWN28,
    AUDIODSP_ADDON_CH_UNKNOWN29,
    AUDIODSP_ADDON_CH_UNKNOWN30,
    AUDIODSP_ADDON_CH_UNKNOWN31,
    AUDIODSP_ADDON_CH_UNKNOWN32,
    AUDIODSP_ADDON_CH_UNKNOWN33,
    AUDIODSP_ADDON_CH_UNKNOWN34,
    AUDIODSP_ADDON_CH_UNKNOWN35,
    AUDIODSP_ADDON_CH_UNKNOWN36,
    AUDIODSP_ADDON_CH_UNKNOWN37,
    AUDIODSP_ADDON_CH_UNKNOWN38,
    AUDIODSP_ADDON_CH_UNKNOWN39,
    AUDIODSP_ADDON_CH_UNKNOWN40,
    AUDIODSP_ADDON_CH_UNKNOWN41,
    AUDIODSP_ADDON_CH_UNKNOWN42,
    AUDIODSP_ADDON_CH_UNKNOWN43,
    AUDIODSP_ADDON_CH_UNKNOWN44,
    AUDIODSP_ADDON_CH_UNKNOWN45,
    AUDIODSP_ADDON_CH_UNKNOWN46,
    AUDIODSP_ADDON_CH_UNKNOWN47,
    AUDIODSP_ADDON_CH_UNKNOWN48,
    AUDIODSP_ADDON_CH_UNKNOWN49,
    AUDIODSP_ADDON_CH_UNKNOWN50,
    AUDIODSP_ADDON_CH_UNKNOWN51,
    AUDIODSP_ADDON_CH_UNKNOWN52,
    AUDIODSP_ADDON_CH_UNKNOWN53,
    AUDIODSP_ADDON_CH_UNKNOWN54,
    AUDIODSP_ADDON_CH_UNKNOWN55,
    AUDIODSP_ADDON_CH_UNKNOWN56,
    AUDIODSP_ADDON_CH_UNKNOWN57,
    AUDIODSP_ADDON_CH_UNKNOWN58,
    AUDIODSP_ADDON_CH_UNKNOWN59,
    AUDIODSP_ADDON_CH_UNKNOWN60,
    AUDIODSP_ADDON_CH_UNKNOWN61,
    AUDIODSP_ADDON_CH_UNKNOWN62,
    AUDIODSP_ADDON_CH_UNKNOWN63,
    AUDIODSP_ADDON_CH_UNKNOWN64,
    
    AUDIODSP_ADDON_CH_MAX
  } AUDIODSP_ADDON_CHANNEL;

  /**
   * @brief The various stream type formats
   * Used for audio DSP processing to know input audio type
   */
  typedef enum
  {
    AUDIODSP_ADDON_ASTREAM_INVALID = -1,
    AUDIODSP_ADDON_ASTREAM_MUSIC,
    AUDIODSP_ADDON_ASTREAM_MOVIE,
    AUDIODSP_ADDON_ASTREAM_GAME,
    AUDIODSP_ADDON_ASTREAM_APP,
    AUDIODSP_ADDON_ASTREAM_PHONE,
    AUDIODSP_ADDON_ASTREAM_MESSAGE,

    AUDIODSP_ADDON_ASTREAM_AUTO,
    AUDIODSP_ADDON_ASTREAM_MAX
  } AUDIODSP_ADDON_STREAMTYPE;

  /**
   * @brief The various base type formats
   * Used for audio DSP processing to know input audio source
   */
  typedef enum
  {
    AUDIODSP_ADDON_ABASE_UNKNOWN = -1,
    AUDIODSP_ADDON_ABASE_STEREO = 0,
    AUDIODSP_ADDON_ABASE_MONO,
    AUDIODSP_ADDON_ABASE_MULTICHANNEL,
    AUDIODSP_ADDON_ABASE_AC3,
    AUDIODSP_ADDON_ABASE_EAC3,
    AUDIODSP_ADDON_ABASE_DTS,
    AUDIODSP_ADDON_ABASE_DTSHD_MA,
    AUDIODSP_ADDON_ABASE_DTSHD_HRA,
    AUDIODSP_ADDON_ABASE_TRUEHD,
    AUDIODSP_ADDON_ABASE_MLP,
    AUDIODSP_ADDON_ABASE_FLAC,

    AUDIODSP_ADDON_ABASE_ADPCM,
    AUDIODSP_ADDON_ABASE_DPCM,
    AUDIODSP_ADDON_ABASE_PCM,
    AUDIODSP_ADDON_ABASE_MP2,
    AUDIODSP_ADDON_ABASE_MP3,
    AUDIODSP_ADDON_ABASE_AAC,
    AUDIODSP_ADDON_ABASE_VORBIS,
    AUDIODSP_ADDON_ABASE_DVAUDIO,
    AUDIODSP_ADDON_ABASE_WMAV1,
    AUDIODSP_ADDON_ABASE_WMAV2,
    AUDIODSP_ADDON_ABASE_MACE3,
    AUDIODSP_ADDON_ABASE_MACE6,
    AUDIODSP_ADDON_ABASE_VMDAUDIO,
    AUDIODSP_ADDON_ABASE_MP3ADU,
    AUDIODSP_ADDON_ABASE_MP3ON4,
    AUDIODSP_ADDON_ABASE_SHORTEN,
    AUDIODSP_ADDON_ABASE_ALAC,
    AUDIODSP_ADDON_ABASE_WESTWOOD_SND1,
    AUDIODSP_ADDON_ABASE_GSM, ///< as in Berlin toast format
    AUDIODSP_ADDON_ABASE_QDM2,
    AUDIODSP_ADDON_ABASE_COOK,
    AUDIODSP_ADDON_ABASE_TRUESPEECH,
    AUDIODSP_ADDON_ABASE_TTA,
    AUDIODSP_ADDON_ABASE_SMACKAUDIO,
    AUDIODSP_ADDON_ABASE_QCELP,
    AUDIODSP_ADDON_ABASE_WAVPACK,
    AUDIODSP_ADDON_ABASE_DSICINAUDIO,
    AUDIODSP_ADDON_ABASE_IMC,
    AUDIODSP_ADDON_ABASE_MUSEPACK7,
    AUDIODSP_ADDON_ABASE_GSM_MS, ///< as found in WAV
    AUDIODSP_ADDON_ABASE_ATRAC3,
    AUDIODSP_ADDON_ABASE_VOXWARE,
    AUDIODSP_ADDON_ABASE_APE,
    AUDIODSP_ADDON_ABASE_NELLYMOSER,
    AUDIODSP_ADDON_ABASE_MUSEPACK8,
    AUDIODSP_ADDON_ABASE_SPEEX,
    AUDIODSP_ADDON_ABASE_WMAVOICE,
    AUDIODSP_ADDON_ABASE_WMAPRO,
    AUDIODSP_ADDON_ABASE_WMALOSSLESS,
    AUDIODSP_ADDON_ABASE_ATRAC3P,
    AUDIODSP_ADDON_ABASE_SIPR,
    AUDIODSP_ADDON_ABASE_MP1,
    AUDIODSP_ADDON_ABASE_TWINVQ,
    AUDIODSP_ADDON_ABASE_MP4ALS,
    AUDIODSP_ADDON_ABASE_ATRAC1,
    AUDIODSP_ADDON_ABASE_BINKAUDIO_RDFT,
    AUDIODSP_ADDON_ABASE_BINKAUDIO_DCT,
    AUDIODSP_ADDON_ABASE_AAC_LATM,
    AUDIODSP_ADDON_ABASE_QDMC,
    AUDIODSP_ADDON_ABASE_CELT,
    AUDIODSP_ADDON_ABASE_G723_1,
    AUDIODSP_ADDON_ABASE_G729,
    AUDIODSP_ADDON_ABASE_8SVX_EXP,
    AUDIODSP_ADDON_ABASE_8SVX_FIB,
    AUDIODSP_ADDON_ABASE_BMV_AUDIO,
    AUDIODSP_ADDON_ABASE_RALF,
    AUDIODSP_ADDON_ABASE_IAC,
    AUDIODSP_ADDON_ABASE_ILBC,
    AUDIODSP_ADDON_ABASE_OPUS,
    AUDIODSP_ADDON_ABASE_COMFORT_NOISE,
    AUDIODSP_ADDON_ABASE_TAK,
    AUDIODSP_ADDON_ABASE_METASOUND,
    AUDIODSP_ADDON_ABASE_PAF_AUDIO,
    AUDIODSP_ADDON_ABASE_ON2AVC,
    AUDIODSP_ADDON_ABASE_DSS_SP,
    AUDIODSP_ADDON_ABASE_FFWAVESYNTH,
    AUDIODSP_ADDON_ABASE_SONIC,
    AUDIODSP_ADDON_ABASE_SONIC_LS,
    AUDIODSP_ADDON_ABASE_EVRC,
    AUDIODSP_ADDON_ABASE_SMV,
    AUDIODSP_ADDON_ABASE_DSD_LSBF,
    AUDIODSP_ADDON_ABASE_DSD_MSBF,
    AUDIODSP_ADDON_ABASE_DSD_LSBF_PLANAR,
    AUDIODSP_ADDON_ABASE_DSD_MSBF_PLANAR,
    AUDIODSP_ADDON_ABASE_4GV,
    AUDIODSP_ADDON_ABASE_INTERPLAY_ACM,
    AUDIODSP_ADDON_ABASE_XMA1,
    AUDIODSP_ADDON_ABASE_XMA2,
    AUDIODSP_ADDON_ABASE_DST,
    AUDIODSP_ADDON_ABASE_ATRAC3AL,
    AUDIODSP_ADDON_ABASE_ATRAC3PAL,

    AUDIODSP_ADDON_ABASE_MAX
  } AUDIODSP_ADDON_BASETYPE;


  /**
   * @brief The from KODI in settings requested audio process quality.
   * The KODI internal used quality levels is translated to this values
   * for usage on DSP processing add-ons. Is present on iQualityLevel
   * inside AUDIODSP_ADDON_SETTINGS.
   */
  typedef enum
  {
    AUDIODSP_ADDON_QUALITY_UNKNOWN    = -1,             /*!< @brief  Unset, unknown or incorrect quality level */
    AUDIODSP_ADDON_QUALITY_DEFAULT,                     /*!< @brief  Engine's default quality level */

    /* Basic quality levels */
    AUDIODSP_ADDON_QUALITY_LOW,                         /*!< @brief  Low quality level */
    AUDIODSP_ADDON_QUALITY_MID,                         /*!< @brief  Standard quality level */
    AUDIODSP_ADDON_QUALITY_HIGH,                        /*!< @brief  Best sound processing quality */

    /* Optional quality levels */
    AUDIODSP_ADDON_QUALITY_REALLYHIGH                  /*!< @brief  Uncompromising optional quality level, usually with unmeasurable and unnoticeable improvement */
  } AUDIODSP_ADDON_QUALITY;

  /*!
   * @brief Audio DSP menu hook categories.
   * Used to identify on AUDIODSP_ADDON_MENUHOOK given add-on related skin dialog/windows.
   * Except AUDIODSP_ADDON_MENUHOOK_ALL and AUDIODSP_ADDON_MENUHOOK_SETTING are the menus available
   * from DSP playback dialogue which can be opened over KODI file context menu and over
   * button on full screen OSD window.
   *
   * Menu hook AUDIODSP_ADDON_MENUHOOK_SETTING is available from DSP processing setup dialogue.
   */
  typedef enum
  {
    AUDIODSP_ADDON_MENUHOOK_UNKNOWN=-1,       /*!< @brief unknown menu hook */

    AUDIODSP_ADDON_MENUHOOK_MODE,             /*!< @brief for mode processing dialogs */
    AUDIODSP_ADDON_MENUHOOK_INFORMATION,      /*!< @brief dialogue to show generic informations */
    AUDIODSP_ADDON_MENUHOOK_ADDON_SETTINGS,   /*!< @brief for settings */

    AUDIODSP_ADDON_MENUHOOK_MAX,
  } AUDIODSP_MENU_HOOK_CAT;

  /*!
   * @brief Menu hooks that are available in the menus while playing a stream via this add-on.
   */
  typedef struct
  {
    unsigned int          iHookId;                /*!< @brief (required) this hook's identifier */
    unsigned int          iLocalizedStringId;     /*!< @brief (required) the id of the label for this hook in g_localizeStrings */
    AUDIODSP_MENU_HOOK_CAT category;               /*!< @brief (required) category of menu hook */
    unsigned int          iRelevantModeId;        /*!< @brief (required) except category AUDIODSP_ADDON_MENUHOOK_SETTING and AUDIODSP_ADDON_MENUHOOK_ALL must be the related mode id present here */
  } ATTRIBUTE_PACKED AUDIODSP_MENU_HOOK;

  /*!
   * @brief Audio processing settings for in and out arrays
   * Send on creation and before first processed audio packet to add-on
   */
  typedef struct
  {
    unsigned int                StreamID;       /*!< @brief id of the audio stream packets */
    AUDIODSP_ADDON_STREAMTYPE   StreamType;     /*!< @brief the input stream type source eg, Movie or Music */
    AUDIODSP_ADDON_QUALITY      QualityLevel;   /*!< @brief the from KODI selected quality level for signal processing */
    /*!
     * @note about "iProcessSamplerate" and "iProcessFrames" is set from KODI after call of StreamCreate on input re sample add-on, if re-sampling
     * and processing is handled inside the same add-on, this value must be ignored!
     */
  } ATTRIBUTE_PACKED AUDIODSP_ADDON_SETTINGS;

  /*!
   * @brief Stream profile properties
   * Can be used to detect best master processing mode and for post processing methods.
   */
//@{

  /*!
   * @brief Dolby profile types. Given from several formats, e.g. Dolby Digital or TrueHD
   * Used on AUDIODSP_ADDON_PROFILE_AC3_EAC3 and AUDIODSP_ADDON_PROFILE_MLP_TRUEHD
   */
  #define AUDIODSP_ADDON_PROFILE_DOLBY_NONE             0
  #define AUDIODSP_ADDON_PROFILE_DOLBY_SURROUND         1
  #define AUDIODSP_ADDON_PROFILE_DOLBY_PLII             2
  #define AUDIODSP_ADDON_PROFILE_DOLBY_PLIIX            3
  #define AUDIODSP_ADDON_PROFILE_DOLBY_PLIIZ            4
  #define AUDIODSP_ADDON_PROFILE_DOLBY_EX               5
  #define AUDIODSP_ADDON_PROFILE_DOLBY_HEADPHONE        6

  /*!
   * @brief DTS/DTS HD profile types
   * Used on AUDIODSP_ADDON_PROFILE_DTS_DTSHD
   */
  #define AUDIODSP_ADDON_PROFILE_DTS                    0
  #define AUDIODSP_ADDON_PROFILE_DTS_ES                 1
  #define AUDIODSP_ADDON_PROFILE_DTS_96_24              2
  #define AUDIODSP_ADDON_PROFILE_DTS_HD_HRA             3
  #define AUDIODSP_ADDON_PROFILE_DTS_HD_MA              4

  /*!
   * @brief AC3/EAC3 based service types
   * Used on AUDIODSP_ADDON_PROFILE_AC3_EAC3
   */
  #define AUDIODSP_ADDON_SERVICE_TYPE_MAIN              0
  #define AUDIODSP_ADDON_SERVICE_TYPE_EFFECTS           1
  #define AUDIODSP_ADDON_SERVICE_TYPE_VISUALLY_IMPAIRED 2
  #define AUDIODSP_ADDON_SERVICE_TYPE_HEARING_IMPAIRED  3
  #define AUDIODSP_ADDON_SERVICE_TYPE_DIALOGUE          4
  #define AUDIODSP_ADDON_SERVICE_TYPE_COMMENTARY        5
  #define AUDIODSP_ADDON_SERVICE_TYPE_EMERGENCY         6
  #define AUDIODSP_ADDON_SERVICE_TYPE_VOICE_OVER        7
  #define AUDIODSP_ADDON_SERVICE_TYPE_KARAOKE           8

  /*!
   * @brief AC3/EAC3 based room types
   * Present on AUDIODSP_ADDON_PROFILE_AC3_EAC3 and can be used for frequency corrections
   * at post processing, e.g. THX Re-Equalization
   */
  #define AUDIODSP_ADDON_ROOM_TYPE_UNDEFINED            0
  #define AUDIODSP_ADDON_ROOM_TYPE_SMALL                1
  #define AUDIODSP_ADDON_ROOM_TYPE_LARGE                2

  /*!
   * @brief AC3/EAC3 stream profile properties
   */
  //! @todo add handling for it (currently never becomes set)
  typedef struct AUDIODSP_ADDON_PROFILE_AC3_EAC3
  {
    int Profile;                      /*!< defined by AUDIODSP_ADDON_PROFILE_DOLBY_* */
    int ServiceType;                  /*!< defined by AUDIODSP_ADDON_SERVICE_TYPE_* */
    int RoomType;                     /*!< defined by AUDIODSP_ADDON_ROOM_TYPE_* (NOTICE: Information about it currently not supported from ffmpeg and must be implemented) */
  } ATTRIBUTE_PACKED AUDIODSP_ADDON_PROFILE_AC3_EAC3;

  /*!
   * @brief MLP/Dolby TrueHD stream profile properties
   */
  //! @todo add handling for it (currently never becomes set)
  typedef struct AUDIODSP_ADDON_PROFILE_MLP_TRUEHD
  {
    int Profile;                      /*!< defined by AUDIODSP_ADDON_PROFILE_DOLBY_* */
  } ATTRIBUTE_PACKED AUDIODSP_ADDON_PROFILE_MLP_TRUEHD;

  /*!
   * @brief DTS/DTS HD stream profile properties
   */
  //! @todo add handling for it (currently never becomes set)
  typedef struct AUDIODSP_ADDON_PROFILE_DTS_DTSHD
  {
    int Profile;                      /*!< defined by AUDIODSP_ADDON_PROFILE_DTS* */
    bool SurroundMatrix;              /*!< if set to true given 2.0 stream is surround encoded */
  } ATTRIBUTE_PACKED AUDIODSP_ADDON_PROFILE_DTS_DTSHD;

  union AUDIODSP_ADDON_PROFILE
  {
    AUDIODSP_ADDON_PROFILE_AC3_EAC3   ac3_eac3;         /*!< Dolby Digital/Digital+ profile data */
    AUDIODSP_ADDON_PROFILE_MLP_TRUEHD mlp_truehd;       /*!< MLP or Dolby TrueHD profile data */
    AUDIODSP_ADDON_PROFILE_DTS_DTSHD  dts_dtshd;        /*!< DTS/DTS-HD profile data */
  };
  //@}

  /*!
   * @brief Audio DSP stream properties
   * Used to check for the DSP add-on that the stream is supported,
   * as example Dolby Digital Ex processing is only required on Dolby Digital with 5.1 layout
   */
  typedef struct
  {
    unsigned int              uiStreamID;                 /*!< @brief stream id of the audio stream packets */
    AUDIODSP_ADDON_STREAMTYPE eStreamType;                /*!< @brief the input stream type source eg, Movie or Music */
    AUDIODSP_ADDON_BASETYPE   eBaseType;                  /*!< @brief the input stream base type source eg, Dolby Digital */
    const char*               strStreamName;              /*!< @brief the audio stream name */
    const char*               strStreamCodecId;           /*!< @brief codec id string of the audio stream */
    const char*               strStreamLanguage;          /*!< @brief language id of the audio stream */
    AUDIODSP_ADDON_PROFILE    uProfile;                   /*!< @brief current running stream profile data */
  } ATTRIBUTE_PACKED AUDIODSP_ADDON_STREAM_PROPERTIES;

  /*!
   * @brief Audio DSP master mode information
   * Used to get all available modes for current input stream
   */
  typedef struct
  {
    unsigned int      uiUniqueDBModeId;                                 /*!< @brief (required) the inside add-on used identifier for the mode, set by KODI's audio DSP database */
    char              strModeName[ADDON_STANDARD_STRING_LENGTH];        /*!< @brief (required) the addon name of the mode, used on KODI's logs  */

    unsigned int      uiModeNumber;                                     /*!< @brief (required) number of this mode on the add-on, is used on process functions with value "mode_id" */
    bool              bHasSettingsDialog;                               /*!< @brief (required) if setting dialog(s) are available it must be set to true */
    bool              bIsDisabled;                                      /*!< @brief (optional) true if this mode is marked as disabled and not enabled default, only relevant for master processes, all other types always disabled as default */

    unsigned int      iModeName;                                        /*!< @brief (required) the name id of the mode for this hook in g_localizeStrings */
    unsigned int      iModeSetupName;                                   /*!< @brief (optional) the name id of the mode inside settings for this hook in g_localizeStrings */
    unsigned int      iModeDescription;                                 /*!< @brief (optional) the description id of the mode for this hook in g_localizeStrings */
    unsigned int      iModeHelp;                                        /*!< @brief (optional) help string id for inside DSP settings dialog of the mode for this hook in g_localizeStrings */

    char              strOwnModeImage[ADDON_STANDARD_STRING_LENGTH];    /*!< @brief (optional) flag image for the mode */
    char              strOverrideModeImage[ADDON_STANDARD_STRING_LENGTH];/*!< @brief (optional) image to override KODI Image for the mode, eg. Dolby Digital with Dolby Digital Ex (only used on master modes) */
  } ATTRIBUTE_PACKED AUDIODSP_ADDON_MODE_DATA;

  /*!
   * @brief Properties passed to the Create() method of an add-on.
   */
  typedef struct AddonProps_AudioDSP
  {
    const char* strUserPath;                    /*!< @brief path to the user profile */
    const char* strAddonPath;                   /*!< @brief path to this add-on */
  } AddonProps_AudioDSP;

  typedef struct AddonToKodiFuncTable_AudioDSP
  {
    void* kodiInstance;
    void (*add_menu_hook)(void* kodiInstance, AUDIODSP_MENU_HOOK* hook);
    void (*remove_menu_hook)(void* kodiInstance, AUDIODSP_MENU_HOOK* hook);
    void (*register_mode)(void* kodiInstance, AUDIODSP_ADDON_MODE_DATA* mode);
    void (*unregister_mode)(void* kodiInstance, AUDIODSP_ADDON_MODE_DATA* mode);
  } AddonToKodiFuncTable_AudioDSP;

  struct AddonInstance_AudioDSP;
  typedef struct KodiToAddonFuncTable_AudioDSP
  {
    kodi::addon::CInstanceAudioDSP* addonInstance;
    const char* (__cdecl* get_dsp_name)(AddonInstance_AudioDSP const* addonInstance);
    const char* (__cdecl* get_dsp_version)(AddonInstance_AudioDSP const* addonInstance);
    AUDIODSP_ADDON_ERROR (__cdecl* menu_hook)(AddonInstance_AudioDSP const* addonInstance, const AUDIODSP_MENU_HOOK*);
    AUDIODSP_ADDON_ERROR(__cdecl* create_mode)(AddonInstance_AudioDSP const* instance, const AUDIODSP_ADDON_SETTINGS *addonSettings, const AUDIODSP_ADDON_STREAM_PROPERTIES* properties, ADDON_HANDLE handle);
    AUDIODSP_ADDON_ERROR(__cdecl* destroy_mode)(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE handle);
    int (__cdecl* get_mode_input_format)(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode);
    int (__cdecl* get_mode_out_format)(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode);
    unsigned int(__cdecl* needed_mode_frame_size)(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode);
    float (__cdecl* get_mode_delay)(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode);
    unsigned int(__cdecl* process_mode)(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode, const float** array_in, float** array_out, unsigned int samples);
  } KodiToAddonFuncTable_AudioDSP;

  typedef struct AddonInstance_AudioDSP
  {
    AddonProps_AudioDSP props;
    AddonToKodiFuncTable_AudioDSP toKodi;
    KodiToAddonFuncTable_AudioDSP toAddon;
  } AddonInstance_AudioDSP;

} /* extern "C" */

namespace kodi {
namespace addon {

  class CInstanceAudioDSP : public IAddonInstance
  {
  public:
    //==========================================================================
    /// @brief Class constructor
    ///
    CInstanceAudioDSP()
      : IAddonInstance(ADDON_INSTANCE_ADSP)
    {
      if (CAddonBase::m_interface->globalSingleInstance != nullptr)
        throw std::logic_error("kodi::addon::CInstanceAudioDSP: Creation of more as one in single instance way is not allowed!");

      SetAddonStruct(CAddonBase::m_interface->firstKodiInstance);
      CAddonBase::m_interface->globalSingleInstance = this;
    }
    //--------------------------------------------------------------------------

    //==========================================================================
    /// @brief Class constructor
    ///
    /// @param[in] instance             The from Kodi given instance given be
    ///                                 add-on CreateInstance call with instance
    ///                                 id ADDON_INSTANCE_ADSP.
    ///
    CInstanceAudioDSP(KODI_HANDLE instance)
      : IAddonInstance(ADDON_INSTANCE_ADSP)
    {
      if (CAddonBase::m_interface->globalSingleInstance != nullptr)
        throw std::logic_error("kodi::addon::CInstanceAudioDSP: Creation of multiple together with single instance way is not allowed!");

      SetAddonStruct(instance);
    }
    //--------------------------------------------------------------------------

    /*! @name Audio DSP add-on methods */
    //@{
    //==========================================================================
    ///
    /// @return The name reported by the back end that will be displayed in the
    /// UI.
    /// @remarks Valid implementation required.
    ///
    virtual std::string GetDSPName() = 0;
    //--------------------------------------------------------------------------

    //==========================================================================
    ///
    /// @return The version string reported by the back end that will be displayed
    /// in the UI.
    /// @remarks Valid implementation required.
    ///
    virtual std::string GetDSPVersion() = 0;
    //--------------------------------------------------------------------------

    //==========================================================================
    ///
    /// @brief Call one of the menu hooks (if supported).
    /// Supported AUDIODSP_ADDON_MENUHOOK instances have to be added in ADDON_Create(),
    /// by calling AddMenuHook() on the callback.
    /// @param menuHook The hook to call.
    /// @param item The selected item for which the hook was called.
    /// @return AUDIODSP_ADDON_ERROR_NO_ERROR if the hook was called successfully.
    /// @remarks Optional. Return AUDIODSP_ADDON_ERROR_NOT_IMPLEMENTED if this add-on
    /// won't provide this function.
    ///
    virtual AUDIODSP_ADDON_ERROR MenuHook(const AUDIODSP_MENU_HOOK& menuHook) { return AUDIODSP_ADDON_ERROR_NOT_IMPLEMENTED; }
    //--------------------------------------------------------------------------
    //@}

    virtual AUDIODSP_ADDON_ERROR CreateMode(const AUDIODSP_ADDON_SETTINGS& addonSettings, const AUDIODSP_ADDON_STREAM_PROPERTIES& properties, ADDON_HANDLE handle) = 0;

    virtual AUDIODSP_ADDON_ERROR DestroyMode(const ADDON_HANDLE handle) = 0;

    virtual AUDIODSP_ADDON_ERROR GetModeInputFormat(const ADDON_HANDLE mode) = 0;

    virtual AUDIODSP_ADDON_ERROR GetModeOutFormat(const ADDON_HANDLE mode) = 0;

    virtual unsigned int NeededModeFrameSize(const ADDON_HANDLE mode) { return 0; }

    virtual float GetModeDelay(const ADDON_HANDLE mode) { return 0.0f; }

    virtual unsigned int ProcessMode(const ADDON_HANDLE mode, const float** array_in, float** array_out, unsigned int samples) = 0;

    //==========================================================================
    ///
    /// @brief Add or replace a menu hook for the context menu for this add-on
    /// @param hook The hook to add
    ///
    void AddMenuHook(AUDIODSP_MENU_HOOK* hook)
    {
      return m_instanceData->toKodi.add_menu_hook(m_instanceData->toKodi.kodiInstance, hook);
    }
    //--------------------------------------------------------------------------

    //==========================================================================
    ///
    /// @brief Remove a menu hook for the context menu for this add-on
    /// @param hook The hook to remove
    ///
    void RemoveMenuHook(AUDIODSP_MENU_HOOK* hook)
    {
      return m_instanceData->toKodi.remove_menu_hook(m_instanceData->toKodi.kodiInstance, hook);
    }
    //--------------------------------------------------------------------------

    //==========================================================================
    ///
    /// @brief Add or replace master mode information inside audio dsp database.
    /// Becomes identifier written inside mode to iModeID if it was 0 (undefined)
    /// @param mode The master mode to add or update inside database
    ///
    void RegisterMode(AUDIODSP_ADDON_MODE_DATA *mode)
    {
      return m_instanceData->toKodi.register_mode(m_instanceData->toKodi.kodiInstance, mode);
    }
    //--------------------------------------------------------------------------

    //==========================================================================
    ///
    /// @brief Remove a master mode from audio dsp database
    /// @param mode The Mode to remove
    ///
    void UnregisterMode(AUDIODSP_ADDON_MODE_DATA *mode)
    {
      return m_instanceData->toKodi.unregister_mode(m_instanceData->toKodi.kodiInstance, mode);
    }
    //--------------------------------------------------------------------------

  private:
    void SetAddonStruct(KODI_HANDLE instance)
    {
      if (instance == nullptr)
        throw std::logic_error("kodi::addon::CInstanceAudioDSP: Null pointer instance passed.");

      m_instanceData = reinterpret_cast<AddonInstance_AudioDSP*>(instance);

      m_instanceData->toAddon.addonInstance = this;

      m_instanceData->toAddon.get_dsp_name = ADDON_GetDSPName;
      m_instanceData->toAddon.get_dsp_version = ADDON_GetDSPVersion;
      m_instanceData->toAddon.menu_hook = ADDON_MenuHook;
      m_instanceData->toAddon.create_mode = ADDON_CreateMode;
      m_instanceData->toAddon.destroy_mode = ADDON_DestroyMode;
      m_instanceData->toAddon.get_mode_input_format = ADDON_GetModeInputFormat;
      m_instanceData->toAddon.get_mode_out_format = ADDON_GetModeOutFormat;
      m_instanceData->toAddon.needed_mode_frame_size = ADDON_NeededModeFrameSize;
      m_instanceData->toAddon.get_mode_delay = ADDON_GetModeDelay;
      m_instanceData->toAddon.process_mode = ADDON_ProcessMode;
    }

    static inline const char* ADDON_GetDSPName(AddonInstance_AudioDSP const* instance)
    {
      instance->toAddon.addonInstance->m_dspName = instance->toAddon.addonInstance->GetDSPName();
      return instance->toAddon.addonInstance->m_dspName.c_str();
    }

    static inline const char* ADDON_GetDSPVersion(AddonInstance_AudioDSP const* instance)
    {
      instance->toAddon.addonInstance->m_dspVersion = instance->toAddon.addonInstance->GetDSPVersion();
      return instance->toAddon.addonInstance->m_dspVersion.c_str();
    }

    static inline AUDIODSP_ADDON_ERROR ADDON_MenuHook(AddonInstance_AudioDSP const* instance, const AUDIODSP_MENU_HOOK* menuHook)
    {
      return instance->toAddon.addonInstance->MenuHook(*menuHook);
    }

    static inline AUDIODSP_ADDON_ERROR ADDON_CreateMode(AddonInstance_AudioDSP const* instance, const AUDIODSP_ADDON_SETTINGS *addonSettings, const AUDIODSP_ADDON_STREAM_PROPERTIES* properties, ADDON_HANDLE handle)
    {
      return instance->toAddon.addonInstance->CreateMode(*addonSettings, *properties, handle);
    }

    static inline AUDIODSP_ADDON_ERROR ADDON_DestroyMode(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE handle)
    {
      return instance->toAddon.addonInstance->DestroyMode(handle);
    }

    static inline int ADDON_GetModeInputFormat(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode)
    {
      return instance->toAddon.addonInstance->GetModeInputFormat(mode);
    }

    static inline int ADDON_GetModeOutFormat(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode)
    {
      return instance->toAddon.addonInstance->GetModeOutFormat(mode);
    }

    static inline unsigned int ADDON_NeededModeFrameSize(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode)
    {
      return instance->toAddon.addonInstance->NeededModeFrameSize(mode);
    }

    static inline float ADDON_GetModeDelay(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode)
    {
      return instance->toAddon.addonInstance->GetModeDelay(mode);
    }

    static inline unsigned int ADDON_ProcessMode(AddonInstance_AudioDSP const* instance, const ADDON_HANDLE mode, const float** array_in, float** array_out, unsigned int samples)
    {
      return instance->toAddon.addonInstance->ProcessMode(mode, array_in, array_out, samples);
    }

    std::string m_dspName;
    std::string m_dspVersion;
    std::string m_streamInfoString;
    AddonInstance_AudioDSP* m_instanceData;
  };

} /* namespace addon */
} /* namespace kodi */
