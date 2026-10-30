/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - audio_backend_compat.c                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2014 Bobby Smiles                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "api/m64p_types.h"
#include <libretro.h>
#include "device/rcp/ai/ai_controller.h"
#include "../../../../mupen64plus-core/src/main/main.h"
#include "../../../../mupen64plus-core/src/device/device.h"
#include "../../../../mupen64plus-core/src/main/rom.h"
#include "plugin/plugin.h"
#include "device/rcp/ri/ri_controller.h"
#include "device/rcp/vi/vi_controller.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __APPLE__
#include <AudioToolbox/AudioToolbox.h>
#else
#include <audio/conversion/float_to_s16.h>
#include <audio/conversion/s16_to_float.h>
#include <audio/audio_resampler.h>
#endif

extern retro_audio_sample_batch_t audio_batch_cb;

static unsigned MAX_AUDIO_FRAMES = 2048;

#define VI_INTR_TIME 500000
#define OUTPUT_RATE 44100

/* Read header for type definition */
static int GameFreq = 33600;
static unsigned CountsPerSecond;
static unsigned BytesPerSecond;
static unsigned CountsPerByte;

#ifdef __APPLE__
/* Apple AudioConverter - resamples int16 to 44100 Hz.
 * No float intermediates needed (unlike sinc resampler path). */
static AudioConverterRef audio_converter;
static unsigned converter_input_rate;
static int16_t *audio_out_buffer_s16;

/* Input accumulation buffer - keeps data between calls so the converter
 * never sees end-of-stream (which would clear its filter state and
 * cause audio pops at buffer boundaries). */
static int16_t *converter_input_buf;
static size_t input_buf_frames;

/* Margin of input frames to always keep buffered. Ensures the
 * converter's input callback never returns 0 (end-of-stream). */
#define CONVERTER_MARGIN 64

/* Context for AudioConverter input callback */
typedef struct
{
   const int16_t *data;
   size_t frames_left;
} converter_ctx_t;

static OSStatus converter_input_cb(
      AudioConverterRef converter,
      UInt32 *ioNumberDataPackets,
      AudioBufferList *ioData,
      AudioStreamPacketDescription **outDataPacketDescription,
      void *inUserData)
{
   converter_ctx_t *ctx = (converter_ctx_t *)inUserData;
   UInt32 frames_to_provide;

   if (ctx->frames_left == 0)
   {
      *ioNumberDataPackets = 0;
      return noErr;
   }

   frames_to_provide = *ioNumberDataPackets;
   if (frames_to_provide > ctx->frames_left)
      frames_to_provide = (UInt32)ctx->frames_left;

   ioData->mBuffers[0].mData           = (void *)ctx->data;
   ioData->mBuffers[0].mDataByteSize   = frames_to_provide * 4; /* stereo int16 */
   ioData->mBuffers[0].mNumberChannels = 2;

   ctx->data        += frames_to_provide * 2; /* advance by samples */
   ctx->frames_left -= frames_to_provide;
   *ioNumberDataPackets = frames_to_provide;

   return noErr;
}

static int create_audio_converter(unsigned input_rate)
{
   AudioStreamBasicDescription input_desc, output_desc;
   OSStatus err;

   if (audio_converter)
   {
      AudioConverterDispose(audio_converter);
      audio_converter = NULL;
   }

   /* Input: native-endian int16 stereo at game's sample rate
    * (byte swap is done before calling AudioConverter) */
   memset(&input_desc, 0, sizeof(input_desc));
   input_desc.mSampleRate       = input_rate;
   input_desc.mFormatID         = kAudioFormatLinearPCM;
   input_desc.mFormatFlags      = kAudioFormatFlagIsSignedInteger
                                | kAudioFormatFlagIsPacked;
   input_desc.mBytesPerPacket   = 4;
   input_desc.mFramesPerPacket  = 1;
   input_desc.mBytesPerFrame    = 4;
   input_desc.mChannelsPerFrame = 2;
   input_desc.mBitsPerChannel   = 16;

   /* Output: little-endian int16 stereo at 44100 Hz */
   memset(&output_desc, 0, sizeof(output_desc));
   output_desc.mSampleRate       = OUTPUT_RATE;
   output_desc.mFormatID         = kAudioFormatLinearPCM;
   output_desc.mFormatFlags      = kAudioFormatFlagIsSignedInteger
                                 | kAudioFormatFlagIsPacked;
   output_desc.mBytesPerPacket   = 4;
   output_desc.mFramesPerPacket  = 1;
   output_desc.mBytesPerFrame    = 4;
   output_desc.mChannelsPerFrame = 2;
   output_desc.mBitsPerChannel   = 16;

   err = AudioConverterNew(&input_desc, &output_desc, &audio_converter);
   if (err != noErr)
      return -1;

   /* Set high quality resampling */
   UInt32 quality = kAudioConverterQuality_High;
   AudioConverterSetProperty(audio_converter,
         kAudioConverterSampleRateConverterQuality,
         sizeof(quality), &quality);

   /* Disable priming - avoids initial silence/transient when the
    * filter pipeline is empty at stream start or after rate change. */
   UInt32 primeMethod = kConverterPrimeMethod_None;
   AudioConverterSetProperty(audio_converter,
         kAudioConverterPrimeMethod,
         sizeof(primeMethod), &primeMethod);

   converter_input_rate = input_rate;
   return 0;
}

