/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - libretro_core_options.h                            *
 *   Copyright (C) 2020 M4xw <m4x@m4xw.net>                                *
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

#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CORE_NAME
#define CORE_NAME "mupen64plus"
#endif

// Option entries
#define OPTION_ENTRY_RDP_GLIDEN64 "gliden64"
#define OPTION_ENTRY_RSP_HLE "hle"

#ifdef HAVE_THR_AL
#define OPTION_ENTRY_RDP_ANGRYLION "|angrylion"
#else
#define OPTION_ENTRY_RDP_ANGRYLION ""
#endif // HAVE_THR_AL

#ifdef HAVE_PARALLEL_RSP
#define OPTION_ENTRY_RSP_PARALLEL "|parallel"
#else
#define OPTION_ENTRY_RSP_PARALLEL ""
#endif // HAVE_PARALLEL_RSP

#ifdef HAVE_LLE
#define OPTION_ENTRY_RSP_CXD4 "|cxd4"
#else
#define OPTION_ENTRY_RSP_CXD4 ""
#endif // HAVE_LLE

struct retro_core_option_definition option_defs_us[] = {
    {
        CORE_NAME "-cpucore",
        "CPU Core",
        "Select the R4300 CPU Backend, use Interpreter for best compability",
        {
            {"dynamic_recompiler", "Dynarec"},
            {"cached_interpreter", "IR Interpreter"},
            {"pure_interpreter", "Pure Interpreter"},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-rdp-plugin",
        "RDP Plugin",
        "Select a RDP Plugin, use Angrylion (if available) for best compability, GLideN64 for Performance",
        {
            {"gliden64", "GLideN64"},
            {"angrylion", "Angrylion"},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-rsp-plugin",
        "RSP Plugin",
        "Select a RSP Plugin, use HLE for best performance, paraLLEl for best LLE Performance and CXD4 as LLE fallback",
        {
            {"hle", NULL},
            {"parallel", NULL},
            {"cxd4", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-43screensize",
        "4:3 Resolution",
        "(GLN64) Select Render Viewport dimensions (4:3)",
        {
            {"640x480", NULL},
            {"320x240", NULL},
            {"960x720", NULL},
            {"1280x960", NULL},
            {"1440x1080", NULL},
            {"1600x1200", NULL},
            {"1920x1440", NULL},
            {"2240x1680", NULL},
            {"2560x1920", NULL},
            {"2880x2160", NULL},
            {"3200x2400", NULL},
            {"3520x2640", NULL},
            {"3840x2880", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-169screensize",
        "16:9 Resolution",
        "(GLN64) Select Render Viewport dimensions (16:9)",
        {
            {"960x540", NULL},
            {"640x360", NULL},
            {"1280x720", NULL},
            {"1920x1080", NULL},
            {"2560x1440", NULL},
            {"3840x2160", NULL},
            {"4096x2160", NULL},
            {"7680x4320", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-aspect",
        "Aspect Ratio",
        "(GLN64) Select a aspect ratio, 16:9 adjusted means essentially Widescreen hacks",
        {
            {"4:3", NULL},
            {"16:9", NULL},
            {"16:9 adjusted", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-BilinearMode",
        "Bilinear filtering mode",
        "(GLN64) Select a Bilinear Filter",
        {
            {"standard", NULL},
            {"3point", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-MultiSampling",
        "MSAA level",
        "(GLN64) Enable/Disable MultiSampling (0 = disabled)",
        {
            {"0", NULL},
            {"2", NULL},
            {"4", NULL},
            {"8", NULL},
            {"16", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-FXAA",
        "FXAA",
        "(GLN64) Enable/Disable Fast Approximate Anti-Aliasing FXAA (0 = disabled)",
        {
            {"0", NULL},
            {"1", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-NoiseEmulation",
        "Noise Emulation",
        "(GLN64) Enable Color Noise Emulation (example: SM64 Teleport)",
        {
            {"True", NULL},
            {"False", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableFBEmulation",
        "Framebuffer Emulation",
        "(GLN64) Enable frame and|or depth buffer emulation. Should almost always be enabled",
        {
            {"True", NULL},
            {"False", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableLODEmulation",
        "LOD Emulation",
        "(GLN64) Enable or Disable LOD Emulation",
        {
            {"True", NULL},
            {"False", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableCopyColorToRDRAM",
        "Color buffer to RDRAM",
        "(GLN64) Enable color buffer copy to RDRAM (Off will trade compatibility for Performance)",
        {
            {"Async", NULL},
            {"Sync", NULL},
            {"Off", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableCopyDepthToRDRAM",
        "Depth buffer to RDRAM",
        "(GLN64) Enable depth buffer copy to RDRAM (Off will trade compatibility for Performance)",
        {
            {"Software", NULL},
            {"FromMem", NULL},
            {"Off", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-BackgroundMode",
        "Background Mode",
        "(GLN64)  Render backgrounds mode (HLE only). One piece (fast), Stripped (precise)",
        {
            {"OnePiece", NULL},
            {"Stripped", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableHWLighting",
        "Hardware per-pixel lighting",
        "(GLN64) Enable hardware per-pixel lighting",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-CorrectTexrectCoords",
        "Continuous texrect coords",
        "(GLN64) Make texrect coordinates continuous to avoid black lines between them",
        {
            {"Off", NULL},
            {"Auto", NULL},
            {"Force", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableNativeResTexrects",
        "Native res. 2D texrects",
        "(GLN64) Render 2D texrects in native resolution to fix misalignment between parts of 2D image",
        {
            {"Disabled", NULL},
            {"Optimized", NULL},
            {"Unoptimized", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableLegacyBlending",
        "Less accurate blending mode",
        "(GLN64) Do not use shaders to emulate N64 blending modes. Works faster on slow GPU. Can cause glitches.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableFragmentDepthWrite",
        "GPU shader depth write",
        "(GLN64) Enable writing of fragment depth. Some mobile GPUs do not support it, thus it's optional. Leave enabled.",
        {
            {"True", NULL},
            {"False", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableN64DepthCompare",
        "N64 Depth Compare",
        "(GLN64) Enable N64 depth compare instead of OpenGL standard one. Experimental",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableShadersStorage",
        "Cache GPU Shaders",
        "(GLN64) Use persistent storage for compiled shaders",
        {
            {"True", NULL},
            {"False", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableTextureCache",
        "Cache Textures",
        "(GLN64) Save texture cache to hard disk",
        {
            {"True", NULL},
            {"False", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableOverscan",
        "Overscan",
        "(GLN64) Enable resulted image crop by Oversca",
        {
            {"Enabled", NULL},
            {"Disabled", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-OverscanTop",
        "Overscan Offset (Top)",
        "(GLN64) Overscan Top Offset",
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            {"6", NULL},
            {"7", NULL},
            {"8", NULL},
            {"9", NULL},
            {"10", NULL},
            {"11", NULL},
            {"12", NULL},
            {"13", NULL},
            {"14", NULL},
            {"15", NULL},
            {"16", NULL},
            {"17", NULL},
            {"18", NULL},
            {"19", NULL},
            {"20", NULL},
            {"21", NULL},
            {"22", NULL},
            {"23", NULL},
            {"24", NULL},
            {"25", NULL},
            {"26", NULL},
            {"27", NULL},
            {"28", NULL},
            {"29", NULL},
            {"30", NULL},
            {"31", NULL},
            {"32", NULL},
            {"33", NULL},
            {"34", NULL},
            {"35", NULL},
            {"36", NULL},
            {"37", NULL},
            {"38", NULL},
            {"39", NULL},
            {"40", NULL},
            {"41", NULL},
            {"42", NULL},
            {"43", NULL},
            {"44", NULL},
            {"45", NULL},
            {"46", NULL},
            {"47", NULL},
            {"48", NULL},
            {"49", NULL},
            {"50", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-OverscanLeft",
        "Overscan Offset (Left)",
        "(GLN64) Overscan Left Offset",
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            {"6", NULL},
            {"7", NULL},
            {"8", NULL},
            {"9", NULL},
            {"10", NULL},
            {"11", NULL},
            {"12", NULL},
            {"13", NULL},
            {"14", NULL},
            {"15", NULL},
            {"16", NULL},
            {"17", NULL},
            {"18", NULL},
            {"19", NULL},
            {"20", NULL},
            {"21", NULL},
            {"22", NULL},
            {"23", NULL},
            {"24", NULL},
            {"25", NULL},
            {"26", NULL},
            {"27", NULL},
            {"28", NULL},
            {"29", NULL},
            {"30", NULL},
            {"31", NULL},
            {"32", NULL},
            {"33", NULL},
            {"34", NULL},
            {"35", NULL},
            {"36", NULL},
            {"37", NULL},
            {"38", NULL},
            {"39", NULL},
            {"40", NULL},
            {"41", NULL},
            {"42", NULL},
            {"43", NULL},
            {"44", NULL},
            {"45", NULL},
            {"46", NULL},
            {"47", NULL},
            {"48", NULL},
            {"49", NULL},
            {"50", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-OverscanRight",
        "Overscan Offset (Right)",
        "(GLN64) Overscan Right Offset",
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            {"6", NULL},
            {"7", NULL},
            {"8", NULL},
            {"9", NULL},
            {"10", NULL},
            {"11", NULL},
            {"12", NULL},
            {"13", NULL},
            {"14", NULL},
            {"15", NULL},
            {"16", NULL},
            {"17", NULL},
            {"18", NULL},
            {"19", NULL},
            {"20", NULL},
            {"21", NULL},
            {"22", NULL},
            {"23", NULL},
            {"24", NULL},
            {"25", NULL},
            {"26", NULL},
            {"27", NULL},
            {"28", NULL},
            {"29", NULL},
            {"30", NULL},
            {"31", NULL},
            {"32", NULL},
            {"33", NULL},
            {"34", NULL},
            {"35", NULL},
            {"36", NULL},
            {"37", NULL},
            {"38", NULL},
            {"39", NULL},
            {"40", NULL},
            {"41", NULL},
            {"42", NULL},
            {"43", NULL},
            {"44", NULL},
            {"45", NULL},
            {"46", NULL},
            {"47", NULL},
            {"48", NULL},
            {"49", NULL},
            {"50", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-OverscanBottom",
        "Overscan Offset (Bottom)",
        "(GLN64) Overscan Bottom Offset",
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            {"6", NULL},
            {"7", NULL},
            {"8", NULL},
            {"9", NULL},
            {"10", NULL},
            {"11", NULL},
            {"12", NULL},
            {"13", NULL},
            {"14", NULL},
            {"15", NULL},
            {"16", NULL},
            {"17", NULL},
            {"18", NULL},
            {"19", NULL},
            {"20", NULL},
            {"21", NULL},
            {"22", NULL},
            {"23", NULL},
            {"24", NULL},
            {"25", NULL},
            {"26", NULL},
            {"27", NULL},
            {"28", NULL},
            {"29", NULL},
            {"30", NULL},
            {"31", NULL},
            {"32", NULL},
            {"33", NULL},
            {"34", NULL},
            {"35", NULL},
            {"36", NULL},
            {"37", NULL},
            {"38", NULL},
            {"39", NULL},
            {"40", NULL},
            {"41", NULL},
            {"42", NULL},
            {"43", NULL},
            {"44", NULL},
            {"45", NULL},
            {"46", NULL},
            {"47", NULL},
            {"48", NULL},
            {"49", NULL},
            {"50", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-MaxTxCacheSize",
        "Max texture cache size",
        "(GLN64) Set Max texture cache size (in elements). Prevents instability if black squared pop in",
        {
            {"8000", NULL},
            {"4000", NULL},
            {"1500", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-txFilterMode",
        "Texture filter",
        "(GLN64) Select Texture Filtering mode",
        {
            {"None", NULL},
            {"Smooth filtering 1", NULL},
            {"Smooth filtering 2", NULL},
            {"Smooth filtering 3", NULL},
            {"Smooth filtering 4", NULL},
            {"Sharp filtering 1", NULL},
            {"Sharp filtering 2", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-txEnhancementMode",
        "Texture Enhancement",
        "(GLN64) Enable Texture enhancements (As-Is will just cache)",
        {
            {"None", NULL},
            {"As Is", NULL},
            {"X2", NULL},
            {"X2SAI", NULL},
            {"HQ2X", NULL},
            {"HQ2XS", NULL},
            {"LQ2X", NULL},
            {"LQ2XS", NULL},
            {"HQ4X", NULL},
            {"2xBRZ", NULL},
            {"3xBRZ", NULL},
            {"4xBRZ", NULL},
            {"5xBRZ", NULL},
            {"6xBRZ", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-txFilterIgnoreBG",
        "Don't filter background textures",
        "(GLN64) Ignore filtering for Background Textures",
        {
            {"True", NULL},
            {"False", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-txHiresEnable",
        "Use High-Res textures",
        "(GLN64) Enable High-Res Texture packs",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-txCacheCompression",
        "Use High-Res Texture Cache Compression",
        "(GLN64) Compress created texture caches",
        {
            {"True", NULL},
            {"False", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-txHiresFullAlphaChannel",
        "Use High-Res Full Alpha Channel",
        "(GLN64) This should be enabled unless it's a old RICE Texture pack",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableEnhancedTextureStorage",
        "Use enhanced Texture Storage",
        "(GLN64) Use in addition to Texture cache, will use lazy loading and trade memory consumption against loading speeds",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-EnableEnhancedHighResStorage",
        "Use enhanced Hi-Res Storage",
        "(GLN64) Use in addition to High-Res textures, will use lazy loading and trade memory consumption against loading speeds",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-angrylion-vioverlay",
        "VI Overlay",
        "(AL) Select VI Overlay filtering",
        {
            {"Filtered", NULL},
            {"AA+Blur", NULL},
            {"AA+Dedither", NULL},
            {"AA only", NULL},
            {"Unfiltered", NULL},
            {"Depth", NULL},
            {"Coverage", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-angrylion-sync",
        "Thread sync level",
        "(AL) Select Sync level (trades accuracy for performance)",
        {
            {"Low", NULL},
            {"Medium", NULL},
            {"High", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-angrylion-multithread",
        "Multi-threading",
        "(AL) Enable multithread, it's prefered to have it match your Physical CPU Core count",
        {
            {"all threads", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            {"6", NULL},
            {"7", NULL},
            {"8", NULL},
            {"9", NULL},
            {"10", NULL},
            {"11", NULL},
            {"12", NULL},
            {"13", NULL},
            {"14", NULL},
            {"15", NULL},
            {"16", NULL},
            {"17", NULL},
            {"18", NULL},
            {"19", NULL},
            {"20", NULL},
            {"21", NULL},
            {"22", NULL},
            {"23", NULL},
            {"24", NULL},
            {"25", NULL},
            {"26", NULL},
            {"27", NULL},
            {"28", NULL},
            {"29", NULL},
            {"30", NULL},
            {"31", NULL},
            {"32", NULL},
            {"33", NULL},
            {"34", NULL},
            {"35", NULL},
            {"36", NULL},
            {"37", NULL},
            {"38", NULL},
            {"39", NULL},
            {"40", NULL},
            {"41", NULL},
            {"42", NULL},
            {"43", NULL},
            {"44", NULL},
            {"45", NULL},
            {"46", NULL},
            {"47", NULL},
            {"48", NULL},
            {"49", NULL},
            {"50", NULL},
            {"51", NULL},
            {"52", NULL},
            {"53", NULL},
            {"54", NULL},
            {"55", NULL},
            {"56", NULL},
            {"57", NULL},
            {"58", NULL},
            {"59", NULL},
            {"60", NULL},
            {"61", NULL},
            {"62", NULL},
            {"63", NULL},
            {"75", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-angrylion-overscan",
        "Hide overscan",
        "(AL) Hide overscan borders",
        {
            {"disabled", NULL},
            {"enabled", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-FrameDuping",
        "Frame Duplication",
        "Enable Frame duplication to improve smoothing on low-end. It's not frameskip",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-Framerate",
        "Framerate",
        "Fullspeed will enforce Count per Op 1 and FBEmu settings, this will break some games!",
        {
            {"Original", NULL},
            {"Fullspeed", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-virefresh",
        "VI Refresh (Overclock)",
        "Select a VI Refresh clock, Auto does not impact behaviour, other values override CountPerScanline",
        {
            {"Auto", NULL},
            {"1500", NULL},
            {"2200", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-astick-deadzone",
        "Analog Deadzone (percent)",
        "Select a Analog Deadzone",
        {
            {"15", NULL},
            {"20", NULL},
            {"25", NULL},
            {"30", NULL},
            {"0", NULL},
            {"5", NULL},
            {"10", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-astick-sensitivity",
        "Analog Sensitivity (percent)",
        "Select a Analog Sensitivity",
        {
            {"100", NULL},
            {"105", NULL},
            {"110", NULL},
            {"115", NULL},
            {"120", NULL},
            {"125", NULL},
            {"130", NULL},
            {"135", NULL},
            {"140", NULL},
            {"145", NULL},
            {"150", NULL},
            {"50", NULL},
            {"55", NULL},
            {"60", NULL},
            {"65", NULL},
            {"70", NULL},
            {"75", NULL},
            {"80", NULL},
            {"85", NULL},
            {"90", NULL},
            {"95", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-r-cbutton",
        "Right C Button",
        "Select Right C Button mapping",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-l-cbutton",
        "Left C Button",
        "Select Left C Button mapping",
        {
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            {"C1", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-d-cbutton",
        "Down C Button",
        "Select Down C Button mapping",
        {
            {"C3", NULL},
            {"C4", NULL},
            {"C1", NULL},
            {"C2", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-u-cbutton",
        "Up C Button",
        "Select Up C Button mapping",
        {
            {"C4", NULL},
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-alt-map",
        "Independent C-button Controls",
        "Use a alternate control scheme, useful for some 3rdparty controllers",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-ForceDisableExtraMem",
        "Disable Expansion Pak",
        "Force disable Expansion Pak (this might improve performance for some games at cost of fidelity and will break games that require it)",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-pak1",
        "Player 1 Pak",
        "Select Payer 1 Controller Pak",
        {
            {"memory", NULL},
            {"rumble", NULL},
            {"none", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-pak2",
        "Player 2 Pak",
        "Select Payer 2 Controller Pak",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-pak3",
        "Player 3 Pak",
        "Select Payer 3 Controller Pak",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-pak4",
        "Player 4 Pak",
        "Select Payer 4 Controller Pak",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {NULL, NULL},
        },
    },
    {
        CORE_NAME "-CountPerOp",
        "Count Per Op",
        "Count per Op is used to approximate instructions based on the embedded Database. This will break stuff!",
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {NULL, NULL},
        },
    },
    {NULL, NULL, NULL, {{0}}, NULL},
};

struct retro_core_option_definition *option_defs_intl[RETRO_LANGUAGE_LAST] = {
    option_defs_us, /* RETRO_LANGUAGE_ENGLISH */
    NULL,           /* RETRO_LANGUAGE_JAPANESE */
    NULL,           /* RETRO_LANGUAGE_FRENCH */
    NULL,           /* RETRO_LANGUAGE_SPANISH */
    NULL,           /* RETRO_LANGUAGE_GERMAN */
    NULL,           /* RETRO_LANGUAGE_ITALIAN */
    NULL,           /* RETRO_LANGUAGE_DUTCH */
    NULL,           /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
    NULL,           /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
    NULL,           /* RETRO_LANGUAGE_RUSSIAN */
    NULL,           /* RETRO_LANGUAGE_KOREAN */
    NULL,           /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
    NULL,           /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
    NULL,           /* RETRO_LANGUAGE_ESPERANTO */
    NULL,           /* RETRO_LANGUAGE_POLISH */
    NULL,           /* RETRO_LANGUAGE_VIETNAMESE */
    NULL,           /* RETRO_LANGUAGE_ARABIC */
    NULL,           /* RETRO_LANGUAGE_GREEK */
    NULL,           /* RETRO_LANGUAGE_TURKISH */
};

static INLINE void libretro_set_core_options(retro_environment_t environ_cb)
{
    unsigned version = 0;

    if (!environ_cb)
        return;

    if (environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version) && (version == 1))
    {
        struct retro_core_options_intl core_options_intl;
        unsigned language = 0;

        core_options_intl.us = option_defs_us;
        core_options_intl.local = NULL;

        if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
            (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
            core_options_intl.local = option_defs_intl[language];

        environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_intl);
    }
    else
    {
        size_t i;
        size_t option_index = 0;
        size_t num_options = 0;
        struct retro_variable *variables = NULL;
        char **values_buf = NULL;

        /* Determine number of options
    * > Note: We are going to skip a number of irrelevant
    *   core options when building the retro_variable array,
    *   but we'll allocate space for all of them. The difference
    *   in resource usage is negligible, and this allows us to
    *   keep the code 'cleaner' */
        while (true)
        {
            if (option_defs_us[num_options].key)
                num_options++;
            else
                break;
        }

        /* Allocate arrays */
        variables = (struct retro_variable *)calloc(num_options + 1, sizeof(struct retro_variable));
        values_buf = (char **)calloc(num_options, sizeof(char *));

        if (!variables || !values_buf)
            goto error;

        /* Copy parameters from option_defs_us array */
        for (i = 0; i < num_options; i++)
        {
            const char *key = option_defs_us[i].key;
            const char *desc = option_defs_us[i].desc;
            const char *default_value = option_defs_us[i].default_value;
            struct retro_core_option_value *values = option_defs_us[i].values;
            size_t buf_len = 3;
            size_t default_index = 0;

            values_buf[i] = NULL;

            if (desc)
            {
                size_t num_values = 0;

                /* Determine number of values */
                while (true)
                {
                    if (values[num_values].value)
                    {
                        /* Check if this is the default value */
                        if (default_value)
                            if (strcmp(values[num_values].value, default_value) == 0)
                                default_index = num_values;

                        buf_len += strlen(values[num_values].value);
                        num_values++;
                    }
                    else
                        break;
                }

                /* Build values string */
                if (num_values > 1)
                {
                    size_t j;

                    buf_len += num_values - 1;
                    buf_len += strlen(desc);

                    values_buf[i] = (char *)calloc(buf_len, sizeof(char));
                    if (!values_buf[i])
                        goto error;

                    strcpy(values_buf[i], desc);
                    strcat(values_buf[i], "; ");

                    /* Default value goes first */
                    strcat(values_buf[i], values[default_index].value);

                    /* Add remaining values */
                    for (j = 0; j < num_values; j++)
                    {
                        if (j != default_index)
                        {
                            strcat(values_buf[i], "|");
                            strcat(values_buf[i], values[j].value);
                        }
                    }
                }
            }

            variables[option_index].key = key;
            variables[option_index].value = values_buf[i];
            option_index++;
        }

        /* Set variables */
        environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

    error:

        /* Clean up */
        if (values_buf)
        {
            for (i = 0; i < num_options; i++)
            {
                if (values_buf[i])
                {
                    free(values_buf[i]);
                    values_buf[i] = NULL;
                }
            }

            free(values_buf);
            values_buf = NULL;
        }

        if (variables)
        {
            free(variables);
            variables = NULL;
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif
