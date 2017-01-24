#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libretro.h"
#include "libretro_private.h"
#include "GLideN64_libretro.h"

#include <libco.h>

#include <glsm/glsmsym.h>

#include "api/m64p_frontend.h"
#include "plugin/plugin.h"
#include "api/m64p_types.h"
#include "r4300/r4300.h"
#include "memory/memory.h"
#include "main/main.h"
#include "main/cheat.h"
#include "main/version.h"
#include "main/savestates.h"
#include "main/mupen64plus.ini.h"
#include "api/m64p_config.h"
#include "osal_files.h"
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

#define PATH_SIZE 2048

#define ISHEXDEC ((codeLine[cursor]>='0') && (codeLine[cursor]<='9')) || ((codeLine[cursor]>='a') && (codeLine[cursor]<='f')) || ((codeLine[cursor]>='A') && (codeLine[cursor]<='F'))

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
int astick_sensitivity;
int r_cbutton;
int l_cbutton;
int d_cbutton;
int u_cbutton;

static uint8_t* game_data = NULL;
static uint32_t game_size = 0;

static bool     emu_initialized     = false;
static unsigned initial_boot        = true;
static unsigned audio_buffer_size   = 2048;

static unsigned retro_filtering     = 0;
static bool     first_context_reset = false;
static bool     initializing        = true;

uint32_t retro_screen_width;
uint32_t retro_screen_height;
float retro_screen_aspect;

uint32_t bilinearMode = 0;
uint32_t EnableHWLighting = 0;
uint32_t CorrectTexrectCoords = 0;
uint32_t enableNativeResTexrects = 0;
uint32_t enableLegacyBlending = 0;
uint32_t EnableCopyColorToRDRAM = 0;
uint32_t EnableCopyDepthToRDRAM = 0;
uint32_t AspectRatio = 0;
uint32_t txFilterMode = 0;
uint32_t txEnhancementMode = 0;
uint32_t txHiresEnable = 0;
uint32_t txHiresFullAlphaChannel = 0;
uint32_t txFilterIgnoreBG = 0;
uint32_t MultiSampling = 0;
uint32_t EnableFragmentDepthWrite = 0;
uint32_t EnableShadersStorage = 0;
uint32_t CropMode = 0;
uint32_t EnableFBEmulation = 0;
uint32_t CountPerOp = 0;

int rspMode = 0;
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
            "CPU Core; dynamic_recompiler|cached_interpreter|pure_interpreter" },
#else
            "CPU Core; cached_interpreter|pure_interpreter" },
#endif
        { "glupen64-rspmode",
#ifndef VC
            "RSP Mode; HLE|LLE" },
#else
            "RSP Mode; HLE" },
#endif
        { "glupen64-43screensize",
            "4:3 Resolution; 320x240|640x480|960x720|1280x960|1600x1200|1920x1440" },
        { "glupen64-169screensize",
            "16:9 Resolution; 640x360|960x540|1280x720|1920x1080|3840x2160" },
        { "glupen64-aspect",
            "Aspect Ratio; 4:3|16:9|16:9 adjusted" },
        { "glupen64-BilinearMode",
            "Bilinear filtering mode; standard|3point" },
#ifndef HAVE_OPENGLES2
        { "glupen64-MultiSampling",
            "MSAA level; 0|2|4|8|16" },
#endif
        { "glupen64-EnableFBEmulation",
            "Framebuffer Emulation; True|False" },
        { "glupen64-EnableCopyColorToRDRAM",
#ifndef HAVE_OPENGLES
            "Color buffer to RDRAM; Async|Sync|Off" },
#else
            "Color buffer to RDRAM; Off|Async|Sync" },
#endif
        { "glupen64-EnableCopyDepthToRDRAM",
            "Depth buffer to RDRAM; Software|FromMem|Off" },
        { "glupen64-EnableHWLighting",
            "Hardware per-pixel lighting; False|True" },
        { "glupen64-CorrectTexrectCoords",
            "Continuous texrect coords; Off|Auto|Force" },
        { "glupen64-EnableNativeResTexrects",
            "Native res. 2D texrects; False|True" },
