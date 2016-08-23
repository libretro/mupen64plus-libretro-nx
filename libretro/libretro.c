#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libretro.h"
#include "GLideN64_libretro.h"
#include <libco.h>

#include <glsm/glsmsym.h>

#include "api/m64p_frontend.h"
#include "plugin/plugin.h"
#include "api/m64p_types.h"
#include "r4300/r4300.h"
#include "memory/memory.h"
#include "main/main.h"
#include "main/version.h"
#include "main/savestates.h"
#include "main/rom.h"
#include "pi/pi_controller.h"
#include "si/pif.h"
#include "libretro_memory.h"

#include "audio_plugin.h"

#ifndef PRESCALE_WIDTH
#define PRESCALE_WIDTH  640
#endif

#ifndef PRESCALE_HEIGHT
#define PRESCALE_HEIGHT 625
#endif

struct retro_perf_callback perf_cb;
retro_get_cpu_features_t perf_get_cpu_features_cb = NULL;

retro_log_printf_t log_cb = NULL;
retro_video_refresh_t video_cb = NULL;
retro_input_poll_t poll_cb = NULL;
retro_input_state_t input_cb = NULL;
retro_audio_sample_batch_t audio_batch_cb = NULL;
retro_environment_t environ_cb = NULL;

struct retro_rumble_interface rumble;

save_memory_data saved_memory;

static cothread_t game_thread;
cothread_t retro_thread;

int astick_deadzone;

static uint8_t* game_data = NULL;
static uint32_t game_size = 0;

static bool     emu_initialized     = false;
static unsigned initial_boot        = true;
static unsigned audio_buffer_size   = 2048;

static unsigned retro_filtering     = 0;
static bool     first_context_reset = false;

uint32_t *blitter_buf;
uint32_t *blitter_buf_lock   = NULL;

uint32_t retro_screen_width;
uint32_t retro_screen_height;
u32 bilinearMode = 0;
u32 EnableNoise = 0;
u32 EnableLOD = 0;
u32 EnableHWLighting = 0;
u32 CorrectTexrectCoords = 0;
u32 enableNativeResTexrects = 0;
u32 enableLegacyBlending = 0;
u32 EnableFBEmulation = 0;
u32 UseNativeResolutionFactor = 0;
u32 EnableCopyAuxiliaryToRDRAM = 0;
u32 EnableCopyColorToRDRAM = 0;
u32 EnableCopyDepthToRDRAM = 0;
u32 EnableCopyColorFromRDRAM = 0;
u32 BufferSwapMode = 0;
f32 PolygonOffsetFactor = 0.0;
u32 EnableFragmentDepthWrite = 0;
// after the controller's CONTROL* member has been assigned we can update
// them straight from here...
extern struct
{
    CONTROL *control;
    BUTTONS buttons;
} controller[4];
// ...but it won't be at least the first time we're called, in that case set
// these instead for input_plugin to read.
int pad_pak_types[4];
int pad_present[4] = {1, 1, 1, 1};

static void n64DebugCallback(void* aContext, int aLevel, const char* aMessage)
{
    char buffer[1024];
    snprintf(buffer, 1024, "mupen64plus: %s\n", aMessage);
    if (log_cb)
       log_cb(RETRO_LOG_INFO, buffer);
}

extern m64p_rom_header ROM_HEADER;

static void setup_variables(void)
{
   struct retro_variable variables[] = {
      { "glupen64-cpucore",
#ifdef DYNAREC
#if defined(IOS)
         "CPU Core; cached_interpreter|pure_interpreter|dynamic_recompiler" },
#else
         "CPU Core; dynamic_recompiler|cached_interpreter|pure_interpreter" },
#endif
#else
         "CPU Core; cached_interpreter|pure_interpreter" },
#endif
      { "glupen64-screensize",
         "Resolution; 320x240|640x480|960x720|1280x960|1600x1200|1920x1440|2240x1680" },
      { "glupen64-BilinearMode",
         "Bilinear filtering mode; standard|3point" },
      { "glupen64-EnableFBEmulation",
#if defined(VC) || defined(ANDROID)
         "Enable frame buffer emulation; False|True" },
#else
         "Enable frame buffer emulation; True|False" },
#endif
      { "glupen64-UseNativeResolutionFactor",
         "Frame buffer size is the factor of N64 native resolution; 0|1|2|4" },
      { "glupen64-EnableCopyAuxiliaryToRDRAM",
         "Copy auxiliary buffers to RDRAM; False|True" },
      { "glupen64-EnableCopyColorToRDRAM",
#ifdef ANDROID
         "Enable color buffer copy to RDRAM; Off|Sync|Async" },