#else
/* Non-Apple: use libretro-common sinc resampler */
static const retro_resampler_t *resampler;
static void *resampler_audio_data;
static float *audio_in_buffer_float;
static float *audio_out_buffer_float;
static int16_t *audio_out_buffer_s16;

void (*audio_convert_s16_to_float_arm)(float *out,
      const int16_t *in, size_t samples, float gain);
void (*audio_convert_float_to_s16_arm)(int16_t *out,
      const float *in, size_t samples);
#endif

void deinit_audio_libretro(void)
{
#ifdef __APPLE__
   if (audio_converter)
   {
      AudioConverterDispose(audio_converter);
      audio_converter = NULL;
   }
   free(audio_out_buffer_s16);
   audio_out_buffer_s16 = NULL;
   free(converter_input_buf);
   converter_input_buf = NULL;
   input_buf_frames = 0;
#else
   if (resampler && resampler_audio_data)
   {
      resampler->free(resampler_audio_data);
      resampler = NULL;
      resampler_audio_data = NULL;
      free(audio_in_buffer_float);
      free(audio_out_buffer_float);
      free(audio_out_buffer_s16);
   }
#endif
}

void init_audio_libretro(unsigned max_audio_frames)
{
   MAX_AUDIO_FRAMES = max_audio_frames;

#ifdef __APPLE__
   /* Allocate output buffer - sized for maximum expansion ratio */
   audio_out_buffer_s16 = malloc(2 * MAX_AUDIO_FRAMES * 2 * sizeof(int16_t));
   /* Allocate input accumulation buffer */
   converter_input_buf = malloc(2 * MAX_AUDIO_FRAMES * 2 * sizeof(int16_t));
   input_buf_frames = 0;
   /* Converter will be created on first use when we know the input rate */
   audio_converter = NULL;
   converter_input_rate = 0;
#else
   retro_resampler_realloc(&resampler_audio_data, &resampler, "sinc", RESAMPLER_QUALITY_DONTCARE, 1.0);

   audio_in_buffer_float  = malloc(2 * MAX_AUDIO_FRAMES * sizeof(float));
   audio_out_buffer_float = malloc(2 * MAX_AUDIO_FRAMES * sizeof(float));
   audio_out_buffer_s16   = malloc(2 * MAX_AUDIO_FRAMES * sizeof(int16_t));

   convert_s16_to_float_init_simd();
   convert_float_to_s16_init_simd();
#endif
}

static void aiDacrateChanged(void *user_data, unsigned int frequency)
{
   GameFreq        = frequency;
   BytesPerSecond  = frequency * 4;
   CountsPerSecond = VI_INTR_TIME * 60 /* TODO/FIXME - dehardcode */;
   CountsPerByte   = CountsPerSecond / BytesPerSecond;

#if 0
   printf("CountsPerByte: %d, GameFreq: %d\n", CountsPerByte, GameFreq);
#endif
}

/* A fully compliant implementation is not really possible with just the zilmar spec.
 * We assume bits == 16 (assumption compatible with audio-sdl plugin implementation)
 */
void set_audio_format_via_libretro(void* user_data,
      unsigned int frequency)
{
   struct ai_controller* ai = (struct ai_controller*)user_data;
   uint32_t saved_ai_dacrate = ai->regs[AI_DACRATE_REG];

   /* notify plugin of the new frequency (can't do the same for bits) */
   ai->regs[AI_DACRATE_REG] = ai->vi->clock / frequency - 1;

   aiDacrateChanged(user_data, frequency);

   /* restore original registers values */
   ai->regs[AI_DACRATE_REG] = saved_ai_dacrate;
}