#if defined(HAVE_OPENGLES)
        { "glupen64-EnableLegacyBlending",
            "Less accurate blending mode; True|False" },
        { "glupen64-EnableFragmentDepthWrite",
            "GPU shader depth write; False|True" },
#else
        { "glupen64-EnableLegacyBlending",
            "Less accurate blending mode; False|True" },
        { "glupen64-EnableFragmentDepthWrite",
            "GPU shader depth write; True|False" },
#endif
        { "glupen64-EnableShadersStorage",
            "Cache GPU Shaders; True|False" },
        { "glupen64-CropMode",
            "Crop Mode; Auto|Off" },
        { "glupen64-txFilterMode",
            "Texture filter; None|Smooth filtering 1|Smooth filtering 2|Smooth filtering 3|Smooth filtering 4|Sharp filtering 1|Sharp filtering 2" },
        { "glupen64-txEnhancementMode",
            "Texture Enhancement; None|As Is|X2|X2SAI|HQ2X|HQ2XS|LQ2X|LQ2XS|HQ4X|2xBRZ|3xBRZ|4xBRZ|5xBRZ|6xBRZ" },
        { "glupen64-txFilterIgnoreBG",
            "Filter background textures; True|False" },
        { "glupen64-txHiresEnable",
            "Use High-Res textures; False|True" },
        { "glupen64-txHiresFullAlphaChannel",
            "Use High-Res Full Alpha Channel; False|True" },
        {"glupen64-astick-deadzone",
           "Analog Deadzone (percent); 15|20|25|30|0|5|10"},
        {"glupen64-astick-sensitivity",
           "Analog Sensitivity (percent); 100|95|90|85|80|105|110"},
        {"glupen64-r-cbutton",
           "Right C Button; C1|C2|C3|C4"},
        {"glupen64-l-cbutton",
           "Left C Button; C2|C3|C4|C1"},
        {"glupen64-d-cbutton",
           "Down C Button; C3|C4|C1|C2"},
        {"glupen64-u-cbutton",
           "Up C Button; C4|C1|C2|C3"},
        {"glupen64-pak1",
           "Player 1 Pak; memory|rumble|none"},
        {"glupen64-pak2",
           "Player 2 Pak; none|memory|rumble"},
        {"glupen64-pak3",
           "Player 3 Pak; none|memory|rumble"},
        {"glupen64-pak4",
           "Player 4 Pak; none|memory|rumble"},
        { "glupen64-CountPerOp",
            "Count Per Op; 0|1|2|3" },
        { NULL, NULL },
    };

    static const struct retro_controller_description port[] = {
        { "Controller", RETRO_DEVICE_JOYPAD },
        { "RetroPad", RETRO_DEVICE_JOYPAD },
    };

    static const struct retro_controller_info ports[] = {
        { port, 2 },
        { port, 2 },
        { port, 2 },
        { port, 2 },
        { 0, 0 }
    };

    environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
    environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
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

    initializing = false;
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
#if defined(HAVE_OPENGLES2)
    info->library_name = "GLupeN64 GLES2";
#elif defined(HAVE_OPENGLES3)
    info->library_name = "GLupeN64 GLES3";
#else
    info->library_name = "GLupeN64 OpenGL";
#endif
#ifndef GIT_VERSION
#define GIT_VERSION " git"
#endif
    info->library_version = "2.5" GIT_VERSION;
    info->valid_extensions = "n64|v64|z64|bin|u1|ndd";
    info->need_fullpath = false;
    info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    info->geometry.base_width   = retro_screen_width;
    info->geometry.base_height  = retro_screen_height;
    info->geometry.max_width    = retro_screen_width;
    info->geometry.max_height   = retro_screen_height;
    info->geometry.aspect_ratio = retro_screen_aspect;
    info->timing.fps = vi_expected_refresh_rate_from_tv_standard(ROM_PARAMS.systemtype);
    info->timing.sample_rate = 44100.0;
}