#else
         "Enable color buffer copy to RDRAM; Async|Sync|Off" },
#endif
      { "glupen64-EnableCopyDepthToRDRAM",
         "Enable depth buffer copy to RDRAM; Off|FromMem|Software" },
      { "glupen64-EnableCopyColorFromRDRAM",
         "Enable color buffer copy from RDRAM; False|True" },
      { "glupen64-BufferSwapMode",
         "When to swap buffers; VIupdate|OriginChange|BufferUpdate" },
      { "glupen64-EnableNoise",
         "Enable color noise emulation; True|False" },
      { "glupen64-EnableLOD",
         "Enable LOD emulation; True|False" },
      { "glupen64-EnableHWLighting",
         "Enable hardware per-pixel lighting; False|True" },
      { "glupen64-CorrectTexrectCoords",
         "Make texrect coordinates continuous; Off|Auto|Force" },
      { "glupen64-EnableNativeResTexrects",
         "Render 2D texrects in native resolution; False|True" },
      { "glupen64-EnableLegacyBlending",
#if defined(VC) || defined(ANDROID)
         "Faster but less accurate blending mode; True|False" },
#else
         "Faster but less accurate blending mode; False|True" },
#endif
      ( "glupen64-EnableFragmentDepthWrite",
#ifdef ANDROID
         "Enable writing of fragment depth; False|True" },
#else
         "Enable writing of fragment depth; True|False" },
#endif
#ifdef ANDROID
      { "glupen64-PolygonOffsetFactor",
         "Scale used to calculate depth values; -3.0f|0.2f|-1.5f|-0.2f|-0.001f|-2.0f" },
#endif
      {"glupen64-audio-buffer-size",
         "Audio Buffer Size (restart); 2048|1024"},
      {"glupen64-astick-deadzone",
        "Analog Deadzone (percent); 15|20|25|30|0|5|10"},
      {"glupen64-pak1",
        "Player 1 Pak; memory|rumble|none"},
      {"glupen64-pak2",
        "Player 2 Pak; memory|rumble|none"},
      {"glupen64-pak3",
        "Player 3 Pak; memory|rumble|none"},
      {"glupen64-pak4",
        "Player 4 Pak; memory|rumble|none"},
      { NULL, NULL },
   };

   environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
}


static bool emu_step_load_data()
{
   if(CoreStartup(FRONTEND_API_VERSION, ".", ".", "Core", n64DebugCallback, 0, 0) && log_cb)
       log_cb(RETRO_LOG_ERROR, "mupen64plus: Failed to initialize core\n");

   log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_ROM_OPEN\n");

   if(CoreDoCommand(M64CMD_ROM_OPEN, game_size, (void*)game_data))
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "mupen64plus: Failed to load ROM\n");
       goto load_fail;
   }

   free(game_data);
   game_data = NULL;

   log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_ROM_GET_HEADER\n");

   if(CoreDoCommand(M64CMD_ROM_GET_HEADER, sizeof(ROM_HEADER), &ROM_HEADER))
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "mupen64plus; Failed to query ROM header information\n");
      goto load_fail;
   }

   return true;

load_fail:
   free(game_data);
   game_data = NULL;
   stop = 1;

   return false;
}

static void emu_step_initialize(void)
{
   if (emu_initialized)
      return;

   emu_initialized = true;

   plugin_connect_all();
}

static void EmuThreadFunction(void)
{
   log_cb(RETRO_LOG_INFO, "EmuThread: M64CMD_EXECUTE. \n");

   CoreDoCommand(M64CMD_EXECUTE, 0, NULL);
}

void reinit_gfx_plugin(void)
{
    if(first_context_reset)
    {
        first_context_reset = false;
        emu_step_initialize();
    }
}

const char* retro_get_system_directory(void)
{
    const char* dir;
    environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir);

    return dir ? dir : ".";
}


void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb)   { }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_cb = cb; }


void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   setup_variables();
}

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "GLupeN64";
   info->library_version = "2.5";
   info->valid_extensions = "n64|v64|z64|bin|u1|ndd";
   info->need_fullpath = false;
   info->block_extract = false;
}

