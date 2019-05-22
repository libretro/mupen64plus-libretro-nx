#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libretro.h"
#include "libretro_private.h"
#include "GLideN64_libretro.h"

#include <libco.h>

#ifdef HAVE_LIBNX
#include <switch.h>
#endif

#include <glsm/glsmsym.h>

#include "api/m64p_frontend.h"
#include "plugin/plugin.h"
#include "api/m64p_types.h"
#include "device/r4300/r4300_core.h"
#include "device/memory/memory.h"
#include "main/main.h"
#include "api/callbacks.h"
#include "main/cheat.h"
#include "main/version.h"
#include "main/savestates.h"
#include "main/mupen64plus.ini.h"
#include "api/m64p_config.h"
#include "osal_files.h"
#include "main/rom.h"
#include "device/rcp/pi/pi_controller.h"
#include "device/pif/pif.h"
#include "libretro_memory.h"

#include "audio_plugin.h"

#ifndef CORE_NAME
#define CORE_NAME "mupen64plus"
#endif

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
bool alternate_mapping;

static uint8_t* game_data = NULL;
static uint32_t game_size = 0;

static bool     emu_initialized     = false;
static unsigned initial_boot        = true;
static unsigned audio_buffer_size   = 2048;

static unsigned retro_filtering     = 0;
static bool     first_context_reset = false;
static bool     initializing        = true;

bool libretro_swap_buffer;

uint32_t retro_screen_width = 320;
uint32_t retro_screen_height = 240;
float retro_screen_aspect = 4.0 / 3.0;

uint32_t bilinearMode = 0;
uint32_t EnableHWLighting = 0;
uint32_t CorrectTexrectCoords = 0;
uint32_t enableNativeResTexrects = 0;
uint32_t enableLegacyBlending = 0;
uint32_t EnableCopyColorToRDRAM = 0;
uint32_t EnableCopyDepthToRDRAM = 0;
uint32_t AspectRatio = 0;
uint32_t MaxTxCacheSize = 0;
uint32_t txFilterMode = 0;
uint32_t txEnhancementMode = 0;
uint32_t txHiresEnable = 0;
uint32_t txHiresFullAlphaChannel = 0;
uint32_t txFilterIgnoreBG = 0;
uint32_t EnableFXAA = 0;
uint32_t MultiSampling = 0;
uint32_t EnableFragmentDepthWrite = 0;
uint32_t EnableShadersStorage = 0;
uint32_t EnableTextureCache = 0;
uint32_t EnableFBEmulation = 0;
uint32_t EnableFrameDuping = 0;
uint32_t EnableNoiseEmulation = 0;
uint32_t EnableLODEmulation = 0;
uint32_t EnableFullspeed = 0;
uint32_t CountPerOp = 0;
uint32_t CountPerScanlineOverride = 0;

// Overscan options
#define GLN64_OVERSCAN_SCALING "0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|24|25|26|27|28|29|30|31|32|33|34|35|36|37|38|39|40|41|42|43|44|45|46|47|48|49|50"
uint32_t EnableOverscan = 0;
uint32_t OverscanTop = 0;
uint32_t OverscanLeft = 0;
uint32_t OverscanRight = 0;
uint32_t OverscanBottom = 0;

int rspMode = 0;

extern struct device g_dev;
extern unsigned int emumode;
extern struct cheat_ctx g_cheat_ctx;

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
    snprintf(buffer, 1024, CORE_NAME ": %s\n", aMessage);
    if (log_cb)
        log_cb(RETRO_LOG_INFO, buffer);
}

extern m64p_rom_header ROM_HEADER;

