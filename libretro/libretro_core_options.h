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

#include <mupen64plus-next_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define HAVE_NO_LANGEXTRA
struct retro_core_option_v2_category option_cats_us[] = {
   {
      "input",
      "Pak/Controller Options",
      "Configure Core Pak/Controller Options."
   },
   {
      "gliden64",
      "GLideN64",
      "Configure GLideN64 Options."
   },
#ifdef HAVE_PARALLEL_RDP
   {
      "parallel_rdp",
      "ParaLLEl-RDP",
      "Configure ParaLLEl-RDP Options."
   },
#endif // HAVE_PARALLEL_RDP
#ifdef HAVE_THR_AL
   {
      "angrylion",
      "Angrylion",
      "Configure Angrylion Options."
   },
#endif // HAVE_THR_AL
   { NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_us[] = {
    {
        CORE_NAME "-rdp-plugin",
        "RDP Plugin",
        NULL,
        "Select a RDP Plugin, use Angrylion (if available) for best compability, GLideN64 for Performance",
        NULL,
        NULL,
        {
#ifdef HAVE_THR_AL
            {"angrylion", "Angrylion"},
#endif
#ifdef HAVE_PARALLEL_RDP
            {"parallel", "ParaLLEl-RDP"},
#endif
            {"gliden64", "GLideN64"},
            { NULL, NULL },
        },
        "gliden64"
    },
    {
        CORE_NAME "-43screensize",
        "4:3 Resolution",
        NULL,
        "(GLN64) Select Render Viewport dimensions (4:3).",
        "Select Render Viewport dimensions (4:3).",
        "gliden64",
        {
            {"320x240", NULL},
            {"640x480", NULL},
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
            { NULL, NULL },
        },
        "640x480"
    },
    {
        CORE_NAME "-169screensize", // Keep this key for compatibility with existing config files. // TODO: revisit-later
        "Wide Resolution",
        NULL,
        "(GLN64) Select Render Viewport dimensions for wider resolutions.",
        "Select Render Viewport dimensions for wider resolutions.",
        "gliden64",
        {
            {"640x360",    "640x360 (16:9)"},
            {"960x540",    "960x540 (16:9)"},
            {"1280x720",   "1280x720 (16:9)"},
            {"1706x720",   "1706x720 (64:27)"},
            {"1366x768",   "1366x768 (16:9)"},
            {"1920x810",   "1920x810 (64:27)"},
            {"1920x1080",  "1920x1080 (16:9)"},
            {"2560x1080",  "2560x1080 (64:27)"},
            {"2560x1440",  "2560x1440 (16:9)"},
            {"3414x1440",  "3414x1440 (64:27)"},
            {"3840x2160",  "3840x2160 (16:9)"},
            {"4096x2160",  "4096x2160 (17:9)"},
            {"5120x2160",  "5120x2160 (64:27)"},
            {"7680x3240",  "7680x3240 (64:27)"},
            {"7680x4320",  "7680x4320 (16:9)"},
            {"10240x4320", "10240x4320 (64:27)"},
            { NULL, NULL },
        },
        "960x540"
    },
    {
        CORE_NAME "-aspect",
        "Aspect Ratio",
        NULL,
        "(GLN64) Select the aspect ratio. 'adjusted' means essentially Widescreen hacks.",
        "Select the aspect ratio. 'adjusted' means essentially Widescreen hacks.",
        "gliden64",
        {
            {"4:3", "Original (4:3)"},
            {"16:9", "Wide (Stretched)"},
            {"16:9 adjusted", "Wide (Adjusted)"}, // Calculates the aspect ratio based on the selected "16:9 resolution".
            { NULL, NULL },
        },
        "4:3"
    },
    {
        CORE_NAME "-EnableNativeResFactor",
        "Native Resolution Factor",
        NULL,
        "(GLN64) Render at N times the native resolution.",
        "Render at N times the native resolution.",
        "gliden64",
        {
            {"0", "Disabled"},
            {"1", "1x"},
            {"2", "2x"},
            {"3", "3x"},
            {"4", "4x"},
            {"5", "5x"},
            {"6", "6x"},
            {"7", "7x"},
            {"8", "8x"},
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-ThreadedRenderer",
        "Threaded Renderer",
        NULL,
        "(GLN64) Use the Threaded Renderer, improves performance but increases input lag.",
        "Use the Threaded Renderer, improves performance but increases input lag.",
        "gliden64",
        {
            {"True", "Enabled"},
            {"False", "Disabled"},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-BilinearMode",
        "Bilinear filtering mode",
        NULL,
        "(GLN64) Select a Bilinear filtering method, 3point is the original system specific way.",
        "Select a Bilinear filtering method, 3point is the original system specific way.",
        "gliden64",
        {
            {"3point", NULL},
            {"standard", NULL},
            { NULL, NULL },
        },
        "standard"
    },
    {
        CORE_NAME "-HybridFilter",
        "Hybrid Filter",
        NULL,
        "(GLN64) Hybrid integer scaling filter, this can be slow with low-end GPUs.",
        "Hybrid integer scaling filter, this can be slow with low-end GPUs.",
        "gliden64",
        {
            {"True", "Enabled"},
            {"False", "Disabled"},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-DitheringPattern",
        "Dithering",
        NULL,
        "(GLN64) Applies dithering pattern to output image.",
        "Applies dithering pattern to output image.",
        "gliden64",
        {
            {"True", "Enabled"},
            {"False", "Disabled"},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-DitheringQuantization",
        "Dithering Quantization",
        NULL,
        "(GLN64) Dither with color quantization.",
        "Dither with color quantization.",
        "gliden64",
        {
            {"True", "Enabled"},
            {"False", "Disabled"},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-RDRAMImageDitheringMode",
        "Image Dithering Mode",
        NULL,
        "(GLN64) Dithering mode for image in RDRAM.",
        "Dithering mode for image in RDRAM.",
        "gliden64",
        {
            {"False", "Disabled"},
            {"Bayer", NULL},
            {"MagicSquare", "Magic Square"},
            {"BlueNoise", "Blue Noise"},
            { NULL, NULL },
        },
        "False"
    },
#ifndef HAVE_OPENGLES2
    {
        CORE_NAME "-MultiSampling",
        "MSAA level",
        NULL,
        "(GLN64) Anti-Aliasing level (0 = disabled).",
        "Anti-Aliasing level (0 = disabled).",
        "gliden64",
        {
            {"0", NULL},
            {"2", NULL},
            {"4", NULL},
            {"8", NULL},
            {"16", NULL},
            { NULL, NULL },
        },
        "0"
    },
#endif
    {
        CORE_NAME "-FXAA",
        "FXAA",
        NULL,
        "(GLN64) Fast Approximate Anti-Aliasing shader, moderately blur textures (0 = disabled).",
        "Fast Approximate Anti-Aliasing shader, moderately blur textures (0 = disabled).",
        "gliden64",
        {
            {"0", NULL},
            {"1", NULL},
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-EnableLODEmulation",
        "LOD Emulation",
        NULL,
        "(GLN64) Calculate per-pixel Level Of Details to select texture mip levels and blend them with each other using LOD fraction.",
        "Calculate per-pixel Level Of Details to select texture mip levels and blend them with each other using LOD fraction.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-EnableFBEmulation",
        "Framebuffer Emulation",
        NULL,
        "(GLN64) Frame/depth buffer emulation. Disabling it can shorten input lag for particular games, but also break some special effects.",
        "Frame/depth buffer emulation. Disabling it can shorten input lag for particular games, but also break some special effects.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
#ifndef VC
        "True"
#else
        "False"
#endif // VC
    },
    {
        CORE_NAME "-EnableCopyAuxToRDRAM",
        "Copy auxiliary buffers to RDRAM",
        NULL,
        "(GLN64) Copy auxiliary buffers to RDRAM (fixes some Game artifacts like Paper Mario Intro).",
        "Copy auxiliary buffers to RDRAM (fixes some Game artifacts like Paper Mario Intro).",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False",
    },
    {
        CORE_NAME "-EnableCopyColorToRDRAM",
        "Color buffer to RDRAM",
        NULL,
        "(GLN64) Color buffer copy to RDRAM (Off will trade compatibility for Performance).",
        "Color buffer copy to RDRAM (Off will trade compatibility for Performance).",
        "gliden64",
        {
            {"Off", NULL},
            {"Sync", NULL},
#ifndef HAVE_OPENGLES2
            {"Async", "DoubleBuffer"},
            {"TripleBuffer", "TripleBuffer"},
#endif // HAVE_OPENGLES2
            { NULL, NULL },
        },
#ifndef HAVE_OPENGLES2
        "Async"
#else
        "Sync"
#endif // HAVE_OPENGLES2
    },
    {
        CORE_NAME "-EnableCopyColorFromRDRAM",
        "Enable color buffer copy from RDRAM",
        NULL,
        NULL,
        NULL,
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False",
    },
    {
        CORE_NAME "-EnableCopyDepthToRDRAM",
        "Depth buffer to RDRAM",
        NULL,
        "(GLN64) Depth buffer copy to RDRAM (Off will trade compatibility for Performance).",
        "Depth buffer copy to RDRAM (Off will trade compatibility for Performance).",
        "gliden64",
        {
            {"Off", NULL},
            {"Software", NULL},
            {"FromMem", NULL},
            { NULL, NULL },
        },
        "Software"
    },
    {
        CORE_NAME "-BackgroundMode",
        "Background Mode",
        NULL,
        "(GLN64) Render backgrounds mode (HLE only). One piece (fast), Stripped (precise).",
        "Render backgrounds mode (HLE only). One piece (fast), Stripped (precise).",
        "gliden64",
        {
            {"Stripped", NULL},
            {"OnePiece", NULL},
            { NULL, NULL },
        },
        "OnePiece"
    },
    {
        CORE_NAME "-EnableHWLighting",
        "Hardware per-pixel lighting",
        NULL,
        "(GLN64) Standard per-vertex lighting when disabled. Slightly different rendering.",
        "Standard per-vertex lighting when disabled. Slightly different rendering.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-CorrectTexrectCoords",
        "Continuous texrect coords",
        NULL,
        "(GLN64) Make texrect coordinates continuous to avoid black lines between them.",
        "Make texrect coordinates continuous to avoid black lines between them.",
        "gliden64",
        {
            {"Off", NULL},
            {"Auto", NULL},
            {"Force", NULL},
            { NULL, NULL },
        },
        "Off"
    },
    {
        CORE_NAME "-EnableInaccurateTextureCoordinates",
        "Enable inaccurate texture coordinates",
        NULL,
        "(GLN64) Enables inaccurate texture coordinate calculations. This can improve performance and texture pack compatibity at the cost of accuracy.",
        "Enables inaccurate texture coordinate calculations. This can improve performance and texture pack compatibity at the cost of accuracy.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-EnableTexCoordBounds",
        "Enable native-res boundaries for texture coordinates",
        NULL,
        "(GLN64) Bound texture rectangle texture coordinates to the values they take in native resolutions. It prevents garbage due to fetching out of texture bounds, but can result in hard edges.",
        "Bound texture rectangle texture coordinates to the values they take in native resolutions. It prevents garbage due to fetching out of texture bounds, but can result in hard edges.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-EnableNativeResTexrects",
        "Native res. 2D texrects",
        NULL,
        "(GLN64) Render 2D texrects in native resolution to fix misalignment between parts of 2D image (example: Mario Kart driver selection portraits).",
        "Render 2D texrects in native resolution to fix misalignment between parts of 2D image (example: Mario Kart driver selection portraits).",
        "gliden64",
        {
            {"Disabled", NULL},
            {"Unoptimized", NULL},
            {"Optimized", NULL},
            { NULL, NULL },
        },
        "Disabled"
    },
    {
        CORE_NAME "-EnableLegacyBlending",
        "Less accurate blending mode",
        NULL,
        "(GLN64) Do not use shaders to emulate N64 blending modes. Works faster on slow GPU. Can cause glitches.",
        "Do not use shaders to emulate N64 blending modes. Works faster on slow GPU. Can cause glitches.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
#ifdef HAVE_OPENGLES
        "True"
#else
        "False"
#endif
    },
    {
        CORE_NAME "-EnableFragmentDepthWrite",
        "GPU shader depth write",
        NULL,
        "(GLN64) Enable writing of fragment depth. Some mobile GPUs do not support it, thus it's optional. Leave enabled.",
        "Enable writing of fragment depth. Some mobile GPUs do not support it, thus it's optional. Leave enabled.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
#ifdef HAVE_OPENGLES
        "False"
#else
        "True"
#endif
    },
#if !defined(VC) && !defined(HAVE_OPENGLES)
    {
        CORE_NAME "-EnableN64DepthCompare",
        "N64 Depth Compare",
        NULL,
        "(GLN64) Enable N64 depth compare instead of OpenGL standard one. Experimental, Fast mode will have more glitches.",
        "Enable N64 depth compare instead of OpenGL standard one. Experimental, Fast mode will have more glitches.",
        "gliden64",
        {
            {"False", "Off"},
            {"True", "Fast"},
            {"Compatible", NULL},
        },
        "False"
    },
    {
        CORE_NAME "-EnableShadersStorage",
        "Cache GPU Shaders",
        NULL,
        "(GLN64) Use persistent storage for compiled shaders.",
        "Use persistent storage for compiled shaders.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
#endif
    {
        CORE_NAME "-EnableTextureCache",
        "Cache Textures",
        NULL,
        "(GLN64) Save texture cache to hard disk.",
        "Save texture cache to hard disk.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-EnableOverscan",
        "Overscan",
        NULL,
        "(GLN64) Crop black borders from the overscan region around the screen.",
        "Crop black borders from the overscan region around the screen.",
        "gliden64",
        {
            {"Disabled", NULL},
            {"Enabled", NULL},
            { NULL, NULL },
        },
        "Enabled"
    },
    {
        CORE_NAME "-OverscanTop",
        "Overscan Offset (Top)",
        NULL,
        "(GLN64) Overscan Top Offset.",
        "Overscan Top Offset.",
        "gliden64",
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
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-OverscanLeft",
        "Overscan Offset (Left)",
        NULL,
        "(GLN64) Overscan Left Offset.",
        "Overscan Left Offset.",
        "gliden64",
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
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-OverscanRight",
        "Overscan Offset (Right)",
        NULL,
        "(GLN64) Overscan Right Offset.",
        "Overscan Right Offset.",
        "gliden64",
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
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-OverscanBottom",
        "Overscan Offset (Bottom)",
        NULL,
        "(GLN64) Overscan Bottom Offset.",
        "Overscan Bottom Offset.",
        "gliden64",
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
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-MaxHiResTxVramLimit",
        "Max High-Res VRAM Limit",
        NULL,
        "(GLN64) Limit High-Res textures size in VRAM (in MB, 0 = no limit).",
        "Limit High-Res textures size in VRAM (in MB, 0 = no limit).",
        "gliden64",
        {
            {"0", NULL},
            {"500", NULL},
            {"1000", NULL},
            {"1500", NULL},
            {"2000", NULL},
            {"2500", NULL},
            {"3000", NULL},
            {"3500", NULL},
            {"4000", NULL},
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-MaxTxCacheSize",
        "Max texture cache size",
        NULL,
        "(GLN64) Set Max texture cache size (in elements). Reduce it if you experience black textures leading to a crash.",
        "Set Max texture cache size (in elements). Reduce it if you experience black textures leading to a crash.",
        "gliden64",
        {
            {"1500", NULL},
            {"4000", NULL},
            {"8000", NULL},
            { NULL, NULL },
        },
#if defined(VC)
        "1500"
#elif defined(HAVE_LIBNX)
        "4000"
#else
        "8000"
#endif
    },
    {
        CORE_NAME "-txFilterMode",
        "Texture filter",
        NULL,
        "(GLN64) Select Texture Filtering mode.",
        "Select Texture Filtering mode.",
        "gliden64",
        {
            {"None", NULL},
            {"Smooth filtering 1", NULL},
            {"Smooth filtering 2", NULL},
            {"Smooth filtering 3", NULL},
            {"Smooth filtering 4", NULL},
            {"Sharp filtering 1", NULL},
            {"Sharp filtering 2", NULL},
            { NULL, NULL },
        },
        "None"
    },
    {
        CORE_NAME "-txEnhancementMode",
        "Texture Enhancement",
        NULL,
        "(GLN64) Various Texture Filters ('As-Is' will just cache).",
        "Various Texture Filters ('As-Is' will just cache).",
        "gliden64",
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
            { NULL, NULL },
        },
        "None"
    },
    {
        CORE_NAME "-txFilterIgnoreBG",
        "Don't filter background textures",
        NULL,
        "(GLN64) Ignore filtering for Background Textures.",
        "Ignore filtering for Background Textures.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-txHiresEnable",
        "Use High-Res textures",
        NULL,
        "(GLN64) Enable High-Res Texture packs if available.",
        "Enable High-Res Texture packs if available.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-txCacheCompression",
        "Use High-Res Texture Cache Compression",
        NULL,
        "(GLN64) Compress created texture caches.",
        "Compress created texture caches.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-txHiresFullAlphaChannel",
        "Use High-Res Full Alpha Channel",
        NULL,
        "(GLN64) This should be enabled unless it's a old RICE Texture pack.",
        "This should be enabled unless it's a old RICE Texture pack.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-EnableEnhancedTextureStorage",
        "Use enhanced Texture Storage",
        NULL,
        "(GLN64) Use in addition to Texture cache, will use lazy loading and trade memory consumption against loading speeds.",
        "Use in addition to Texture cache, will use lazy loading and trade memory consumption against loading speeds.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-EnableHiResAltCRC",
        "Use alternative method for High-Res Checksums",
        NULL,
        "(GLN64) Use an alternative method for High-Res paletted textures CRC calculations.",
        "Use an alternative method for High-Res paletted textures CRC calculations.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-EnableEnhancedHighResStorage",
        "Use enhanced Hi-Res Storage",
        NULL,
        "(GLN64) Use in addition to High-Res textures, will use lazy loading and trade memory consumption against loading speeds.",
        "Use in addition to High-Res textures, will use lazy loading and trade memory consumption against loading speeds.",
        "gliden64",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-GLideN64IniBehaviour",
        "INI Behaviour",
        NULL,
        "(GLN64) Specifies INI Settings behaviour. This should really only contain essential options. Changing this can and will break ROM's, if the correct options aren't set manually. Some options may only be set via INI (fbInfoDisabled).",
        "Specifies INI Settings behaviour. This should really only contain essential options. Changing this can and will break ROM's, if the correct options aren't set manually. Some options may only be set via INI (fbInfoDisabled).",
        "gliden64",
        {
            {"late", "Prioritize INI over Core Options"},
            {"early", "Prioritize Core Options over INI"},
            {"disabled", "Disable INI"},
            { NULL, NULL },
        },
        "late"
    },
#ifdef HAVE_PARALLEL_RDP
    {
        CORE_NAME "-parallel-rdp-synchronous",
        "(ParaLLEl-RDP) Synchronous RDP",
        "Synchronous RDP",
        "Enable full accuracy for CPU accessed frame buffers.",
        NULL,
        "parallel_rdp",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-overscan",
        "(ParaLLEl-RDP) Crop overscan",
        "Crop overscan",
        "Crop pixels around edge of screen.",
        NULL,
        "parallel_rdp",
        {
            { "0", NULL },
            { "2", NULL },
            { "4", NULL },
            { "6", NULL },
            { "8", NULL },
            { "10", NULL },
            { "12", NULL },
            { "14", NULL },
            { "16", NULL },
            { "18", NULL },
            { "20", NULL },
            { "22", NULL },
            { "24", NULL },
            { "26", NULL },
            { "28", NULL },
            { "30", NULL },
            { "32", NULL },
            { "34", NULL },
            { "36", NULL },
            { "38", NULL },
            { "40", NULL },
            { "42", NULL },
            { "44", NULL },
            { "46", NULL },
            { "48", NULL },
            { "50", NULL },
            { "52", NULL },
            { "54", NULL },
            { "56", NULL },
            { "58", NULL },
            { "60", NULL },
            { "62", NULL },
            { "64", NULL },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-divot-filter",
        "(ParaLLEl-RDP) VI Divot filter",
        "VI Divot filter",
        "Allow VI divot filter, cleans up stray black pixels.",
        NULL,
        "parallel_rdp",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
            { NULL, NULL },
        }

    },
    {
        CORE_NAME "-parallel-rdp-gamma-dither",
        "(ParaLLEl-RDP) VI Gamma dither",
        "VI Gamma dither",
        "Allow VI gamma dither.",
        NULL,
        "parallel_rdp",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
            { NULL, NULL },
        }

    },
    {
        CORE_NAME "-parallel-rdp-vi-aa",
        "(ParaLLEl-RDP) VI anti-aliasing",
        "VI anti-aliasing",
        "Allow VI anti-aliased fetch filter, smooths polygon edges.",
        NULL,
        "parallel_rdp",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
            { NULL, NULL },
        }

    },
    {
        CORE_NAME "-parallel-rdp-vi-bilinear",
        "(ParaLLEl-RDP) VI bilinear",
        "VI bilinear",
        "Allow VI bilinear scaling on scanout.",
        NULL,
        "parallel_rdp",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
            { NULL, NULL },
        }

    },
    {
        CORE_NAME "-parallel-rdp-dither-filter",
        "(ParaLLEl-RDP) VI dither filter",
        "VI dither filter",
        "Allow VI de-dither filter, recovers significant color depth.",
        NULL,
        "parallel_rdp",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-upscaling",
        "(ParaLLEl-RDP) Upscaling factor (restart)",
        "Upscaling factor (restart)",
        "Apply internal upscaling factor.",
        NULL,
        "parallel_rdp",
        {
            { "1x", NULL },
            { "2x", NULL },
            { "4x", NULL },
            { "8x", NULL },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-super-sampled-read-back",
        "(ParaLLEl-RDP) SSAA framebuffer effects (restart)",
        "SSAA framebuffer effects (restart)",
        "Super sample framebuffer effects. May introduce artifacts.",
        NULL,
        "parallel_rdp",
        {
            { "False", "Disabled" },
            { "True", "Enabled" },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-super-sampled-read-back-dither",
        "(ParaLLEl-RDP) Dither SSAA framebuffer effects (restart)",
        "Dither SSAA framebuffer effects (restart)",
        "Dither super sampled framebuffer effects.",
        NULL,
        "parallel_rdp",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-downscaling",
        "(ParaLLEl-RDP) Downsampling factor",
        "Downsampling factor",
        "Downscales output after VI, equivalent to SSAA.",
        NULL,
        "parallel_rdp",
        {
            { "disable", NULL },
            { "1/2", NULL },
            { "1/4", NULL },
            { "1/8", NULL },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-native-texture-lod",
        "(ParaLLEl-RDP) Native texture LOD",
        "Native texture LOD",
        "Use native texture LOD computation when upscaling, effectively a LOD bias.",
        NULL,
        "parallel_rdp",
        {
            { "False", "Disabled" },
            { "True", "Enabled" },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-native-tex-rect",
        "(ParaLLEl-RDP) Native resolution TEX_RECT",
        "Native resolution TEX_RECT",
        "TEX_RECT primitives should generally be rendered at native resolution to avoid seams.",
        NULL,
        "parallel_rdp",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
            { NULL, NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-deinterlace-method",
        "(ParaLLEl-RDP) Deinterlacing method",
        "Deinterlacing method",
        "Weave should only be used with 1x scaling factor and special CRT shaders.",
        NULL,
        "parallel_rdp",
        {
            { "Bob", NULL },
            { "Weave", NULL },
            { NULL, NULL },
        }
    },
#endif
#ifdef HAVE_THR_AL
    {
        CORE_NAME "-angrylion-vioverlay",
        "VI Overlay",
        NULL,
        "(AL) Select VI Overlay filtering. 'Filtered' is the original system rendering.",
        "Select VI Overlay filtering. 'Filtered' is the original system rendering.",
        "angrylion",
        {
            {"Filtered", NULL},
            {"AA+Blur", NULL},
            {"AA+Dedither", NULL},
            {"AA only", NULL},
            {"Unfiltered", NULL},
            {"Depth", NULL},
            {"Coverage", NULL},
            { NULL, NULL },
        },
        "Filtered"
    },
    {
        CORE_NAME "-angrylion-sync",
        "Thread sync level",
        NULL,
        "(AL) Select Sync level (trades accuracy for performance).",
        "Select Sync level (trades accuracy for performance).",
        "angrylion",
        {
            {"Low", NULL},
            {"Medium", NULL},
            {"High", NULL},
            { NULL, NULL },
        },
        "Low"
    },
    {
        CORE_NAME "-angrylion-multithread",
        "Multi-threading",
        NULL,
        "(AL) Default 'all threads' is prefered to have it match your Physical CPU Core count. '1' should behave as the original angrylion, possibly fixing some bugs.",
        "Default 'all threads' is prefered to have it match your Physical CPU Core count. '1' should behave as the original angrylion, possibly fixing some bugs.",
        "angrylion",
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
            { NULL, NULL },
        },
        "all threads"
    },
    {
        CORE_NAME "-angrylion-overscan",
        "Hide overscan",
        NULL,
        "(AL) Hide overscan borders.",
        "Hide overscan borders.",
        "angrylion",
        {
            {"disabled", NULL},
            {"enabled", NULL},
            { NULL, NULL },
        },
        "disabled"
    },
#endif
    {
        CORE_NAME "-cpucore",
        "CPU Core",
        NULL,
        "Select the R4300 CPU Backend, use Interpreter for best compability",
        NULL,
        NULL,
        {
            {"pure_interpreter", "Pure Interpreter"},
            {"cached_interpreter", "Cached Interpreter"},
#ifdef DYNAREC
            {"dynamic_recompiler", "Dynarec"},
#endif
            { NULL, NULL },
        },
#ifdef DYNAREC
        "dynamic_recompiler"
#else
        "cached_interpreter"
#endif
    },
    {
        CORE_NAME "-rsp-plugin",
        "RSP Plugin",
        NULL,
        "Select a RSP Plugin, use HLE for best performance, ParaLLEl for best LLE Performance and CXD4 as LLE fallback",
        NULL,
        NULL,
        {
#ifdef HAVE_LLE
            {"cxd4", "CXD4"},
#endif
#ifdef HAVE_PARALLEL_RSP
            {"parallel", "ParaLLEl"},
#endif
            {"hle", "HLE"},
            { NULL, NULL },
        },
        "hle"
    },
    {
        CORE_NAME "-FrameDuping",
        "Frame Duplication",
        NULL,
        "Enable Frame duplication to improve smoothing on low-end. Different from frameskip.",
        NULL,
        NULL,
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
#ifdef HAVE_LIBNX
        "True"
#else
        "False"
#endif
    },
    {
        CORE_NAME "-Framerate",
        "Framerate",
        NULL,
        "Fullspeed will enforce Count per Op 1 and FBEmu settings, this will break some games!",
        NULL,
        NULL,
        {
            {"Original", NULL},
            {"Fullspeed", NULL},
            { NULL, NULL },
        },
        "Original"
    },
    {
        CORE_NAME "-virefresh",
        "VI Refresh (Overclock)",
        NULL,
        "Select a VI Refresh clock, Auto does not impact behaviour, other values override CountPerScanline.",
        NULL,
        NULL,
        {
            {"Auto", NULL},
            {"1500", NULL},
            {"2200", NULL},
            { NULL, NULL },
        },
        "Auto"
    },
    {
        CORE_NAME "-ForceDisableExtraMem",
        "Disable Expansion Pak",
        NULL,
        "Force disable Expansion Pak (might improve performance for some games while reducing emulation accuracy, will break games that require it).",
        NULL,
        NULL,
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-IgnoreTLBExceptions",
        "Ignore emulated TLB Exceptions",
        NULL,
        "(HACK) Ignore emulated TLB Exceptions, this might fix some broken romhacks. This option might be removed in the future.",
        NULL,
        NULL,
        {
            {"False", "Don't Ignore"},
            {"OnlyNotEnabled", "Ignore TLB Exceptions if not using TLB"},
            {"AlwaysIgnoreTLB", "Always Ignore TLB Exceptions"},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-CountPerOp",
        "Count Per Op",
        NULL,
        "Count per Op is used to approximate the Counter reg, 0 will use the embedded Database (or default to 2). Changing this will break stuff!",
        NULL,
        NULL,
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-CountPerOpDenomPot",
        "Count Per Op Divider (Overclock)",
        NULL,
        "Denominator for Count per Op (allowing sub-1 Count per Op in practice). Changing this can break stuff!",
        NULL,
        NULL,
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
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-astick-deadzone",
        "Analog Deadzone (percent)",
        NULL,
        "Size of the non responsive area around an analog stick.",
        NULL,
        "input",
        {
            {"0", NULL},
            {"5", NULL},
            {"10", NULL},
            {"15", NULL},
            {"20", NULL},
            {"25", NULL},
            {"30", NULL},
            { NULL, NULL },
        },
        "15"
    },
    {
        CORE_NAME "-astick-sensitivity",
        "Analog Sensitivity (percent)",
        NULL,
        "Adjust how far the stick needs to be moved to reach its max value.",
        NULL,
        "input",
        {
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
            { NULL, NULL },
        },
        "100"
    },
    {
        CORE_NAME "-astick-snap-angle-active",
        "Snap Analog Stick Angle to Cardinal Directions",
        NULL,
        "Should the analog stick snap to a cardinal direction to better support circular design controllers",
        NULL,
        "input",
        {
            {"enabled", NULL},
            {"disabled", NULL},
            { NULL, NULL },
        },
        "disabled"
    },
    {
        CORE_NAME "-astick-snap-max-angle",
        "Maximum Snap Angle",
        NULL,
        "Up to what angle difference should the controller snap to the next cardinal direction. Example: Set to 5 will snap values from 85 to 95 degrees to 90",
        NULL,
        "input",
        {
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
            { NULL, NULL },
        },
        "15"
    },
    {
        CORE_NAME "-r-cbutton",
        "Right C Button",
        NULL,
        "Select Right C Button mapping.",
        NULL,
        "input",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            { NULL, NULL },
        },
        "C1"
    },
    {
        CORE_NAME "-l-cbutton",
        "Left C Button",
        NULL,
        "Select Left C Button mapping.",
        NULL,
        "input",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            { NULL, NULL },
        },
        "C2"
    },
    {
        CORE_NAME "-d-cbutton",
        "Down C Button",
        NULL,
        "Select Down C Button mapping.",
        NULL,
        "input",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            { NULL, NULL },
        },
        "C3"
    },
    {
        CORE_NAME "-u-cbutton",
        "Up C Button",
        NULL,
        "Select Up C Button mapping.",
        NULL,
        "input",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            { NULL, NULL },
        },
        "C4"
    },
    {
        CORE_NAME "-alt-map",
        "Independent C-button Controls",
        NULL,
        "Use an alternate control scheme, useful for some 3rdparty controllers.",
        NULL,
        "input",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-pak1",
        "Player 1 Pak",
        NULL,
        "Select Player 1 Controller Pak.",
        NULL,
        "input",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {"transfer", NULL},
            { NULL, NULL },
        },
        "memory"
    },
    {
        CORE_NAME "-pak2",
        "Player 2 Pak",
        NULL,
        "Select Player 2 Controller Pak.",
        NULL,
        "input",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {"transfer", NULL},
            { NULL, NULL },
        },
        "none"
    },
    {
        CORE_NAME "-pak3",
        "Player 3 Pak",
        NULL,
        "Select Player 3 Controller Pak.",
        NULL,
        "input",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {"transfer", NULL},
            { NULL, NULL },
        },
        "none"
    },
    {
        CORE_NAME "-pak4",
        "Player 4 Pak",
        NULL,
        "Select Player 4 Controller Pak.",
        NULL,
        "input",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {"transfer", NULL},
            { NULL, NULL },
        },
        "none"
    },
    { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};

struct retro_core_options_v2 options_us = {
   option_cats_us,
   option_defs_us
};

struct retro_core_options_v2 *option_defs_intl[RETRO_LANGUAGE_LAST] = {
   &options_us, /* RETRO_LANGUAGE_ENGLISH */
   NULL,            /* RETRO_LANGUAGE_JAPANESE */
   NULL,            /* RETRO_LANGUAGE_FRENCH */
   NULL,            /* RETRO_LANGUAGE_SPANISH */
   NULL,            /* RETRO_LANGUAGE_GERMAN */
   NULL,            /* RETRO_LANGUAGE_ITALIAN */
   NULL,            /* RETRO_LANGUAGE_DUTCH */
   NULL,            /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
   NULL,            /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
   NULL,            /* RETRO_LANGUAGE_RUSSIAN */
   NULL,            /* RETRO_LANGUAGE_KOREAN */
   NULL,            /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
   NULL,            /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
   NULL,            /* RETRO_LANGUAGE_ESPERANTO */
   NULL,            /* RETRO_LANGUAGE_POLISH */
   NULL,            /* RETRO_LANGUAGE_VIETNAMESE */
   NULL,            /* RETRO_LANGUAGE_ARABIC */
   NULL,            /* RETRO_LANGUAGE_GREEK */
   NULL,            /* RETRO_LANGUAGE_TURKISH */
   NULL,            /* RETRO_LANGUAGE_SLOVAK */
   NULL,            /* RETRO_LANGUAGE_PERSIAN */
   NULL,            /* RETRO_LANGUAGE_HEBREW */
   NULL,            /* RETRO_LANGUAGE_ASTURIAN */
   NULL,            /* RETRO_LANGUAGE_FINNISH */
};


static INLINE void libretro_set_core_options(retro_environment_t environ_cb,
      bool *categories_supported)
{
   unsigned version  = 0;
#ifndef HAVE_NO_LANGEXTRA
   unsigned language = 0;
#endif

   if (!environ_cb || !categories_supported)
      return;

   *categories_supported = false;

   if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
      version = 0;

   if (version >= 2)
   {
#ifndef HAVE_NO_LANGEXTRA
      struct retro_core_options_v2_intl core_options_intl;

      core_options_intl.us    = &options_us;
      core_options_intl.local = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
          (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
         core_options_intl.local = options_intl[language];

      *categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL,
            &core_options_intl);
#else
      *categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2,
            &options_us);
#endif
   }
   else
   {
      size_t i, j;
      size_t option_index              = 0;
      size_t num_options               = 0;
      struct retro_core_option_definition
            *option_v1_defs_us         = NULL;
#ifndef HAVE_NO_LANGEXTRA
      size_t num_options_intl          = 0;
      struct retro_core_option_v2_definition
            *option_defs_intl          = NULL;
      struct retro_core_option_definition
            *option_v1_defs_intl       = NULL;
      struct retro_core_options_intl
            core_options_v1_intl;
#endif
      struct retro_variable *variables = NULL;
      char **values_buf                = NULL;

      /* Determine total number of options */
      while (true)
      {
         if (option_defs_us[num_options].key)
            num_options++;
         else
            break;
      }

      if (version >= 1)
      {
         /* Allocate US array */
         option_v1_defs_us = (struct retro_core_option_definition *)
               calloc(num_options + 1, sizeof(struct retro_core_option_definition));

         /* Copy parameters from option_defs_us array */
         for (i = 0; i < num_options; i++)
         {
            struct retro_core_option_v2_definition *option_def_us = &option_defs_us[i];
            struct retro_core_option_value *option_values         = option_def_us->values;
            struct retro_core_option_definition *option_v1_def_us = &option_v1_defs_us[i];
            struct retro_core_option_value *option_v1_values      = option_v1_def_us->values;

            option_v1_def_us->key           = option_def_us->key;
            option_v1_def_us->desc          = option_def_us->desc;
            option_v1_def_us->info          = option_def_us->info;
            option_v1_def_us->default_value = option_def_us->default_value;

            /* Values must be copied individually... */
            while (option_values->value)
            {
               option_v1_values->value = option_values->value;
               option_v1_values->label = option_values->label;

               option_values++;
               option_v1_values++;
            }
         }

#ifndef HAVE_NO_LANGEXTRA
         if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
             (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH) &&
             options_intl[language])
            option_defs_intl = options_intl[language]->definitions;

         if (option_defs_intl)
         {
            /* Determine number of intl options */
            while (true)
            {
               if (option_defs_intl[num_options_intl].key)
                  num_options_intl++;
               else
                  break;
            }

            /* Allocate intl array */
            option_v1_defs_intl = (struct retro_core_option_definition *)
                  calloc(num_options_intl + 1, sizeof(struct retro_core_option_definition));

            /* Copy parameters from option_defs_intl array */
            for (i = 0; i < num_options_intl; i++)
            {
               struct retro_core_option_v2_definition *option_def_intl = &option_defs_intl[i];
               struct retro_core_option_value *option_values           = option_def_intl->values;
               struct retro_core_option_definition *option_v1_def_intl = &option_v1_defs_intl[i];
               struct retro_core_option_value *option_v1_values        = option_v1_def_intl->values;

               option_v1_def_intl->key           = option_def_intl->key;
               option_v1_def_intl->desc          = option_def_intl->desc;
               option_v1_def_intl->info          = option_def_intl->info;
               option_v1_def_intl->default_value = option_def_intl->default_value;

               /* Values must be copied individually... */
               while (option_values->value)
               {
                  option_v1_values->value = option_values->value;
                  option_v1_values->label = option_values->label;

                  option_values++;
                  option_v1_values++;
               }
            }
         }

         core_options_v1_intl.us    = option_v1_defs_us;
         core_options_v1_intl.local = option_v1_defs_intl;

         environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_v1_intl);
#else
         environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, option_v1_defs_us);
#endif
      }
      else
      {
         /* Allocate arrays */
         variables  = (struct retro_variable *)calloc(num_options + 1,
               sizeof(struct retro_variable));
         values_buf = (char **)calloc(num_options, sizeof(char *));

         if (!variables || !values_buf)
            goto error;

         /* Copy parameters from option_defs_us array */
         for (i = 0; i < num_options; i++)
         {
            const char *key                        = option_defs_us[i].key;
            const char *desc                       = option_defs_us[i].desc;
            const char *default_value              = option_defs_us[i].default_value;
            struct retro_core_option_value *values = option_defs_us[i].values;
            size_t buf_len                         = 3;
            size_t default_index                   = 0;

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
               if (num_values > 0)
               {
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

            variables[option_index].key   = key;
            variables[option_index].value = values_buf[i];
            option_index++;
         }

         /* Set variables */
         environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
      }

error:
      /* Clean up */

      if (option_v1_defs_us)
      {
         free(option_v1_defs_us);
         option_v1_defs_us = NULL;
      }

#ifndef HAVE_NO_LANGEXTRA
      if (option_v1_defs_intl)
      {
         free(option_v1_defs_intl);
         option_v1_defs_intl = NULL;
      }
#endif

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
