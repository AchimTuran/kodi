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
#ifndef __STDC_LIMIT_MACROS
  #define __STDC_LIMIT_MACROS
#endif

#include "AEUtil.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

#include <cassert>

extern "C" {
#include "libavutil/channel_layout.h"
}

/* declare the rng seed and initialize it */
unsigned int CAEUtil::m_seed = (unsigned int)(CurrentHostCounter() / 1000.0f);
#if defined(HAVE_SSE2) && defined(__SSE2__)
  /* declare the SSE seed and initialize it */
  MEMALIGN(16, __m128i CAEUtil::m_sseSeed) = _mm_set_epi32(CAEUtil::m_seed, CAEUtil::m_seed+1, CAEUtil::m_seed, CAEUtil::m_seed+1);
#endif

void AEDelayStatus::SetDelay(double d)
{
  delay = d;
  maxcorrection = d;
  tick = CurrentHostCounter();
}

double AEDelayStatus::GetDelay()
{
  double d = 0;
  if (tick)
    d = (double)(CurrentHostCounter() - tick) / CurrentHostFrequency();
  if (d > maxcorrection)
    d = maxcorrection;

  return delay - d;
}

CAEChannelInfo CAEUtil::GuessChLayout(const unsigned int channels)
{
  CLog::Log(LOGWARNING, "CAEUtil::GuessChLayout - "
    "This method should really never be used, please fix the code that called this");

  CAEChannelInfo result;
  if (channels < 1 || channels > 8)
    return result;

  switch (channels)
  {
    case 1: result = AE_CH_LAYOUT_1_0; break;
    case 2: result = AE_CH_LAYOUT_2_0; break;
    case 3: result = AE_CH_LAYOUT_3_0; break;
    case 4: result = AE_CH_LAYOUT_4_0; break;
    case 5: result = AE_CH_LAYOUT_5_0; break;
    case 6: result = AE_CH_LAYOUT_5_1; break;
    case 7: result = AE_CH_LAYOUT_7_0; break;
    case 8: result = AE_CH_LAYOUT_7_1; break;
  }

  return result;
}

const char* CAEUtil::GetStdChLayoutName(const enum AEStdChLayout layout)
{
  if (layout < 0 || layout >= AE_CH_LAYOUT_MAX)
    return "UNKNOWN";

  static const char* layouts[AE_CH_LAYOUT_MAX] =
  {
    "1.0",
    "2.0", "2.1", "3.0", "3.1", "4.0",
    "4.1", "5.0", "5.1", "7.0", "7.1"
  };

  return layouts[layout];
}

const unsigned int CAEUtil::DataFormatToBits(const enum AEDataFormat dataFormat)
{
  if (dataFormat < 0 || dataFormat >= AE_FMT_MAX)
    return 0;

  static const unsigned int formats[AE_FMT_MAX] =
  {
    8,                   /* U8     */

    16,                  /* S16BE  */
    16,                  /* S16LE  */
    16,                  /* S16NE  */
    
    32,                  /* S32BE  */
    32,                  /* S32LE  */
    32,                  /* S32NE  */
    
    32,                  /* S24BE  */
    32,                  /* S24LE  */
    32,                  /* S24NE  */
    32,                  /* S24NER */
    
    24,                  /* S24BE3 */
    24,                  /* S24LE3 */
    24,                  /* S24NE3 */
    
    sizeof(double) << 3, /* DOUBLE */
    sizeof(float ) << 3, /* FLOAT  */

     8,                  /* RAW    */

     8,                  /* U8P    */
    16,                  /* S16NEP */
    32,                  /* S32NEP */
    32,                  /* S24NEP */
    32,                  /* S24NERP*/
    24,                  /* S24NE3P*/
    sizeof(double) << 3, /* DOUBLEP */
    sizeof(float ) << 3  /* FLOATP  */
 };

  return formats[dataFormat];
}

const unsigned int CAEUtil::DataFormatToUsedBits(const enum AEDataFormat dataFormat)
{
  if (dataFormat == AE_FMT_S24BE4 || dataFormat == AE_FMT_S24LE4 ||
      dataFormat == AE_FMT_S24NE4 || dataFormat == AE_FMT_S24NE4MSB)
    return 24;
  else
    return DataFormatToBits(dataFormat);
}

const unsigned int CAEUtil::DataFormatToDitherBits(const enum AEDataFormat dataFormat)
{
  if (dataFormat == AE_FMT_S24NE4MSB)
    return 8;
  if (dataFormat == AE_FMT_S24NE3)
    return -8;
  else
    return 0;
}

const char* CAEUtil::StreamTypeToStr(const enum CAEStreamInfo::DataType dataType)
{
  switch (dataType)
  {
    case CAEStreamInfo::STREAM_TYPE_AC3:
      return "STREAM_TYPE_AC3";
    case CAEStreamInfo::STREAM_TYPE_DTSHD:
      return "STREAM_TYPE_DTSHD";
    case CAEStreamInfo::STREAM_TYPE_DTSHD_CORE:
      return "STREAM_TYPE_DTSHD_CORE";
    case CAEStreamInfo::STREAM_TYPE_DTS_1024:
      return "STREAM_TYPE_DTS_1024";
    case CAEStreamInfo::STREAM_TYPE_DTS_2048:
      return "STREAM_TYPE_DTS_2048";
    case CAEStreamInfo::STREAM_TYPE_DTS_512:
      return "STREAM_TYPE_DTS_512";
    case CAEStreamInfo::STREAM_TYPE_EAC3:
      return "STREAM_TYPE_EAC3";
    case CAEStreamInfo::STREAM_TYPE_MLP:
      return "STREAM_TYPE_MLP";
    case CAEStreamInfo::STREAM_TYPE_TRUEHD:
      return "STREAM_TYPE_TRUEHD";

    default:
      return "STREAM_TYPE_NULL";
  }
}