unsigned retro_get_region (void)
{
    return ((ROM_PARAMS.systemtype == SYSTEM_PAL) ? RETRO_REGION_PAL : RETRO_REGION_NTSC);
}

void copy_file(char * ininame, char * fileName)
{
    const char* filename = ConfigGetSharedDataFilepath(fileName);
    FILE *fp = fopen(filename, "w");
    if (fp != NULL)    {
        fputs(ininame, fp);
        fclose(fp);
    }
}

void retro_init(void)
{
    uint64_t serialization_quirks = RETRO_SERIALIZATION_QUIRK_MUST_INITIALIZE;
    char* sys_pathname;
    wchar_t w_pathname[PATH_SIZE];
    environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sys_pathname);
    char pathname[PATH_SIZE];
    strncpy(pathname, sys_pathname, PATH_SIZE);
    if (pathname[(strlen(pathname)-1)] != '/' && pathname[(strlen(pathname)-1)] != '\\')
        strcat(pathname, "/");
    strcat(pathname, "GLupeN64/");
    mbstowcs(w_pathname, pathname, PATH_SIZE);
    if (!osal_path_existsW(w_pathname) || !osal_is_directory(w_pathname))
        osal_mkdirp(w_pathname);
    copy_file(inifile, "mupen64plus.ini");

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
    environ_cb(RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS, &serialization_quirks);
    initializing = true;

    retro_thread = co_active();
    game_thread = co_create(65536 * sizeof(void*) * 16, EmuThreadFunction);
}

void retro_deinit(void)
{
    CoreDoCommand(M64CMD_STOP, 0, NULL);
    deinit_audio_libretro();

    if (perf_cb.perf_log)
        perf_cb.perf_log();
}

void update_controllers()
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

void update_variables()
{
    struct retro_variable var;

    var.key = "glupen64-rspmode";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "HLE"))
            rspMode = 0;
        else
            rspMode = 1;
    }

    var.key = "glupen64-BilinearMode";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "3point"))
            bilinearMode = 0;
        else
            bilinearMode = 1;
    }

    var.key = "glupen64-MultiSampling";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        MultiSampling = atoi(var.value);
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

    var.key = "glupen64-txFilterMode";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "Smooth filtering 1"))
            txFilterMode = 1;
        else if (!strcmp(var.value, "Smooth filtering 2"))
            txFilterMode = 2;
        else if (!strcmp(var.value, "Smooth filtering 3"))
            txFilterMode = 3;
        else if (!strcmp(var.value, "Smooth filtering 4"))
            txFilterMode = 4;
        else if (!strcmp(var.value, "Sharp filtering 1"))
            txFilterMode = 5;
        else if (!strcmp(var.value, "Sharp filtering 2"))
            txFilterMode = 6;
        else
            txFilterMode = 0;
    }

    var.key = "glupen64-txEnhancementMode";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "As Is"))
            txEnhancementMode = 1;
        else if (!strcmp(var.value, "X2"))
            txEnhancementMode = 2;
        else if (!strcmp(var.value, "X2SAI"))
            txEnhancementMode = 3;
        else if (!strcmp(var.value, "HQ2X"))
            txEnhancementMode = 4;
        else if (!strcmp(var.value, "HQ2XS"))
            txEnhancementMode = 5;
        else if (!strcmp(var.value, "LQ2X"))
            txEnhancementMode = 6;
        else if (!strcmp(var.value, "LQ2XS"))
            txEnhancementMode = 7;
        else if (!strcmp(var.value, "HQ4X"))
            txEnhancementMode = 8;
        else if (!strcmp(var.value, "2xBRZ"))
            txEnhancementMode = 9;
        else if (!strcmp(var.value, "3xBRZ"))
            txEnhancementMode = 10;
        else if (!strcmp(var.value, "4xBRZ"))
            txEnhancementMode = 11;
        else if (!strcmp(var.value, "5xBRZ"))
            txEnhancementMode = 12;
        else if (!strcmp(var.value, "6xBRZ"))
            txEnhancementMode = 13;
        else
            txEnhancementMode = 0;
    }

    var.key = "glupen64-txFilterIgnoreBG";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "True"))
            txFilterIgnoreBG = 0;
        else
            txFilterIgnoreBG = 1;
    }

    var.key = "glupen64-txHiresEnable";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "True"))
            txHiresEnable = 1;
        else
            txHiresEnable = 0;
    }

    var.key = "glupen64-txHiresFullAlphaChannel";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "True"))
            txHiresFullAlphaChannel = 1;
        else
            txHiresFullAlphaChannel = 0;
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

    var.key = "glupen64-EnableFragmentDepthWrite";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "True"))
            EnableFragmentDepthWrite = 1;
        else
            EnableFragmentDepthWrite = 0;
    }

    var.key = "glupen64-EnableShadersStorage";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "True"))
            EnableShadersStorage = 1;
        else
            EnableShadersStorage = 0;
    }

    var.key = "glupen64-CropMode";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "Auto"))
            CropMode = 1;
        else
            CropMode = 0;
    }

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

    var.key = "glupen64-aspect";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "16:9 adjusted")) {
             AspectRatio = 3;
             retro_screen_aspect = 16.0 / 9.0;
        } else if (!strcmp(var.value, "16:9")) {
             AspectRatio = 2;
             retro_screen_aspect = 16.0 / 9.0;
        } else {
             AspectRatio = 1;
             retro_screen_aspect = 4.0 / 3.0;
        }
    }

    if (AspectRatio == 1)
        var.key = "glupen64-43screensize";
    else
        var.key = "glupen64-169screensize";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        sscanf(var.value, "%dx%d", &retro_screen_width, &retro_screen_height);
    }

    var.key = "glupen64-astick-deadzone";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
        astick_deadzone = (int)(atoi(var.value) * 0.01f * 0x8000);

    var.key = "glupen64-astick-sensitivity";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
        astick_sensitivity = atoi(var.value);

    var.key = "glupen64-CountPerOp";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        CountPerOp = atoi(var.value);
    }

    var.key = "glupen64-r-cbutton";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "C1"))
            r_cbutton = RETRO_DEVICE_ID_JOYPAD_A;
        else if (!strcmp(var.value, "C2"))
            r_cbutton = RETRO_DEVICE_ID_JOYPAD_Y;
        else if (!strcmp(var.value, "C3"))
            r_cbutton = RETRO_DEVICE_ID_JOYPAD_B;
        else if (!strcmp(var.value, "C4"))
            r_cbutton = RETRO_DEVICE_ID_JOYPAD_X;
    }

    var.key = "glupen64-l-cbutton";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "C1"))
            l_cbutton = RETRO_DEVICE_ID_JOYPAD_A;
        else if (!strcmp(var.value, "C2"))
            l_cbutton = RETRO_DEVICE_ID_JOYPAD_Y;
        else if (!strcmp(var.value, "C3"))
            l_cbutton = RETRO_DEVICE_ID_JOYPAD_B;
        else if (!strcmp(var.value, "C4"))
            l_cbutton = RETRO_DEVICE_ID_JOYPAD_X;
    }

    var.key = "glupen64-d-cbutton";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "C1"))
            d_cbutton = RETRO_DEVICE_ID_JOYPAD_A;
        else if (!strcmp(var.value, "C2"))
            d_cbutton = RETRO_DEVICE_ID_JOYPAD_Y;
        else if (!strcmp(var.value, "C3"))
            d_cbutton = RETRO_DEVICE_ID_JOYPAD_B;
        else if (!strcmp(var.value, "C4"))
            d_cbutton = RETRO_DEVICE_ID_JOYPAD_X;
    }

    var.key = "glupen64-u-cbutton";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "C1"))
            u_cbutton = RETRO_DEVICE_ID_JOYPAD_A;
        else if (!strcmp(var.value, "C2"))
            u_cbutton = RETRO_DEVICE_ID_JOYPAD_Y;
        else if (!strcmp(var.value, "C3"))
            u_cbutton = RETRO_DEVICE_ID_JOYPAD_B;
        else if (!strcmp(var.value, "C4"))
            u_cbutton = RETRO_DEVICE_ID_JOYPAD_X;
    }
    update_controllers();
}