// Get the system type associated to a ROM country code.
static m64p_system_type rom_country_code_to_system_type(char country_code)
{
    switch (country_code)
    {
        // PAL codes
        case 0x44:
        case 0x46:
        case 0x49:
        case 0x50:
        case 0x53:
        case 0x55:
        case 0x58:
        case 0x59:
            return SYSTEM_PAL;

        // NTSC codes
        case 0x37:
        case 0x41:
        case 0x45:
        case 0x4a:
        default: // Fallback for unknown codes
            return SYSTEM_NTSC;
    }
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   m64p_system_type region = rom_country_code_to_system_type(ROM_HEADER.Country_code);

   info->geometry.base_width   = retro_screen_width;
   info->geometry.base_height  = retro_screen_height;
   info->geometry.max_width    = retro_screen_width;
   info->geometry.max_height   = retro_screen_height;
   info->geometry.aspect_ratio = 4.0 / 3.0;
   //info->timing.fps = (region == SYSTEM_PAL) ? 50.0 : (60/1.001);
   info->timing.fps = (double)ROM_PARAMS.vilimit;
   info->timing.sample_rate = 44100.0;
}

unsigned retro_get_region (void)
{
   m64p_system_type region = rom_country_code_to_system_type(ROM_HEADER.Country_code);
   return ((region == SYSTEM_PAL) ? RETRO_REGION_PAL : RETRO_REGION_NTSC);
}

void retro_init(void)
{
   struct retro_log_callback log;
   unsigned colorMode = RETRO_PIXEL_FORMAT_XRGB8888;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf_cb))
      perf_get_cpu_features_cb = perf_cb.get_cpu_features;
   else
      perf_get_cpu_features_cb = NULL;

   environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &colorMode);
   environ_cb(RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE, &rumble);

   blitter_buf = (uint32_t*)calloc(
         PRESCALE_WIDTH * PRESCALE_HEIGHT, sizeof(uint32_t)
         );
   blitter_buf_lock = blitter_buf;

   retro_thread = co_active();
   game_thread = co_create(65536 * sizeof(void*) * 16, EmuThreadFunction);
}

void retro_deinit(void)
{
   main_stop();

   if (blitter_buf)
      free(blitter_buf);
   blitter_buf      = NULL;
   blitter_buf_lock = NULL;

   deinit_audio_libretro();

   if (perf_cb.perf_log)
      perf_cb.perf_log();
}

extern void ChangeSize();

void update_variables(bool startup)
{
   struct retro_variable var;

   var.key = "glupen64-BilinearMode";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "3point"))
         bilinearMode = 0;
      else
         bilinearMode = 1;
   }

   var.key = "glupen64-EnableFBEmulation";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "False"))
         EnableFBEmulation = 0;
      else
         EnableFBEmulation = 1;
   }

   var.key = "glupen64-UseNativeResolutionFactor";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "4"))
         UseNativeResolutionFactor = 4;
      else if (!strcmp(var.value, "3"))
         UseNativeResolutionFactor = 3;
      else if (!strcmp(var.value, "2"))
         UseNativeResolutionFactor = 2;
      else if (!strcmp(var.value, "1"))
         UseNativeResolutionFactor = 1;
      else
         UseNativeResolutionFactor = 0;
   }

   var.key = "glupen64-EnableCopyAuxiliaryToRDRAM";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "True"))
         EnableCopyAuxiliaryToRDRAM = 1;
      else
         EnableCopyAuxiliaryToRDRAM = 0;
   }

   var.key = "glupen64-EnableCopyColorToRDRAM";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "Async"))
         EnableCopyColorToRDRAM = 2;
      else if (!strcmp(var.value, "Sync"))
         EnableCopyColorToRDRAM = 1;
      else
         EnableCopyColorToRDRAM = 0;
   }

   var.key = "glupen64-EnableCopyDepthToRDRAM";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "Software"))
         EnableCopyDepthToRDRAM = 2;
      else if (!strcmp(var.value, "FromMem"))
         EnableCopyDepthToRDRAM = 1;
      else
         EnableCopyDepthToRDRAM = 0;
   }

   var.key = "glupen64-EnableCopyColorFromRDRAM";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "True"))
         EnableCopyColorFromRDRAM = 1;
      else
         EnableCopyColorFromRDRAM = 0;
   }

   var.key = "glupen64-EnableFragmentDepthWrite";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "True"))
         EnableFragmentDepthWrite = 1;
      else
         EnableFragmentDepthWrite = 0;
   }

   var.key = "glupen64-BufferSwapMode";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "BufferUpdate"))
         BufferSwapMode = 2;
      else if (!strcmp(var.value, "OriginChange"))
         BufferSwapMode = 1;
      else
         BufferSwapMode = 0;
   }

   var.key = "glupen64-EnableNoise";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "False"))
         EnableNoise = 0;
      else
         EnableNoise = 1;
   }

   var.key = "glupen64-EnableLOD";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "False"))
         EnableLOD = 0;
      else
         EnableLOD = 1;
   }

   var.key = "glupen64-EnableHWLighting";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "True"))
         EnableHWLighting = 1;
      else
         EnableHWLighting = 0;
   }

   var.key = "glupen64-CorrectTexrectCoords";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "Force"))
         CorrectTexrectCoords = 2;
      else if (!strcmp(var.value, "Auto"))
         CorrectTexrectCoords = 1;
      else
         CorrectTexrectCoords = 0;
   }

   var.key = "glupen64-EnableNativeResTexrects";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "True"))
         enableNativeResTexrects = 1;
      else
         enableNativeResTexrects = 0;
   }

   var.key = "glupen64-EnableLegacyBlending";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "True"))
         enableLegacyBlending = 1;
      else
         enableLegacyBlending = 0;
   }
