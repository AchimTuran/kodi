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

#include <vector>

#include "ActiveAEDSP.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
}

namespace ActiveAE
{
  class CSampleBuffer;
  class IAEResample;
  class CActiveAEBufferPoolADSP;

  //@{
  /*!
   * Individual DSP Processing class
   */
  class CActiveAEDSPProcess
  {
    public:
      CActiveAEDSPProcess(unsigned int streamId);
      virtual ~CActiveAEDSPProcess();

      //@{
      /*!>
       * Create the dsp processing with check of all addons about the used input and output audio format.
       * @param inputFormat The used audio stream input format
       * @param outputFormat Audio output format which is needed to send to the sinks
       * @param upmix stereo upmix value from KODI settings is passed in with it
       * @param quality The requested quality from settings
       * @param streamType The input stream type to find allowed master process dsp addons for it, e.g. AE_DSP_ASTREAM_MUSIC
       * @return True if the dsp processing becomes available
       */
      bool Create(const AEAudioFormat &inputFormat, const AEAudioFormat &outputFormat, bool upmix, bool bypassDSP, AEQuality quality, AUDIODSP_ADDON_STREAMTYPE streamType,
                  enum AVMatrixEncoding matrix_encoding, enum AVAudioServiceType audio_service_type, int profile);

      /*!>
       * Destroy all allocated dsp addons for this stream id and stops the processing.
       */
      void Destroy();

      /*!>
       * Force processing function (Process(...)) to perform a reinitialization of addons and data
       */
      void ForceReinit();

      /*!>
       * Get the stream id for this processing class
       */
      unsigned int GetStreamId() const;

      /*!>
      * Get the currently used input stream format
      * @note used to have a fallback to normal operation without dsp
      */
      AEAudioFormat GetOutputFormat();

      /*!>
       * Get the currently used input stream format
       * @note used to have a fallback to normal operation without dsp
       */
      AEAudioFormat GetInputFormat();

      /*!>
       * Get the inside addons used samplerate for this class
       */
      unsigned int GetProcessSamplerate();

      /*!>
       * Get the amount of percent what the cpu need to process complete dsp stream
       * @return The current cpu usage
       */
      float GetCPUUsage(void) const;

      /*!>
       * It returns the on input source detected stream type, not always the active one.
       */
      AUDIODSP_ADDON_STREAMTYPE GetDetectedStreamType();

      /*!>
       * Get the currently on addons processed audio stream type which is set from KODI,
       * it is user selectable or if auto mode is enabled it becomes detected upon the
       * stream input source, eg. Movie, Music...
       */
      AUDIODSP_ADDON_STREAMTYPE GetUsedStreamType();

      /*!>
       * Get the currently on addons processed audio base type which is detected from KODI.
       * The base type is relevant to the type of input source, eg. Mono, Stereo, Dolby Digital...
       */
      AUDIODSP_ADDON_BASETYPE GetUsedBaseType();

      /*!>
       * Used check that asked addon with his mode id is used on given stream identifier
       * @param category The type to get
       * @param iAddonId The ID of the addon to get the menu entries for it
       * @param iModeNumber From addon defined identifier of the mode
       * @return true if in use
       */
      bool IsMenuHookModeActive(AUDIODSP_MENU_HOOK_CAT category, int iAddonId, unsigned int iModeNumber);

    protected:
      friend class CActiveAEBufferPoolADSP;

      /*!>
       * Master processing
       * @param in the ActiveAE input samples
       * @param out the processed ActiveAE output samples
       * @return true if processing becomes performed correct
       */
      bool Process(CSampleBuffer *in, CSampleBuffer *out);

      /*!>
       * Returns the time in seconds that it will take
       * for the next added packet to be heard from the speakers.
       * @return seconds
       */
      float GetDelay();