const char* CAEUtil::DataFormatToStr(const enum AEDataFormat dataFormat)
{
  if (dataFormat < 0 || dataFormat >= AE_FMT_MAX)
    return "UNKNOWN";

  static const char *formats[AE_FMT_MAX] =
  {
    "AE_FMT_U8",

    "AE_FMT_S16BE",
    "AE_FMT_S16LE",
    "AE_FMT_S16NE",
    
    "AE_FMT_S32BE",
    "AE_FMT_S32LE",
    "AE_FMT_S32NE",
    
    "AE_FMT_S24BE4",
    "AE_FMT_S24LE4",
    "AE_FMT_S24NE4",  /* S24 in 4 bytes */
    "AE_FMT_S24NE4MSB",
    
    "AE_FMT_S24BE3",
    "AE_FMT_S24LE3",
    "AE_FMT_S24NE3", /* S24 in 3 bytes */
    
    "AE_FMT_DOUBLE",
    "AE_FMT_FLOAT",
    
    "AE_FMT_RAW",

    /* planar formats */
    "AE_FMT_U8P",
    "AE_FMT_S16NEP",
    "AE_FMT_S32NEP",
    "AE_FMT_S24NE4P",
    "AE_FMT_S24NE4MSBP",
    "AE_FMT_S24NE3P",
    "AE_FMT_DOUBLEP",
    "AE_FMT_FLOATP"
  };

  return formats[dataFormat];
}

#if defined(HAVE_SSE) && defined(__SSE__)
void CAEUtil::SSEMulArray(float *data, const float mul, uint32_t count)
{
  const __m128 m = _mm_set_ps1(mul);

  /* work around invalid alignment */
  while (((uintptr_t)data & 0xF) && count > 0)
  {
    data[0] *= mul;
    ++data;
    --count;
  }

  uint32_t even = count & ~0x3;
  for (uint32_t i = 0; i < even; i+=4, data+=4)
  {
    __m128 to      = _mm_load_ps(data);
    *(__m128*)data = _mm_mul_ps (to, m);
  }

  if (even != count)
  {
    uint32_t odd = count - even;
    if (odd == 1)
      data[0] *= mul;
    else
    {
      __m128 to;
      if (odd == 2)
      {
        to = _mm_setr_ps(data[0], data[1], 0, 0);
        __m128 ou = _mm_mul_ps(to, m);
        data[0] = ((float*)&ou)[0];
        data[1] = ((float*)&ou)[1];
      }
      else
      {
        to = _mm_setr_ps(data[0], data[1], data[2], 0);
        __m128 ou = _mm_mul_ps(to, m);
        data[0] = ((float*)&ou)[0];
        data[1] = ((float*)&ou)[1];
        data[2] = ((float*)&ou)[2];
      }
    }
  }
}

void CAEUtil::SSEMulAddArray(float *data, float *add, const float mul, uint32_t count)
{
  const __m128 m = _mm_set_ps1(mul);

  /* work around invalid alignment */
  while ((((uintptr_t)data & 0xF) || ((uintptr_t)add & 0xF)) && count > 0)
  {
    data[0] += add[0] * mul;
    ++add;
    ++data;
    --count;
  }

  uint32_t even = count & ~0x3;
  for (uint32_t i = 0; i < even; i+=4, data+=4, add+=4)
  {
    __m128 ad      = _mm_load_ps(add );
    __m128 to      = _mm_load_ps(data);
    *(__m128*)data = _mm_add_ps (to, _mm_mul_ps(ad, m));
  }

  if (even != count)
  {
    uint32_t odd = count - even;
    if (odd == 1)
      data[0] += add[0] * mul;
    else
    {
      __m128 ad;
      __m128 to;
      if (odd == 2)
      {
        ad = _mm_setr_ps(add [0], add [1], 0, 0);
        to = _mm_setr_ps(data[0], data[1], 0, 0);
        __m128 ou = _mm_add_ps(to, _mm_mul_ps(ad, m));
        data[0] = ((float*)&ou)[0];
        data[1] = ((float*)&ou)[1];
      }
      else
      {
        ad = _mm_setr_ps(add [0], add [1], add [2], 0);
        to = _mm_setr_ps(data[0], data[1], data[2], 0);
        __m128 ou = _mm_add_ps(to, _mm_mul_ps(ad, m));
        data[0] = ((float*)&ou)[0];
        data[1] = ((float*)&ou)[1];
        data[2] = ((float*)&ou)[2];
      }
    }
  }
}
#endif

inline float CAEUtil::SoftClamp(const float x)
{
#if 1
    /*
       This is a rational function to approximate a tanh-like soft clipper.
       It is based on the pade-approximation of the tanh function with tweaked coefficients.
       See: http://www.musicdsp.org/showone.php?id=238
    */
    if (x < -3.0f)
      return -1.0f;
    else if (x >  3.0f)
      return 1.0f;
    float y = x * x;
    return x * (27.0f + y) / (27.0f + 9.0f * y);
#else
    /* slower method using tanh, but more accurate */

    static const double k = 0.9f;
    /* perform a soft clamp */
    if (x >  k)
      x = (float) (tanh((x - k) / (1 - k)) * (1 - k) + k);
    else if (x < -k)
      x = (float) (tanh((x + k) / (1 - k)) * (1 - k) - k);

    /* hard clamp anything still outside the bounds */
    if (x >  1.0f)
      return  1.0f;
    if (x < -1.0f)
      return -1.0f;

    /* return the final sample */
    return x;
#endif
}

void CAEUtil::ClampArray(float *data, uint32_t count)
{
#if !defined(HAVE_SSE) || !defined(__SSE__)
  for (uint32_t i = 0; i < count; ++i)
    data[i] = SoftClamp(data[i]);

#else
  const __m128 c1 = _mm_set_ps1(27.0f);
  const __m128 c2 = _mm_set_ps1(27.0f + 9.0f);

  /* work around invalid alignment */
  while (((uintptr_t)data & 0xF) && count > 0)
  {
    data[0] = SoftClamp(data[0]);
    ++data;
    --count;
  }

  uint32_t even = count & ~0x3;
  for (uint32_t i = 0; i < even; i+=4, data+=4)
  {
    /* tanh approx clamp */
    __m128 dt = _mm_load_ps(data);
    __m128 tmp     = _mm_mul_ps(dt, dt);
    *(__m128*)data = _mm_div_ps(
      _mm_mul_ps(
        dt,
        _mm_add_ps(c1, tmp)
      ),
      _mm_add_ps(c2, tmp)
    );
  }

  if (even != count)
  {
    uint32_t odd = count - even;
    if (odd == 1)
      data[0] = SoftClamp(data[0]);
    else
    {
      __m128 dt;
      __m128 tmp;
      __m128 out;
      if (odd == 2)
      {
        /* tanh approx clamp */
        dt  = _mm_setr_ps(data[0], data[1], 0, 0);
        tmp = _mm_mul_ps(dt, dt);
        out = _mm_div_ps(
          _mm_mul_ps(
            dt,
            _mm_add_ps(c1, tmp)
          ),
          _mm_add_ps(c2, tmp)
        );

        data[0] = ((float*)&out)[0];
        data[1] = ((float*)&out)[1];
      }
      else
      {
        /* tanh approx clamp */
        dt  = _mm_setr_ps(data[0], data[1], data[2], 0);
        tmp = _mm_mul_ps(dt, dt);
        out = _mm_div_ps(
          _mm_mul_ps(
            dt,
            _mm_add_ps(c1, tmp)
          ),
          _mm_add_ps(c2, tmp)
        );

        data[0] = ((float*)&out)[0];
        data[1] = ((float*)&out)[1];
        data[2] = ((float*)&out)[2];
      }
    }
  }
#endif
}

bool CAEUtil::S16NeedsByteSwap(AEDataFormat in, AEDataFormat out)
{
  const AEDataFormat nativeFormat =
#ifdef WORDS_BIGENDIAN
    AE_FMT_S16BE;
#else
    AE_FMT_S16LE;
#endif

  if (in == AE_FMT_S16NE || (in == AE_FMT_RAW))
    in = nativeFormat;
  if (out == AE_FMT_S16NE || (out == AE_FMT_RAW))
    out = nativeFormat;

  return in != out;
}

uint64_t CAEUtil::GetAVChannelLayout(const CAEChannelInfo &info)
{
  uint64_t channelLayout = 0;
  if (info.HasChannel(AE_CH_FL))   channelLayout |= AV_CH_FRONT_LEFT;
  if (info.HasChannel(AE_CH_FR))   channelLayout |= AV_CH_FRONT_RIGHT;
  if (info.HasChannel(AE_CH_FC))   channelLayout |= AV_CH_FRONT_CENTER;
  if (info.HasChannel(AE_CH_LFE))  channelLayout |= AV_CH_LOW_FREQUENCY;
  if (info.HasChannel(AE_CH_BL))   channelLayout |= AV_CH_BACK_LEFT;
  if (info.HasChannel(AE_CH_BR))   channelLayout |= AV_CH_BACK_RIGHT;
  if (info.HasChannel(AE_CH_FLOC)) channelLayout |= AV_CH_FRONT_LEFT_OF_CENTER;
  if (info.HasChannel(AE_CH_FROC)) channelLayout |= AV_CH_FRONT_RIGHT_OF_CENTER;
  if (info.HasChannel(AE_CH_BC))   channelLayout |= AV_CH_BACK_CENTER;
  if (info.HasChannel(AE_CH_SL))   channelLayout |= AV_CH_SIDE_LEFT;
  if (info.HasChannel(AE_CH_SR))   channelLayout |= AV_CH_SIDE_RIGHT;
  if (info.HasChannel(AE_CH_TC))   channelLayout |= AV_CH_TOP_CENTER;
  if (info.HasChannel(AE_CH_TFL))  channelLayout |= AV_CH_TOP_FRONT_LEFT;
  if (info.HasChannel(AE_CH_TFC))  channelLayout |= AV_CH_TOP_FRONT_CENTER;
  if (info.HasChannel(AE_CH_TFR))  channelLayout |= AV_CH_TOP_FRONT_RIGHT;
  if (info.HasChannel(AE_CH_TBL))   channelLayout |= AV_CH_TOP_BACK_LEFT;
  if (info.HasChannel(AE_CH_TBC))   channelLayout |= AV_CH_TOP_BACK_CENTER;
  if (info.HasChannel(AE_CH_TBR))   channelLayout |= AV_CH_TOP_BACK_RIGHT;

  return channelLayout;
}

CAEChannelInfo CAEUtil::GetAEChannelLayout(uint64_t layout)
{
  CAEChannelInfo channelLayout;
  channelLayout.Reset();

  if (layout & AV_CH_FRONT_LEFT)       channelLayout += AE_CH_FL;
  if (layout & AV_CH_FRONT_RIGHT)      channelLayout += AE_CH_FR;
  if (layout & AV_CH_FRONT_CENTER)     channelLayout += AE_CH_FC;
  if (layout & AV_CH_LOW_FREQUENCY)    channelLayout += AE_CH_LFE;
  if (layout & AV_CH_BACK_LEFT)        channelLayout += AE_CH_BL;
  if (layout & AV_CH_BACK_RIGHT)       channelLayout += AE_CH_BR;
  if (layout & AV_CH_FRONT_LEFT_OF_CENTER)  channelLayout += AE_CH_FLOC;
  if (layout & AV_CH_FRONT_RIGHT_OF_CENTER) channelLayout += AE_CH_FROC;
  if (layout & AV_CH_BACK_CENTER)      channelLayout += AE_CH_BC;
  if (layout & AV_CH_SIDE_LEFT)        channelLayout += AE_CH_SL;
  if (layout & AV_CH_SIDE_RIGHT)       channelLayout += AE_CH_SR;
  if (layout & AV_CH_TOP_CENTER)       channelLayout += AE_CH_TC;
  if (layout & AV_CH_TOP_FRONT_LEFT)   channelLayout += AE_CH_TFL;
  if (layout & AV_CH_TOP_FRONT_CENTER) channelLayout += AE_CH_TFC;
  if (layout & AV_CH_TOP_FRONT_RIGHT)  channelLayout += AE_CH_TFR;
  if (layout & AV_CH_TOP_BACK_LEFT)    channelLayout += AE_CH_BL;
  if (layout & AV_CH_TOP_BACK_CENTER)  channelLayout += AE_CH_BC;
  if (layout & AV_CH_TOP_BACK_RIGHT)   channelLayout += AE_CH_BR;

  return channelLayout;
}

AVSampleFormat CAEUtil::GetAVSampleFormat(AEDataFormat format)
{
  switch (format)
  {
    case AEDataFormat::AE_FMT_U8:
      return AV_SAMPLE_FMT_U8;
    case AEDataFormat::AE_FMT_S16NE:
      return AV_SAMPLE_FMT_S16;
    case AEDataFormat::AE_FMT_S32NE:
      return AV_SAMPLE_FMT_S32;
    case AEDataFormat::AE_FMT_S24NE4:
      return AV_SAMPLE_FMT_S32;
    case AEDataFormat::AE_FMT_S24NE4MSB:
      return AV_SAMPLE_FMT_S32;
    case AEDataFormat::AE_FMT_S24NE3:
      return AV_SAMPLE_FMT_S32;
    case AEDataFormat::AE_FMT_FLOAT:
      return AV_SAMPLE_FMT_FLT;
    case AEDataFormat::AE_FMT_DOUBLE:
      return AV_SAMPLE_FMT_DBL;
    case AEDataFormat::AE_FMT_U8P:
      return AV_SAMPLE_FMT_U8P;
    case AEDataFormat::AE_FMT_S16NEP:
      return AV_SAMPLE_FMT_S16P;
    case AEDataFormat::AE_FMT_S32NEP:
      return AV_SAMPLE_FMT_S32P;
    case AEDataFormat::AE_FMT_S24NE4P:
      return AV_SAMPLE_FMT_S32P;
    case AEDataFormat::AE_FMT_S24NE4MSBP:
      return AV_SAMPLE_FMT_S32P;
    case AEDataFormat::AE_FMT_S24NE3P:
      return AV_SAMPLE_FMT_S32P;
    case AEDataFormat::AE_FMT_FLOATP:
      return AV_SAMPLE_FMT_FLTP;
    case AEDataFormat::AE_FMT_DOUBLEP:
      return AV_SAMPLE_FMT_DBLP;
    case AEDataFormat::AE_FMT_RAW:
      return AV_SAMPLE_FMT_U8;
    case AEDataFormat::AE_FMT_MAX:
    case AEDataFormat::AE_FMT_INVALID:
      return AV_SAMPLE_FMT_NONE;
    default:
    {
      if (AE_IS_PLANAR(format))
        return AV_SAMPLE_FMT_FLTP;
      else
        return AV_SAMPLE_FMT_FLT;
    }
  }
}

uint64_t CAEUtil::GetAVChannel(enum AEChannel aechannel)
{
  switch (aechannel)
  {
  case AE_CH_FL:   return AV_CH_FRONT_LEFT;
  case AE_CH_FR:   return AV_CH_FRONT_RIGHT;
  case AE_CH_FC:   return AV_CH_FRONT_CENTER;
  case AE_CH_LFE:  return AV_CH_LOW_FREQUENCY;
  case AE_CH_BL:   return AV_CH_BACK_LEFT;
  case AE_CH_BR:   return AV_CH_BACK_RIGHT;
  case AE_CH_FLOC: return AV_CH_FRONT_LEFT_OF_CENTER;
  case AE_CH_FROC: return AV_CH_FRONT_RIGHT_OF_CENTER;
  case AE_CH_BC:   return AV_CH_BACK_CENTER;
  case AE_CH_SL:   return AV_CH_SIDE_LEFT;
  case AE_CH_SR:   return AV_CH_SIDE_RIGHT;
  case AE_CH_TC:   return AV_CH_TOP_CENTER;
  case AE_CH_TFL:  return AV_CH_TOP_FRONT_LEFT;
  case AE_CH_TFC:  return AV_CH_TOP_FRONT_CENTER;
  case AE_CH_TFR:  return AV_CH_TOP_FRONT_RIGHT;
  case AE_CH_TBL:  return AV_CH_TOP_BACK_LEFT;
  case AE_CH_TBC:  return AV_CH_TOP_BACK_CENTER;
  case AE_CH_TBR:  return AV_CH_TOP_BACK_RIGHT;
  default:
    return 0;
  }
}

int CAEUtil::GetAVChannelIndex(enum AEChannel aechannel, uint64_t layout)
{
  return av_get_channel_layout_channel_index(layout, GetAVChannel(aechannel));
}

AEMatrixEncoding CAEUtil::GetAEMatrixEncoding(AVMatrixEncoding AVEncoding)
{
  switch (AVEncoding)
  {
    case AV_MATRIX_ENCODING_NONE:
      return AE_MATRIX_ENCODING_NONE;

    case AV_MATRIX_ENCODING_DOLBY:
      return AE_MATRIX_ENCODING_DOLBY;

    case AV_MATRIX_ENCODING_DPLII:
      return AE_MATRIX_ENCODING_DPLII;

    case AV_MATRIX_ENCODING_DPLIIX:
      return AE_MATRIX_ENCODING_DPLIIX;

    case AV_MATRIX_ENCODING_DPLIIZ:
      return AE_MATRIX_ENCODING_DPLIIZ;

    case AV_MATRIX_ENCODING_DOLBYEX:
      return AE_MATRIX_ENCODING_DOLBYEX;

    case AV_MATRIX_ENCODING_DOLBYHEADPHONE:
      return AE_MATRIX_ENCODING_DOLBYHEADPHONE;

    default:
      return AE_MATRIX_ENCODING_NONE;
  }
}

AEAudioServiceType CAEUtil::GetAEAudioServiceType(AVAudioServiceType AVServiceType)
{
  switch (AVServiceType)
  {
    case AV_AUDIO_SERVICE_TYPE_MAIN:
      return AE_AUDIO_SERVICE_TYPE_MAIN;

    case AV_AUDIO_SERVICE_TYPE_EFFECTS:
      return AE_AUDIO_SERVICE_TYPE_EFFECTS;

    case AV_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED:
      return AE_AUDIO_SERVICE_TYPE_VISUALLY_IMPAIRED;

    case AV_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED:
      return AE_AUDIO_SERVICE_TYPE_HEARING_IMPAIRED;

    case AV_AUDIO_SERVICE_TYPE_DIALOGUE:
      return AE_AUDIO_SERVICE_TYPE_DIALOGUE;

    case AV_AUDIO_SERVICE_TYPE_COMMENTARY:
      return AE_AUDIO_SERVICE_TYPE_COMMENTARY;

    case AV_AUDIO_SERVICE_TYPE_EMERGENCY:
      return AE_AUDIO_SERVICE_TYPE_EMERGENCY;

    case AV_AUDIO_SERVICE_TYPE_VOICE_OVER:
      return AE_AUDIO_SERVICE_TYPE_VOICE_OVER;

    case AV_AUDIO_SERVICE_TYPE_KARAOKE:
      return AE_AUDIO_SERVICE_TYPE_KARAOKE;

    default:
      return AE_AUDIO_SERVICE_INVALID;
  }
}

AESourceFormat CAEUtil::GetAESourceFormat(AVCodecID AVCodec)
{
  switch (AVCodec)
  {
    case AV_CODEC_ID_PCM_S16LE:
    case AV_CODEC_ID_PCM_S16BE:
    case AV_CODEC_ID_PCM_U16LE:
    case AV_CODEC_ID_PCM_U16BE:
    case AV_CODEC_ID_PCM_S8:
    case AV_CODEC_ID_PCM_U8:
    case AV_CODEC_ID_PCM_MULAW:
    case AV_CODEC_ID_PCM_ALAW:
    case AV_CODEC_ID_PCM_S32LE:
    case AV_CODEC_ID_PCM_S32BE:
    case AV_CODEC_ID_PCM_U32LE:
    case AV_CODEC_ID_PCM_U32BE:
    case AV_CODEC_ID_PCM_S24LE:
    case AV_CODEC_ID_PCM_S24BE:
    case AV_CODEC_ID_PCM_U24LE:
    case AV_CODEC_ID_PCM_U24BE:
    case AV_CODEC_ID_PCM_S24DAUD:
    case AV_CODEC_ID_PCM_ZORK:
    case AV_CODEC_ID_PCM_S16LE_PLANAR:
    case AV_CODEC_ID_PCM_DVD:
    case AV_CODEC_ID_PCM_F32BE:
    case AV_CODEC_ID_PCM_F32LE:
    case AV_CODEC_ID_PCM_F64BE:
    case AV_CODEC_ID_PCM_F64LE:
    case AV_CODEC_ID_PCM_BLURAY:
    case AV_CODEC_ID_PCM_LXF:
    case AV_CODEC_ID_S302M:
    case AV_CODEC_ID_PCM_S8_PLANAR:
    case AV_CODEC_ID_PCM_S24LE_PLANAR:
    case AV_CODEC_ID_PCM_S32LE_PLANAR:
    case AV_CODEC_ID_PCM_S16BE_PLANAR:
    case AV_CODEC_ID_PCM_S64LE:
    case AV_CODEC_ID_PCM_S64BE:
    case AV_CODEC_ID_PCM_F16LE:
    case AV_CODEC_ID_PCM_F24LE:
      return AE_SOURCE_FORMAT_PCM;

    /* various ADPCM codecs */
    case AV_CODEC_ID_ADPCM_IMA_QT:
    case AV_CODEC_ID_ADPCM_IMA_WAV:
    case AV_CODEC_ID_ADPCM_IMA_DK3:
    case AV_CODEC_ID_ADPCM_IMA_DK4:
    case AV_CODEC_ID_ADPCM_IMA_WS:
    case AV_CODEC_ID_ADPCM_IMA_SMJPEG:
    case AV_CODEC_ID_ADPCM_MS:
    case AV_CODEC_ID_ADPCM_4XM:
    case AV_CODEC_ID_ADPCM_XA:
    case AV_CODEC_ID_ADPCM_ADX:
    case AV_CODEC_ID_ADPCM_EA:
    case AV_CODEC_ID_ADPCM_G726:
    case AV_CODEC_ID_ADPCM_CT:
    case AV_CODEC_ID_ADPCM_SWF:
    case AV_CODEC_ID_ADPCM_YAMAHA:
    case AV_CODEC_ID_ADPCM_SBPRO_4:
    case AV_CODEC_ID_ADPCM_SBPRO_3:
    case AV_CODEC_ID_ADPCM_SBPRO_2:
    case AV_CODEC_ID_ADPCM_THP:
    case AV_CODEC_ID_ADPCM_IMA_AMV:
    case AV_CODEC_ID_ADPCM_EA_R1:
    case AV_CODEC_ID_ADPCM_EA_R3:
    case AV_CODEC_ID_ADPCM_EA_R2:
    case AV_CODEC_ID_ADPCM_IMA_EA_SEAD:
    case AV_CODEC_ID_ADPCM_IMA_EA_EACS:
    case AV_CODEC_ID_ADPCM_EA_XAS:
    case AV_CODEC_ID_ADPCM_EA_MAXIS_XA:
    case AV_CODEC_ID_ADPCM_IMA_ISS:
    case AV_CODEC_ID_ADPCM_G722:
    case AV_CODEC_ID_ADPCM_IMA_APC:
    case AV_CODEC_ID_ADPCM_VIMA:
    case AV_CODEC_ID_ADPCM_AFC:
    case AV_CODEC_ID_ADPCM_IMA_OKI:
    case AV_CODEC_ID_ADPCM_DTK:
    case AV_CODEC_ID_ADPCM_IMA_RAD:
    case AV_CODEC_ID_ADPCM_G726LE:
    case AV_CODEC_ID_ADPCM_THP_LE:
    case AV_CODEC_ID_ADPCM_PSX:
    case AV_CODEC_ID_ADPCM_AICA:
    case AV_CODEC_ID_ADPCM_IMA_DAT4:
    case AV_CODEC_ID_ADPCM_MTAF:
      return AE_SOURCE_FORMAT_ADPCM;

    /* AMR */
    //! @todo AudioDSP V2 add AMR and RealAudio codecs
    //case AV_CODEC_ID_AMR_NB:
    //case AV_CODEC_ID_AMR_WB:
    /* RealAudio codecs*/
    //case AV_CODEC_ID_RA_144:
    //case AV_CODEC_ID_RA_288:

    /* various DPCM codecs */
    case AV_CODEC_ID_ROQ_DPCM:
    case AV_CODEC_ID_INTERPLAY_DPCM:
    case AV_CODEC_ID_XAN_DPCM:
    case AV_CODEC_ID_SOL_DPCM:
    case AV_CODEC_ID_SDX2_DPCM:
      return AE_SOURCE_FORMAT_DPCM;

    /* audio codecs */
    case AV_CODEC_ID_MP2:
      return AE_SOURCE_FORMAT_MP2;
    case AV_CODEC_ID_MP3: ///< preferred ID for decoding MPEG audio layer 1: 2 or 3
      return AE_SOURCE_FORMAT_MP3;
    case AV_CODEC_ID_AAC:
      return AE_SOURCE_FORMAT_AAC;
    case AV_CODEC_ID_AC3:
      return AE_SOURCE_FORMAT_AC3;
    case AV_CODEC_ID_DTS:
      return AE_SOURCE_FORMAT_DTS;
    case AV_CODEC_ID_VORBIS:
      return AE_SOURCE_FORMAT_VORBIS;
    case AV_CODEC_ID_DVAUDIO:
      return AE_SOURCE_FORMAT_DVAUDIO;
    case AV_CODEC_ID_WMAV1:
      return AE_SOURCE_FORMAT_WMAV1;
    case AV_CODEC_ID_WMAV2:
      return AE_SOURCE_FORMAT_WMAV2;
    case AV_CODEC_ID_MACE3:
      return AE_SOURCE_FORMAT_MACE3;
    case AV_CODEC_ID_MACE6:
      return AE_SOURCE_FORMAT_MACE6;
    case AV_CODEC_ID_VMDAUDIO:
      return AE_SOURCE_FORMAT_VMDAUDIO;
    case AV_CODEC_ID_FLAC:
      return AE_SOURCE_FORMAT_FLAC;
    case AV_CODEC_ID_MP3ADU:
      return AE_SOURCE_FORMAT_MP3ADU;
    case AV_CODEC_ID_MP3ON4:
      return AE_SOURCE_FORMAT_MP3ON4;
    case AV_CODEC_ID_SHORTEN:
      return AE_SOURCE_FORMAT_SHORTEN;
    case AV_CODEC_ID_ALAC:
      return AE_SOURCE_FORMAT_ALAC;
    case AV_CODEC_ID_WESTWOOD_SND1:
      return AE_SOURCE_FORMAT_WESTWOOD_SND1;
    case AV_CODEC_ID_GSM: ///< as in Berlin toast format
      return AE_SOURCE_FORMAT_GSM; ///< as in Berlin toast format
    case AV_CODEC_ID_QDM2:
      return AE_SOURCE_FORMAT_QDM2;
    case AV_CODEC_ID_COOK:
      return AE_SOURCE_FORMAT_COOK;
    case AV_CODEC_ID_TRUESPEECH:
      return AE_SOURCE_FORMAT_TRUESPEECH;
    case AV_CODEC_ID_TTA:
      return AE_SOURCE_FORMAT_TTA;
    case AV_CODEC_ID_SMACKAUDIO:
      return AE_SOURCE_FORMAT_SMACKAUDIO;
    case AV_CODEC_ID_QCELP:
      return AE_SOURCE_FORMAT_QCELP;
    case AV_CODEC_ID_WAVPACK:
      return AE_SOURCE_FORMAT_WAVPACK;
    case AV_CODEC_ID_DSICINAUDIO:
      return AE_SOURCE_FORMAT_DSICINAUDIO;
    case AV_CODEC_ID_IMC:
      return AE_SOURCE_FORMAT_IMC;
    case AV_CODEC_ID_MUSEPACK7:
      return AE_SOURCE_FORMAT_MUSEPACK7;
    case AV_CODEC_ID_MLP:
      return AE_SOURCE_FORMAT_MLP;
    case AV_CODEC_ID_GSM_MS: /* as found in WAV */
      return AE_SOURCE_FORMAT_GSM_MS; ///< as found in WAV
    case AV_CODEC_ID_ATRAC3:
      return AE_SOURCE_FORMAT_ATRAC3;
#if FF_API_VOXWARE
    case AV_CODEC_ID_VOXWARE:
      return AE_SOURCE_FORMAT_VOXWARE;
#endif
    case AV_CODEC_ID_APE:
      return AE_SOURCE_FORMAT_APE;
    case AV_CODEC_ID_NELLYMOSER:
      return AE_SOURCE_FORMAT_NELLYMOSER;
    case AV_CODEC_ID_MUSEPACK8:
      return AE_SOURCE_FORMAT_MUSEPACK8;
    case AV_CODEC_ID_SPEEX:
      return AE_SOURCE_FORMAT_SPEEX;
    case AV_CODEC_ID_WMAVOICE:
      return AE_SOURCE_FORMAT_WMAVOICE;
    case AV_CODEC_ID_WMAPRO:
      return AE_SOURCE_FORMAT_WMAPRO;
    case AV_CODEC_ID_WMALOSSLESS:
      return AE_SOURCE_FORMAT_WMALOSSLESS;
    case AV_CODEC_ID_ATRAC3P:
      return AE_SOURCE_FORMAT_ATRAC3P;
    case AV_CODEC_ID_EAC3:
      return AE_SOURCE_FORMAT_EAC3;
    case AV_CODEC_ID_SIPR:
      return AE_SOURCE_FORMAT_SIPR;
    case AV_CODEC_ID_MP1:
      return AE_SOURCE_FORMAT_MP1;
    case AV_CODEC_ID_TWINVQ:
      return AE_SOURCE_FORMAT_TWINVQ;
    case AV_CODEC_ID_TRUEHD:
      return AE_SOURCE_FORMAT_TRUEHD;
    case AV_CODEC_ID_MP4ALS:
      return AE_SOURCE_FORMAT_MP4ALS;
    case AV_CODEC_ID_ATRAC1:
      return AE_SOURCE_FORMAT_ATRAC1;
    case AV_CODEC_ID_BINKAUDIO_RDFT:
      return AE_SOURCE_FORMAT_BINKAUDIO_RDFT;
    case AV_CODEC_ID_BINKAUDIO_DCT:
      return AE_SOURCE_FORMAT_BINKAUDIO_DCT;
    case AV_CODEC_ID_AAC_LATM:
      return AE_SOURCE_FORMAT_AAC_LATM;
    case AV_CODEC_ID_QDMC:
      return AE_SOURCE_FORMAT_QDMC;
    case AV_CODEC_ID_CELT:
      return AE_SOURCE_FORMAT_CELT;
    case AV_CODEC_ID_G723_1:
      return AE_SOURCE_FORMAT_G723_1;
    case AV_CODEC_ID_G729:
      return AE_SOURCE_FORMAT_G729;
    case AV_CODEC_ID_8SVX_EXP:
      return AE_SOURCE_FORMAT_8SVX_EXP;
    case AV_CODEC_ID_8SVX_FIB:
      return AE_SOURCE_FORMAT_8SVX_FIB;
    case AV_CODEC_ID_BMV_AUDIO:
      return AE_SOURCE_FORMAT_BMV_AUDIO;
    case AV_CODEC_ID_RALF:
      return AE_SOURCE_FORMAT_RALF;
    case AV_CODEC_ID_IAC:
      return AE_SOURCE_FORMAT_IAC;
    case AV_CODEC_ID_ILBC:
      return AE_SOURCE_FORMAT_ILBC;
    case AV_CODEC_ID_OPUS:
      return AE_SOURCE_FORMAT_OPUS;
    case AV_CODEC_ID_COMFORT_NOISE:
      return AE_SOURCE_FORMAT_COMFORT_NOISE;
    case AV_CODEC_ID_TAK:
      return AE_SOURCE_FORMAT_TAK;
    case AV_CODEC_ID_METASOUND:
      return AE_SOURCE_FORMAT_METASOUND;
    case AV_CODEC_ID_PAF_AUDIO:
      return AE_SOURCE_FORMAT_PAF_AUDIO;
    case AV_CODEC_ID_ON2AVC:
      return AE_SOURCE_FORMAT_ON2AVC;
    case AV_CODEC_ID_DSS_SP:
      return AE_SOURCE_FORMAT_DSS_SP;

    case AV_CODEC_ID_FFWAVESYNTH:
      return AE_SOURCE_FORMAT_FFWAVESYNTH;
    case AV_CODEC_ID_SONIC:
      return AE_SOURCE_FORMAT_SONIC;
    case AV_CODEC_ID_SONIC_LS:
      return AE_SOURCE_FORMAT_SONIC_LS;
    case AV_CODEC_ID_EVRC:
      return AE_SOURCE_FORMAT_EVRC;
    case AV_CODEC_ID_SMV:
      return AE_SOURCE_FORMAT_SMV;
    case AV_CODEC_ID_DSD_LSBF:
      return AE_SOURCE_FORMAT_DSD_LSBF;
    case AV_CODEC_ID_DSD_MSBF:
      return AE_SOURCE_FORMAT_DSD_MSBF;
    case AV_CODEC_ID_DSD_LSBF_PLANAR:
      return AE_SOURCE_FORMAT_DSD_LSBF_PLANAR;
    case AV_CODEC_ID_DSD_MSBF_PLANAR:
      return AE_SOURCE_FORMAT_DSD_MSBF_PLANAR;
    case AV_CODEC_ID_4GV:
      return AE_SOURCE_FORMAT_4GV;
    case AV_CODEC_ID_INTERPLAY_ACM:
      return AE_SOURCE_FORMAT_INTERPLAY_ACM;
    case AV_CODEC_ID_XMA1:
      return AE_SOURCE_FORMAT_XMA1;
    case AV_CODEC_ID_XMA2:
      return AE_SOURCE_FORMAT_XMA2;
    case AV_CODEC_ID_DST:
      return AE_SOURCE_FORMAT_DST;
    case AV_CODEC_ID_ATRAC3AL:
      return AE_SOURCE_FORMAT_ATRAC3AL;
    case AV_CODEC_ID_ATRAC3PAL:
      return AE_SOURCE_FORMAT_ATRAC3PAL;
    default:
      return AE_SOURCE_FORMAT_UNKNOWN;
    }

    //! @todo AudioDSP V2 where should these IDs be assigned?
    //return AE_SOURCE_FORMAT_STEREO;
    //return AE_SOURCE_FORMAT_MONO;
    //return AE_SOURCE_FORMAT_MULTICHANNEL;
    //return AE_SOURCE_FORMAT_DTSHD_MA;
    //return AE_SOURCE_FORMAT_DTSHD_HRA;
}