#ifdef ANDROID
   var.key = "glupen64-PolygonOffsetFactor";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "0.2f"))
         PolygonOffsetFactor = 0.2f;
      else if (!strcmp(var.value, "-1.5f"))
         PolygonOffsetFactor = -1.5f;
      else if (!strcmp(var.value, "-0.2f"))
         PolygonOffsetFactor = -0.2f;
      else if (!strcmp(var.value, "-0.001f"))
         PolygonOffsetFactor = -0.001f;
      else if (!strcmp(var.value, "-2.0f"))
         PolygonOffsetFactor = -2.0f;
      else
         PolygonOffsetFactor = -3.0f;
   }
#endif
   var.key = "glupen64-cpucore";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "pure_interpreter"))
          r4300emu = 0;
      else if (!strcmp(var.value, "cached_interpreter"))
          r4300emu = 1;
      else if (!strcmp(var.value, "dynamic_recompiler"))
          r4300emu = 2;
   }

   var.key = "glupen64-screensize";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (sscanf(var.value ? var.value : "640x480", "%dx%d", &retro_screen_width, &retro_screen_height) != 2)
      {
         retro_screen_width = 640;
         retro_screen_height = 480;
      }
   }

   if (startup)
   {
      var.key = "glupen64-audio-buffer-size";
      var.value = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
         audio_buffer_size = atoi(var.value);
   }

   var.key = "glupen64-astick-deadzone";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
      astick_deadzone = (int)(atoi(var.value) * 0.01f * 0x8000);

   {
      struct retro_variable pk1var = { "glupen64-pak1" };
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk1var) && pk1var.value)
      {
         int p1_pak = PLUGIN_NONE;
         if (!strcmp(pk1var.value, "rumble"))
            p1_pak = PLUGIN_RAW;
         else if (!strcmp(pk1var.value, "memory"))
            p1_pak = PLUGIN_MEMPAK;
         
         // If controller struct is not initialised yet, set pad_pak_types instead
         // which will be looked at when initialising the controllers.
         if (controller[0].control)
            controller[0].control->Plugin = p1_pak;
         else
            pad_pak_types[0] = p1_pak;
         
      }
   }

   {
      struct retro_variable pk2var = { "glupen64-pak2" };
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk2var) && pk2var.value)
      {
         int p2_pak = PLUGIN_NONE;
         if (!strcmp(pk2var.value, "rumble"))
            p2_pak = PLUGIN_RAW;
         else if (!strcmp(pk2var.value, "memory"))
            p2_pak = PLUGIN_MEMPAK;
            
         if (controller[1].control)
            controller[1].control->Plugin = p2_pak;
         else
            pad_pak_types[1] = p2_pak;
         
      }
   }
   
   {
      struct retro_variable pk3var = { "glupen64-pak3" };
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk3var) && pk3var.value)
      {
         int p3_pak = PLUGIN_NONE;
         if (!strcmp(pk3var.value, "rumble"))
            p3_pak = PLUGIN_RAW;
         else if (!strcmp(pk3var.value, "memory"))
            p3_pak = PLUGIN_MEMPAK;
            
         if (controller[2].control)
            controller[2].control->Plugin = p3_pak;
         else
            pad_pak_types[2] = p3_pak;
         
      }
   }
  
   {
      struct retro_variable pk4var = { "glupen64-pak4" };
      if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk4var) && pk4var.value)
      {
         int p4_pak = PLUGIN_NONE;
         if (!strcmp(pk4var.value, "rumble"))
            p4_pak = PLUGIN_RAW;
         else if (!strcmp(pk4var.value, "memory"))
            p4_pak = PLUGIN_MEMPAK;
            
         if (controller[3].control)
            controller[3].control->Plugin = p4_pak;
         else
            pad_pak_types[3] = p4_pak;
      }
   }


}