static void aiLenChanged(void* user_data, const void* buffer, size_t size)
{
   uint32_t i;
   int16_t *out      = NULL;
   int16_t *raw_data = (int16_t*)buffer;
   size_t frames     = size / 4;
   uint8_t *p        = (uint8_t*)buffer;

   /* Byte swap - handles endianness and channel order from N64 format */
   for (i = 0; i < size; i += 4)
   {
      p[i ] ^= p[i + 2];
      p[i + 2] ^= p[i ];
      p[i ] ^= p[i + 2];
      p[i + 1] ^= p[i + 3];
      p[i + 3] ^= p[i + 1];
      p[i + 1] ^= p[i + 3];
   }

#ifdef __APPLE__
   /* Create or recreate converter if sample rate changed */
   if (!audio_converter || converter_input_rate != (unsigned)GameFreq)
   {
      if (create_audio_converter(GameFreq) != 0)
         return;
      /* Discard any data buffered at the old sample rate */
      input_buf_frames = 0;
   }

   /* Append byte-swapped input to accumulation buffer */
   if (input_buf_frames + frames > MAX_AUDIO_FRAMES * 2)
   {
      size_t to_drop = (input_buf_frames + frames) - MAX_AUDIO_FRAMES * 2;
      if (to_drop > input_buf_frames)
         to_drop = input_buf_frames;
      memmove(converter_input_buf,
              converter_input_buf + to_drop * 2,
              (input_buf_frames - to_drop) * 4);
      input_buf_frames -= to_drop;
   }
   memcpy(converter_input_buf + input_buf_frames * 2, raw_data, frames * 4);
   input_buf_frames += frames;

   /* Wait until we have enough data for the resampling filter margin */
   if (input_buf_frames <= CONVERTER_MARGIN)
      return;

   {
      converter_ctx_t ctx;
      AudioBufferList output_buffer;
      UInt32 output_frames;
      OSStatus err;
      size_t convertible = input_buf_frames - CONVERTER_MARGIN;

      /* Request output for the convertible portion, leaving
       * CONVERTER_MARGIN frames so the input callback never
       * returns 0 (which would signal end-of-stream). */
      output_frames = (UInt32)((convertible * OUTPUT_RATE) / GameFreq + 1);
      if (output_frames > MAX_AUDIO_FRAMES * 2)
         output_frames = MAX_AUDIO_FRAMES * 2;

      ctx.data        = converter_input_buf;
      ctx.frames_left = input_buf_frames;

      output_buffer.mNumberBuffers = 1;
      output_buffer.mBuffers[0].mNumberChannels = 2;
      output_buffer.mBuffers[0].mDataByteSize   = output_frames * 4;
      output_buffer.mBuffers[0].mData           = audio_out_buffer_s16;

      err = AudioConverterFillComplexBuffer(audio_converter,
            converter_input_cb, &ctx,
            &output_frames, &output_buffer, NULL);

      if (err != noErr && err != 1)
         return;

      /* Remove consumed frames from accumulation buffer */
      if (ctx.frames_left < input_buf_frames)
      {
         input_buf_frames = ctx.frames_left;
         if (input_buf_frames > 0)
            memmove(converter_input_buf, ctx.data,
                    input_buf_frames * 4);
      }

      out = audio_out_buffer_s16;
      while (output_frames)
      {
         size_t ret     = audio_batch_cb(out, output_frames);
         output_frames -= ret;
         out           += ret * 2;
      }
   }

#else
   /* Non-Apple path: sinc resampler with float conversion */
   size_t max_frames, remain_frames;
   double ratio;
   struct resampler_data data = {0};

audio_batch:
   out               = NULL;
   ratio             = 44100.0 / GameFreq;
   max_frames        = (GameFreq > 44100) ? MAX_AUDIO_FRAMES : (size_t)(MAX_AUDIO_FRAMES / ratio - 1);
   remain_frames     = 0;

   if (frames > max_frames)
   {
      remain_frames = frames - max_frames;
      frames = max_frames;
   }

   data.data_in      = audio_in_buffer_float;
   data.data_out     = audio_out_buffer_float;
   data.input_frames = frames;
   data.ratio        = ratio;

   convert_s16_to_float(audio_in_buffer_float, raw_data, frames * 2, 1.0f);
   resampler->process(resampler_audio_data, &data);
   convert_float_to_s16(audio_out_buffer_s16, audio_out_buffer_float, data.output_frames * 2);

   out                    = audio_out_buffer_s16;

   while (data.output_frames)
   {
      size_t ret          = audio_batch_cb(out, data.output_frames);
      data.output_frames -= ret;
      out                += ret * 2;
   }
   if (remain_frames)
   {
      raw_data = raw_data + frames * 2;
      frames   = remain_frames;
      goto audio_batch;
   }
#endif
}

/* Abuse core & audio plugin implementation details to obtain the desired effect. */
void push_audio_samples_via_libretro(void* user_data, const void* buffer, size_t size)
{
   struct ai_controller* ai = (struct ai_controller*)user_data;
   uint32_t saved_ai_length = ai->regs[AI_LEN_REG];
   uint32_t saved_ai_dram = ai->regs[AI_DRAM_ADDR_REG];

   /* notify plugin of new samples to play.
    * Exploit the fact that buffer points in ai->ri->rdram.dram to retrieve dram_addr_reg value */
   ai->regs[AI_DRAM_ADDR_REG] = (uint8_t*)buffer - (uint8_t*)g_dev.ri.rdram->dram;
   ai->regs[AI_LEN_REG] = size;

   aiLenChanged(user_data, buffer, size);

   /* restore original registers vlaues */
   ai->regs[AI_LEN_REG]       = saved_ai_length;
   ai->regs[AI_DRAM_ADDR_REG] = saved_ai_dram;
}