AEProfile CAEUtil::GetAEProfile(int Profile)
{
  switch (Profile)
  {
    case FF_PROFILE_AAC_MAIN:
      return AE_PROFILE_AAC_MAIN;
    case FF_PROFILE_AAC_LOW:
      return AE_PROFILE_AAC_LOW;
    case FF_PROFILE_AAC_SSR:
      return AE_PROFILE_AAC_SSR;
    case FF_PROFILE_AAC_LTP:
      return AE_PROFILE_AAC_LTP;
    case FF_PROFILE_AAC_HE:
      return AE_PROFILE_AAC_HE;
    case FF_PROFILE_AAC_HE_V2:
      return AE_PROFILE_AAC_HE_V2;
    case FF_PROFILE_AAC_LD:
      return AE_PROFILE_AAC_LD;
    case FF_PROFILE_AAC_ELD:
      return AE_PROFILE_AAC_ELD;
    case FF_PROFILE_MPEG2_AAC_LOW:
      return AE_PROFILE_MPEG2_AAC_LOW;
    case FF_PROFILE_MPEG2_AAC_HE:
      return AE_PROFILE_MPEG2_AAC_HE;
    case FF_PROFILE_DTS:
      return AE_PROFILE_DTS;
    case FF_PROFILE_DTS_ES:
      return AE_PROFILE_DTS_ES;
    case FF_PROFILE_DTS_96_24:
      return AE_PROFILE_DTS_96_24;
    case FF_PROFILE_DTS_HD_HRA:
      return AE_PROFILE_DTS_HD_HRA;
    case FF_PROFILE_DTS_HD_MA:
      return AE_PROFILE_DTS_HD_MA;
    case FF_PROFILE_DTS_EXPRESS:
      return AE_PROFILE_DTS_EXPRESS;
    default:
      return AE_PROFILE_UNKNOWN;
  }
}