static void format_saved_memory(void)
{
   format_sram(saved_memory.sram);
   format_eeprom(saved_memory.eeprom, sizeof(saved_memory.eeprom));
   format_flashram(saved_memory.flashram);
   format_mempak(saved_memory.mempack[0]);
   format_mempak(saved_memory.mempack[1]);
   format_mempak(saved_memory.mempack[2]);
   format_mempak(saved_memory.mempack[3]);
}

static void context_reset(void)
{
   static bool first_init = true;
   printf("context_reset.\n");
   glsm_ctl(GLSM_CTL_STATE_CONTEXT_RESET, NULL);

   if (first_init)
   {
      glsm_ctl(GLSM_CTL_STATE_SETUP, NULL);
      first_init = false;
   }

   reinit_gfx_plugin();
}

static void context_destroy(void)
{
   glsm_ctl(GLSM_CTL_STATE_CONTEXT_DESTROY, NULL);
}

static bool context_framebuffer_lock(void *data)
{
   if (!stop)
      return false;
   return true;
}

bool retro_load_game(const struct retro_game_info *game)
{
   glsm_ctx_params_t params = {0};
   format_saved_memory();

   update_variables(true);
   initial_boot = false;

   init_audio_libretro(audio_buffer_size);

   params.context_reset         = context_reset;
   params.context_destroy       = context_destroy;
   params.environ_cb            = environ_cb;
   params.stencil               = false;

   params.framebuffer_lock      = context_framebuffer_lock;

   if (!glsm_ctl(GLSM_CTL_STATE_CONTEXT_INIT, &params))
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "mupen64plus: libretro frontend doesn't have OpenGL support.");
      return false;
   }

   game_data = malloc(game->size);
   memcpy(game_data, game->data, game->size);
   game_size = game->size;

   if (!emu_step_load_data())
      return false;

   first_context_reset = true;

   return true;
}

void retro_unload_game(void)
{
   stop = 1;

   CoreDoCommand(M64CMD_ROM_CLOSE, 0, NULL);
   emu_initialized = false;
}

void retro_run (void)
{
   blitter_buf_lock = blitter_buf;
   glsm_ctl(GLSM_CTL_STATE_BIND, NULL);
   co_switch(game_thread);
   glsm_ctl(GLSM_CTL_STATE_UNBIND, NULL);
   video_cb(RETRO_HW_FRAME_BUFFER_VALID, retro_screen_width, retro_screen_height, 0);
}

void retro_reset (void)
{
    CoreDoCommand(M64CMD_RESET, 1, (void*)0);
}

void *retro_get_memory_data(unsigned type)
{
   return (type == RETRO_MEMORY_SAVE_RAM) ? &saved_memory : 0;
}

size_t retro_get_memory_size(unsigned type)
{
   return (type == RETRO_MEMORY_SAVE_RAM) ? sizeof(saved_memory) : 0;
}



size_t retro_serialize_size (void)
{
    return 16788288 + 1024; // < 16MB and some change... ouch
}

bool retro_serialize(void *data, size_t size)
{
    if (savestates_save_m64p(data, size))
        return true;

    return false;
}

bool retro_unserialize(const void * data, size_t size)
{
    if (savestates_load_m64p(data, size))
        return true;

    return false;
}

//Needed to be able to detach controllers for Lylat Wars multiplayer
//Only sets if controller struct is initialised as addon paks do.
void retro_set_controller_port_device(unsigned in_port, unsigned device) {
    if (in_port < 4){
        switch(device)
        {
            case RETRO_DEVICE_NONE:
                if (controller[in_port].control){
                    controller[in_port].control->Present = 0;
                    break;
                } else {
                    pad_present[in_port] = 0;
                    break;
                }
                
            case RETRO_DEVICE_JOYPAD:
            default:
                if (controller[in_port].control){
                    controller[in_port].control->Present = 1;
                    break;
                } else {
                    pad_present[in_port] = 1;
                    break;
                }
        }
    }
}

unsigned retro_api_version(void) { return RETRO_API_VERSION; }
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) { return false; }
void retro_cheat_reset(void) { }
void retro_cheat_set(unsigned unused, bool unused1, const char* unused2) { }

int retro_return(int just_flipping)
{
   co_switch(retro_thread);

   return 0;
}

uint32_t get_retro_screen_width()
{
   return retro_screen_width;
}

uint32_t get_retro_screen_height()
{
   return retro_screen_height;
}