static void format_saved_memory(void)
{
    format_sram(saved_memory.sram);
    format_eeprom(saved_memory.eeprom, EEPROM_MAX_SIZE);
    format_flashram(saved_memory.flashram);
    format_mempak(saved_memory.mempack + 0 * MEMPAK_SIZE);
    format_mempak(saved_memory.mempack + 1 * MEMPAK_SIZE);
    format_mempak(saved_memory.mempack + 2 * MEMPAK_SIZE);
    format_mempak(saved_memory.mempack + 3 * MEMPAK_SIZE);
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
    if (!game)
        return false;

    glsm_ctx_params_t params = {0};
    format_saved_memory();

    update_variables();
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
    CoreDoCommand(M64CMD_ROM_CLOSE, 0, NULL);
    emu_initialized = false;
}

void retro_run (void)
{
    static bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        update_controllers();
    glsm_ctl(GLSM_CTL_STATE_BIND, NULL);
    co_switch(game_thread);
    glsm_ctl(GLSM_CTL_STATE_UNBIND, NULL);
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
    return 16788288 + 1024 + 4; // < 16MB and some change... ouch
}

bool retro_serialize(void *data, size_t size)
{
    if (initializing)
        return false;

    const char* filename = ConfigGetSharedDataFilepath("savestate.temp");
    int success = savestates_save_m64p((char*)filename);
    FILE *read_ptr;
    read_ptr = fopen(filename, "rb");
    int read_size = fread(data, size, 1, read_ptr);
    fclose(read_ptr);
    remove(filename);
    if (success)
        return true;

    return false;
}

bool retro_unserialize(const void * data, size_t size)
{
    if (initializing)
        return false;

    FILE *write_ptr;
    const char* filename = ConfigGetSharedDataFilepath("savestate.temp");
    write_ptr = fopen(filename,"wb");
    fwrite(data, size, 1, write_ptr);
    fclose(write_ptr);
    int success = savestates_load_m64p((char*)filename);
    remove(filename);
    if (success)
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

void retro_cheat_reset(void)
{
    cheat_delete_all();
}

void retro_cheat_set(unsigned index, bool enabled, const char* codeLine)
{
    char name[256];
    m64p_cheat_code mupenCode[256];
    int matchLength=0,partCount=0;
    uint32_t codeParts[256];
    int cursor;

    //Generate a name
    sprintf(name, "cheat_%u",index);

    //Break the code into Parts
    for (cursor=0;;cursor++)
    {
        if (ISHEXDEC){
            matchLength++;
        } else {
            if (matchLength){
                char codePartS[matchLength];
                strncpy(codePartS,codeLine+cursor-matchLength,matchLength);
                codePartS[matchLength]=0;
                codeParts[partCount++]=strtoul(codePartS,NULL,16);
                matchLength=0;
            }
        }
        if (!codeLine[cursor]){
            break;
        }
    }

    //Assign the parts to mupenCode
    for (cursor=0;2*cursor+1<partCount;cursor++){
        mupenCode[cursor].address=codeParts[2*cursor];
        mupenCode[cursor].value=codeParts[2*cursor+1];
    }

    //Assign to mupenCode
    cheat_add_new(name,mupenCode,partCount/2);
    cheat_set_enabled(name,enabled);
}

void retro_return(void)
{
    co_switch(retro_thread);
}

uint32_t get_retro_screen_width()
{
    return retro_screen_width;
}

uint32_t get_retro_screen_height()
{
    return retro_screen_height;
}

static int GamesharkActive = 0;

int event_gameshark_active(void)
{
    return GamesharkActive;
}

void event_set_gameshark(int active)
{
    // if boolean value doesn't change then just return
    if (!active == !GamesharkActive)
        return;

    // set the button state
    GamesharkActive = (active ? 1 : 0);

    // notify front-end application that gameshark button state has changed
    StateChanged(M64CORE_INPUT_GAMESHARK, GamesharkActive);
}