static void setup_variables(void)
{
    struct retro_variable variables[] = {
        { CORE_NAME "-cpucore",
#ifdef DYNAREC
            "CPU Core; dynamic_recompiler|cached_interpreter|pure_interpreter" },
#else
            "CPU Core; cached_interpreter|pure_interpreter" },
#endif
        { CORE_NAME "-rspmode",
            "RSP Mode; HLE" },
        { CORE_NAME "-43screensize",
            "4:3 Resolution; 320x240|640x480|960x720|1280x960|1440x1080|1600x1200|1920x1440|2240x1680|2560x1920|2880x2160|3200x2400|3520x2640|3840x2880" },
        { CORE_NAME "-169screensize",
            "16:9 Resolution; 640x360|960x540|1280x720|1920x1080|2560x1440|3840x2160|4096x2160|7680x4320" },
        { CORE_NAME "-aspect",
            "Aspect Ratio; 4:3|16:9|16:9 adjusted" },
        { CORE_NAME "-BilinearMode",
            "Bilinear filtering mode; standard|3point" },
#ifndef HAVE_OPENGLES2
        { CORE_NAME "-MultiSampling",
            "MSAA level; 0|2|4|8|16" },
#endif
        { CORE_NAME "-FXAA",
            "FXAA; 0|1" },
        { CORE_NAME "-FrameDuping",
#ifdef HAVE_LIBNX
            "Frame Duplication; True|False" },
#else
            "Frame Duplication; False|True" },
#endif
        { CORE_NAME "-Framerate",
            "Framerate; Original|Fullspeed" },
        { CORE_NAME "-virefresh",
            "VI Refresh (Overclock); Auto|1500|2200" },
        { CORE_NAME "-NoiseEmulation",
            "Noise Emulation; True|False" },

        { CORE_NAME "-EnableFBEmulation",
#ifdef VC
            "Framebuffer Emulation; False|True" },
#else
            "Framebuffer Emulation; True|False" },
#endif

        { CORE_NAME "-EnableLODEmulation",
            "LOD Emulation; True|False" },
        { CORE_NAME "-EnableCopyColorToRDRAM",
#ifndef HAVE_OPENGLES
            "Color buffer to RDRAM; Async|Sync|Off" },
#else
            "Color buffer to RDRAM; Off|Async|Sync" },
#endif
        { CORE_NAME "-EnableCopyDepthToRDRAM",
            "Depth buffer to RDRAM; Software|FromMem|Off" },
        { CORE_NAME "-EnableHWLighting",
            "Hardware per-pixel lighting; False|True" },
        { CORE_NAME "-CorrectTexrectCoords",
            "Continuous texrect coords; Off|Auto|Force" },
        { CORE_NAME "-EnableNativeResTexrects",
            "Native res. 2D texrects; False|True" },
#if defined(HAVE_OPENGLES)
        { CORE_NAME "-EnableLegacyBlending",
            "Less accurate blending mode; True|False" },
        { CORE_NAME "-EnableFragmentDepthWrite",
            "GPU shader depth write; False|True" },
#else
        { CORE_NAME "-EnableLegacyBlending",
            "Less accurate blending mode; False|True" },
        { CORE_NAME "-EnableFragmentDepthWrite",
            "GPU shader depth write; True|False" },
#endif
#ifndef VC
        { CORE_NAME "-EnableShadersStorage",
            "Cache GPU Shaders; True|False" },
#endif
        { CORE_NAME "-EnableTextureCache",
            "Cache Textures; True|False" },
        { CORE_NAME "-EnableOverscan",
            "Overscan; Enabled|Disabled" },
        { CORE_NAME "-OverscanTop",
            "Overscan Offset (Top); " GLN64_OVERSCAN_SCALING },
        { CORE_NAME "-OverscanLeft",
            "Overscan Offset (Left); " GLN64_OVERSCAN_SCALING },
        { CORE_NAME "-OverscanRight",
            "Overscan Offset (Right); " GLN64_OVERSCAN_SCALING },
        { CORE_NAME "-OverscanBottom",
            "Overscan Offset (Bottom); " GLN64_OVERSCAN_SCALING },

        { CORE_NAME "-MaxTxCacheSize",
#if defined(VC)
            "Max texture cache size; 1500|8000|4000" },
#elif defined(HAVE_LIBNX)
            "Max texture cache size; 4000|1500|8000" },
#else
            "Max texture cache size; 8000|4000|1500" },
#endif
        { CORE_NAME "-txFilterMode",
            "Texture filter; None|Smooth filtering 1|Smooth filtering 2|Smooth filtering 3|Smooth filtering 4|Sharp filtering 1|Sharp filtering 2" },
        { CORE_NAME "-txEnhancementMode",
            "Texture Enhancement; None|As Is|X2|X2SAI|HQ2X|HQ2XS|LQ2X|LQ2XS|HQ4X|2xBRZ|3xBRZ|4xBRZ|5xBRZ|6xBRZ" },
        { CORE_NAME "-txFilterIgnoreBG",
            "Filter background textures; True|False" },
        { CORE_NAME "-txHiresEnable",
            "Use High-Res textures; False|True" },
        { CORE_NAME "-txHiresFullAlphaChannel",
            "Use High-Res Full Alpha Channel; False|True" },
        { CORE_NAME "-astick-deadzone",
           "Analog Deadzone (percent); 15|20|25|30|0|5|10"},
        { CORE_NAME "-astick-sensitivity",
           "Analog Sensitivity (percent); 100|105|110|115|120|125|130|135|140|145|150|50|55|60|65|70|75|80|85|90|95"},
        { CORE_NAME "-r-cbutton",
           "Right C Button; C1|C2|C3|C4"},
        { CORE_NAME "-l-cbutton",
           "Left C Button; C2|C3|C4|C1"},
        { CORE_NAME "-d-cbutton",
           "Down C Button; C3|C4|C1|C2"},
        { CORE_NAME "-u-cbutton",
           "Up C Button; C4|C1|C2|C3"},
        { CORE_NAME "-alt-map",
           "Independent C-button Controls; False|True" },
        { CORE_NAME "-pak1",
           "Player 1 Pak; memory|rumble|none"},
        { CORE_NAME "-pak2",
           "Player 2 Pak; none|memory|rumble"},
        { CORE_NAME "-pak3",
           "Player 3 Pak; none|memory|rumble"},
        { CORE_NAME "-pak4",
           "Player 4 Pak; none|memory|rumble"},
        { CORE_NAME "-CountPerOp",
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
    m64p_error ret = CoreStartup(FRONTEND_API_VERSION, ".", ".", "Core", n64DebugCallback, 0, 0);
    if(ret && log_cb)
        log_cb(RETRO_LOG_ERROR, CORE_NAME ": failed to initialize core (err=%i)\n", ret);

    log_cb(RETRO_LOG_DEBUG, CORE_NAME ": [EmuThread] M64CMD_ROM_OPEN\n");

    if(CoreDoCommand(M64CMD_ROM_OPEN, game_size, (void*)game_data))
    {
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, CORE_NAME ": failed to load ROM\n");
        goto load_fail;
    }

    free(game_data);
    game_data = NULL;

    log_cb(RETRO_LOG_DEBUG, CORE_NAME ": [EmuThread] M64CMD_ROM_GET_HEADER\n");

    if(CoreDoCommand(M64CMD_ROM_GET_HEADER, sizeof(ROM_HEADER), &ROM_HEADER))
    {
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, CORE_NAME ": failed to query ROM header information\n");
        goto load_fail;
    }

    return true;

load_fail:
    free(game_data);
    game_data = NULL;
    //stop = 1;

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
    log_cb(RETRO_LOG_DEBUG, CORE_NAME ": [EmuThread] M64CMD_EXECUTE\n");

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
    info->library_name = "Mupen64Plus-Next GLES2";
#elif defined(HAVE_OPENGLES3)
    info->library_name = "Mupen64Plus-Next GLES3";
#else
    info->library_name = "Mupen64Plus-Next OpenGL";
#endif
#ifndef GIT_VERSION
#define GIT_VERSION " git"
#endif
    info->library_version = "1.0" GIT_VERSION;
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
    char* sys_pathname;
    wchar_t w_pathname[PATH_SIZE];
    environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sys_pathname);
    char pathname[PATH_SIZE];
    strncpy(pathname, sys_pathname, PATH_SIZE);
    if (pathname[(strlen(pathname)-1)] != '/' && pathname[(strlen(pathname)-1)] != '\\')
        strcat(pathname, "/");
    strcat(pathname, "Mupen64plus/");
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
    initializing = true;

    retro_thread = co_active();
    game_thread = co_create(65536 * sizeof(void*) * 16, EmuThreadFunction);
}

void retro_deinit(void)
{
    CoreDoCommand(M64CMD_STOP, 0, NULL);
    co_switch(game_thread); /* Let the core thread finish */
    deinit_audio_libretro();

    if (perf_cb.perf_log)
        perf_cb.perf_log();
}

void update_controllers()
{
    struct retro_variable pk1var = { CORE_NAME "-pak1" };
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

    struct retro_variable pk2var = { CORE_NAME "-pak2" };
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

    struct retro_variable pk3var = { CORE_NAME "-pak3" };
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

    struct retro_variable pk4var = { CORE_NAME "-pak4" };
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

    var.key = CORE_NAME "-rspmode";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        rspMode = !strcmp(var.value, "HLE") ? 0 : 1;
    }

    var.key = CORE_NAME "-BilinearMode";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        bilinearMode = !strcmp(var.value, "3point") ? 0 : 1;
    }

    var.key = CORE_NAME "-FXAA";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableFXAA = atoi(var.value);
    }

    var.key = CORE_NAME "-MultiSampling";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        MultiSampling = atoi(var.value);
    }

    var.key = CORE_NAME "-FrameDuping";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableFrameDuping = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-Framerate";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableFullspeed = !strcmp(var.value, "Original") ? 0 : 1;
    }

    var.key = CORE_NAME "-virefresh";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        CountPerScanlineOverride = !strcmp(var.value, "Auto") ? 0 : atoi(var.value);
    }

    var.key = CORE_NAME "-NoiseEmulation";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableNoiseEmulation = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-EnableLODEmulation";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableLODEmulation = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-EnableFBEmulation";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableFBEmulation = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-EnableCopyColorToRDRAM";
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

    var.key = CORE_NAME "-EnableCopyDepthToRDRAM";
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

    var.key = CORE_NAME "-EnableHWLighting";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableHWLighting = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-CorrectTexrectCoords";
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

    var.key = CORE_NAME "-EnableNativeResTexrects";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        enableNativeResTexrects = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-txFilterMode";
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

    var.key = CORE_NAME "-txEnhancementMode";
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

    var.key = CORE_NAME "-txFilterIgnoreBG";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        // "Filter background textures; True|False" (true=filter, false=ignore)
        txFilterIgnoreBG = !strcmp(var.value, "False") ? 1 : 0;
    }

    var.key = CORE_NAME "-txHiresEnable";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        txHiresEnable = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-txHiresFullAlphaChannel";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        txHiresFullAlphaChannel = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-MaxTxCacheSize";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        MaxTxCacheSize = atoi(var.value);
    }

    var.key = CORE_NAME "-EnableLegacyBlending";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        enableLegacyBlending = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-EnableFragmentDepthWrite";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableFragmentDepthWrite = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-EnableShadersStorage";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableShadersStorage = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-EnableTextureCache";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableTextureCache = !strcmp(var.value, "False") ? 0 : 1;
    }

    var.key = CORE_NAME "-cpucore";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "pure_interpreter"))
             emumode = EMUMODE_PURE_INTERPRETER;
        else if (!strcmp(var.value, "cached_interpreter"))
             emumode = EMUMODE_INTERPRETER;
        else if (!strcmp(var.value, "dynamic_recompiler"))
             emumode = EMUMODE_DYNAREC;
    }

    var.key = CORE_NAME "-aspect";
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

    var.key = (AspectRatio == 1 ? CORE_NAME "-43screensize" : CORE_NAME "-169screensize");
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        sscanf(var.value, "%dx%d", &retro_screen_width, &retro_screen_height);
    }

    var.key = CORE_NAME "-astick-deadzone";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
        astick_deadzone = (int)(atoi(var.value) * 0.01f * 0x8000);

    var.key = CORE_NAME "-astick-sensitivity";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
        astick_sensitivity = atoi(var.value);

    var.key = CORE_NAME "-CountPerOp";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        CountPerOp = atoi(var.value);
    }
    if(EnableFullspeed)
    {
        CountPerOp = 1; // Force CountPerOp == 1
        if(!EnableFBEmulation)
            EnableFrameDuping = 1;
    }

    var.key = CORE_NAME "-r-cbutton";
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

    var.key = CORE_NAME "-l-cbutton";
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

    var.key = CORE_NAME "-d-cbutton";
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

    var.key = CORE_NAME "-u-cbutton";
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
    
    var.key = CORE_NAME "-EnableOverscan";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        EnableOverscan = !strcmp(var.value, "Enabled") ? 1 : 0;
    }

    var.key = CORE_NAME "-OverscanTop";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        OverscanTop = atoi(var.value);
    }

    var.key = CORE_NAME "-OverscanLeft";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        OverscanLeft = atoi(var.value);
    }

    var.key = CORE_NAME "-OverscanRight";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        OverscanRight = atoi(var.value);
    }

    var.key = CORE_NAME "-OverscanBottom";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        OverscanBottom = atoi(var.value);
    }

    var.key = CORE_NAME "-alt-map";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        alternate_mapping = !strcmp(var.value, "False") ? 0 : 1;
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
    log_cb(RETRO_LOG_DEBUG, CORE_NAME ": context_reset()\n");
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
    //if (!stop)
     //   return false;
    return true;
}