      /*!>
       * Update the state all AudioDSP modes.
       */
      void UpdateActiveModes();
    //@}
    private:
    //@{
      /*!
       * Helper functions
       */
      void InitFFMpegDSPProcessor();
      bool CreateStreamProfile();
      void ResetStreamFunctionsSelection();
      AUDIODSP_ADDON_STREAMTYPE DetectStreamType(const CFileItem *item);
      const char *GetStreamTypeName(AUDIODSP_ADDON_STREAMTYPE iStreamType);
      void ClearArray(float **array, unsigned int samples);
      bool MasterModeChange(int iModeID, AUDIODSP_ADDON_STREAMTYPE iStreamType = AUDIODSP_ADDON_ASTREAM_INVALID);
      AUDIODSP_ADDON_BASETYPE GetBaseType(AUDIODSP_ADDON_STREAM_PROPERTIES *props);
      bool RecheckProcessArray(unsigned int inputFrames);
      bool ReallocProcessArray(unsigned int requestSize);
      void CalculateCPUUsage(uint64_t iTime);
      void SetFFMpegDSPProcessorArray(float *array_ffmpeg[AUDIODSP_ADDON_CH_MAX], float *array_dsp[AUDIODSP_ADDON_CH_MAX], int idx[AE_CH_MAX], unsigned long ChannelFlags);
    //@}
    //@{
      /*!
       * Data
       */
      const unsigned int                m_streamId;                 /*!< stream id of this class, is a increase/decrease number of the amount of process streams */
      AUDIODSP_ADDON_STREAMTYPE         m_streamTypeDetected;       /*! The detected stream type of the stream from the source of it */
      AUDIODSP_ADDON_STREAMTYPE         m_streamTypeUsed;           /*!< The currently used stream type */
      bool                              m_forceInit;                /*!< if set to true the process function perform a reinitialization of addons and data */
      AE_DSP_ADDONMAP                   m_usedMap;                  /*!< a map of all currently used audio dsp add-on's */
      AEAudioFormat                     m_inputFormat;              /*!< the used input stream format */
      AEAudioFormat                     m_outputFormat;             /*!< the from Kodi requested output format */
      AEQuality                         m_streamQuality;            /*!< from KODI requested stream quality, based also to addons */
      bool                              m_bypassDSP;                /*!< if true, all AudioDSP modes are skipped */
      AUDIODSP_ADDON_SETTINGS           m_addonSettings;            /*!< the current stream's settings passed to dsp add-ons */
      AUDIODSP_ADDON_STREAM_PROPERTIES  m_addonStreamProperties;    /*!< the current stream's properties (eg. stream type) passed to dsp add-ons */
      int                               m_NewMasterMode;            /*!< if master mode is changed it set here and handled by process function */
      AUDIODSP_ADDON_STREAMTYPE         m_NewStreamType;            /*!< if stream type is changed it set here and handled by process function */
      enum AVMatrixEncoding             m_ffMpegMatrixEncoding;
      enum AVAudioServiceType           m_ffMpegAudioServiceType;
      int                               m_ffMpegProfile;
      SwrContext                       *m_convertInput;
      SwrContext                       *m_convertOutput;

      CCriticalSection                  m_critSection;
      CCriticalSection                  m_restartSection;

      /*!>
       * Selected dsp addon functions
       */
      struct sDSPProcessHandle
      {
        void Clear()
        {
          iAddonModeNumber = -1;
          iLastTime        = 0;
        }
        unsigned int        iAddonModeNumber;                       /*!< The identifier, send from addon during mode registration and can be used from addon to select mode from a function table */
        CActiveAEDSPModePtr pMode;                                  /*!< Processing mode information data */
        AE_DSP_ADDON        pAddon;                                 /*!< Addon control class */
        ADDON_HANDLE_STRUCT handle;
        uint64_t            iLastTime;                              /*!< last processing time of the mode */
      };
      std::vector <sDSPProcessHandle>   m_addons_InputProc;         /*!< Input processing list, called to all enabled dsp addons with the basic unchanged input stream, is read only. */
      sDSPProcessHandle                 m_addon_InputResample;      /*!< Input stream resampling over one on settings enabled input resample function only on one addon */
      std::vector <sDSPProcessHandle>   m_addons_PreProc;           /*!< Input stream preprocessing function calls set and aligned from dsp settings stored inside database */
      std::vector <sDSPProcessHandle>   m_addons_MasterProc;        /*!< The current from user selected master processing function on addon */
      int                               m_activeMode;               /*!< the current used master mode, is a pointer to m_addons_MasterProc */
      int                               m_activeModeOutChannels;    /*!< Amount of channels given from active master mode or -1 if unhandled */
      unsigned long                     m_activeModeOutChannelsPresent; /*! The exact present flags of output processing channels from active master mode */
      std::vector <sDSPProcessHandle>   m_addons_PostProc;          /*!< Output stream postprocessing function calls set and aligned from dsp settings stored inside database */
      sDSPProcessHandle                 m_addon_OutputResample;     /*!< Output stream resampling over one on settings enabled output resample function only on one addon */
      std::map<int,ADDON_HANDLE_STRUCT> m_addon_Handles;            /*!< Handle identifier for the called dsp functions */

      /*!>
       * Process arrays
       */
      float                            *m_processArray[2][AUDIODSP_ADDON_CH_MAX];
      unsigned int                      m_processArraySize;

      /*!>
       * CPU usage data
       */
      uint64_t                          m_iLastProcessTime;
      uint64_t                          m_iLastProcessUsage;
      float                             m_fLastProcessUsage;

      /*!>
       * Internal ffmpeg process data
       */
      IAEResample                      *m_resamplerDSPProcessor;       /*!< ffmpeg resampler usage for down mix of input stream to required output channel alignment or internal processing*/
      float                            *m_ffMpegConvertArray[2][AUDIODSP_ADDON_CH_MAX]; /*!< the process array memory pointers for ffmpeg used for format convert. No own memory only addresses taken from m_processArray in correct ffmpeg channel alignment */
      float                            *m_ffMpegProcessArray[2][AUDIODSP_ADDON_CH_MAX]; /*!< the process array memory pointers for ffmpeg. No own memory only addresses taken from m_processArray in correct ffmpeg channel alignment */

      /*!>
       * Index pointers for interleaved audio streams to detect correct channel alignment
       */
      int                               m_idx_in[AE_CH_MAX];
      uint64_t                          m_channelLayoutIn;
      int                               m_idx_out[AE_CH_MAX];
      uint64_t                          m_channelLayoutOut;
    //@}
  };
  //@}
}