bool retro_load_game(const struct retro_game_info *game)
{
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
            log_cb(RETRO_LOG_ERROR, CORE_NAME ": libretro frontend doesn't have OpenGL support\n");
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
    libretro_swap_buffer = false;
    static bool updated = false;
    
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
        update_controllers();
    }

    glsm_ctl(GLSM_CTL_STATE_BIND, NULL);
    co_switch(game_thread);
    glsm_ctl(GLSM_CTL_STATE_UNBIND, NULL);
    if (libretro_swap_buffer)
        video_cb(RETRO_HW_FRAME_BUFFER_VALID, retro_screen_width, retro_screen_height, 0);
    else if(EnableFrameDuping)
        video_cb(NULL, retro_screen_width, retro_screen_height, 0);
}

void retro_reset (void)
{
    CoreDoCommand(M64CMD_RESET, 0, (void*)0);
}

void *retro_get_memory_data(unsigned type)
{
    switch (type)
    {
        case RETRO_MEMORY_SYSTEM_RAM: return g_dev.rdram.dram;
        case RETRO_MEMORY_SAVE_RAM:   return &saved_memory;
    }
    return NULL;
}

size_t retro_get_memory_size(unsigned type)
{
    switch (type)
    {
        case RETRO_MEMORY_SYSTEM_RAM: return RDRAM_MAX_SIZE;
        case RETRO_MEMORY_SAVE_RAM:   return sizeof(saved_memory);
    }
    return 0;
}

size_t retro_serialize_size (void)
{
    return 16788288 + 1024 + 4 + 4096;
}

bool retro_serialize(void *data, size_t size)
{
    if (initializing)
        return false;

    int success = savestates_save_m64p(&g_dev, data);
    if (success)
        return true;

    return false;
}

bool retro_unserialize(const void * data, size_t size)
{
    if (initializing)
        return false;

    int success = savestates_load_m64p(&g_dev, data);
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
    cheat_delete_all(&g_cheat_ctx);
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
    cheat_add_new(&g_cheat_ctx, name, mupenCode, partCount / 2);
    cheat_set_enabled(&g_cheat_ctx, name, enabled);
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
