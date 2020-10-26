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

#define HAVE_NO_LANGEXTRA

struct retro_core_option_definition option_defs_us[] = {
    {
        CORE_NAME "-cpucore",
        "CPU Core",
        "Select the R4300 CPU Backend, use Interpreter for best compability",
        {
            {"pure_interpreter", "Pure Interpreter"},
            {"cached_interpreter", "Cached Interpreter"},
#ifdef DYNAREC
            {"dynamic_recompiler", "Dynarec"},
#endif
            {NULL, NULL},
        },
#ifdef DYNAREC
        "dynamic_recompiler"
#else
        "cached_interpreter"
#endif
    },
    {
        CORE_NAME "-rdp-plugin",
        "RDP Plugin",
        "Select a RDP Plugin, use Angrylion (if available) for best compability, GLideN64 for Performance",
        {
#ifdef HAVE_THR_AL
            {"angrylion", "Angrylion"},
#endif
#ifdef HAVE_PARALLEL_RDP
            {"parallel", "paraLLEl-RDP"},
#endif
            {"gliden64", "GLideN64"},
            {NULL, NULL},
        },
        "gliden64"
    },
    {
        CORE_NAME "-rsp-plugin",
        "RSP Plugin",
        "Select a RSP Plugin, use HLE for best performance, ParaLLEl for best LLE Performance and CXD4 as LLE fallback",
        {
#ifdef HAVE_LLE
            {"cxd4", "CXD4"},
#endif
#ifdef HAVE_PARALLEL_RSP
            {"parallel", "ParaLLEl"},
#endif
            {"hle", "HLE"},
            {NULL, NULL},
        },
        "hle"
    },
    {
        CORE_NAME "-43screensize",
        "4:3 Resolution",
        "(GLN64) Select Render Viewport dimensions (4:3).",
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
            {NULL, NULL},
        },
        "640x480"
    },
    {
        CORE_NAME "-169screensize", // Keep this key for compatibility with existing config files. // TODO: revisit-later
        "Wide Resolution",
        "(GLN64) Select Render Viewport dimensions for wider resolutions.",
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
            {NULL, NULL},
        },
        "960x540"
    },
    {
        CORE_NAME "-aspect",
        "Aspect Ratio",
        "(GLN64) Select the aspect ratio. 'adjusted' means essentially Widescreen hacks.",
        {
            {"4:3", "Original (4:3)"},
            {"16:9", "Wide (Stretched)"},
            {"16:9 adjusted", "Wide (Adjusted)"}, // Calculates the aspect ratio based on the selected "16:9 resolution".
            {NULL, NULL},
        },
        "4:3"
    },
    {
        CORE_NAME "-EnableNativeResFactor",
        "Native Resolution Factor",
        "(GLN64) Render at N times the native resolution.",
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
            {NULL, NULL},
        },
        "0"
    },
    {
        CORE_NAME "-ThreadedRenderer",
        "Threaded Renderer",
        "(GLN64) Use the Threaded Renderer, improves performance but increases input lag.",
        {
            {"True", "Enabled"},
            {"False", "Disabled"},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-BilinearMode",
        "Bilinear filtering mode",
        "(GLN64) Select a Bilinear filtering method, 3point is the original system specific way.",
        {
            {"3point", NULL},
            {"standard", NULL},
            {NULL, NULL},
        },
        "standard"
    },
    {
        CORE_NAME "-HybridFilter",
        "Hybrid Filter",
        "(GLN64) Hybrid integer scaling filter, this can be slow with low-end GPUs.",
        {
            {"True", "Enabled"},
            {"False", "Disabled"},
            {NULL, NULL},
        },
        "True"
    },
    {
        CORE_NAME "-DitheringPattern",
        "Dithering",
        "(GLN64) Applies dithering pattern to output image.",
        {
            {"True", "Enabled"},
            {"False", "Disabled"},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-DitheringQuantization",
        "Dithering Quantization",
        "(GLN64) Dither with color quantization.",
        {
            {"True", "Enabled"},
            {"False", "Disabled"},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-RDRAMImageDitheringMode",
        "Image Dithering Mode",
        "(GLN64) Dithering mode for image in RDRAM.",
        {
            {"False", "Disabled"},
            {"Bayer", NULL},
            {"MagicSquare", "Magic Square"},
            {"BlueNoise", "Blue Noise"},
            {NULL, NULL},
        },
        "False"
    },
#ifndef HAVE_OPENGLES2
    {
        CORE_NAME "-MultiSampling",
        "MSAA level",
        "(GLN64) Anti-Aliasing level (0 = disabled).",
        {
            {"0", NULL},
            {"2", NULL},
            {"4", NULL},
            {"8", NULL},
            {"16", NULL},
            {NULL, NULL},
        },
        "0"
    },
#endif
    {
        CORE_NAME "-FXAA",
        "FXAA",
        "(GLN64) Fast Approximate Anti-Aliasing shader, moderately blur textures (0 = disabled).",
        {
            {"0", NULL},
            {"1", NULL},
            {NULL, NULL},
        },
        "0"
    },
    {
        CORE_NAME "-EnableLODEmulation",
        "LOD Emulation",
        "(GLN64) Calculate per-pixel Level Of Details to select texture mip levels and blend them with each other using LOD fraction.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "True"
    },
    {
        CORE_NAME "-EnableFBEmulation",
        "Framebuffer Emulation",
        "(GLN64) Frame/depth buffer emulation. Disabling it can shorten input lag for particular games, but also break some special effects.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
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
        "(GLN64) Copy auxiliary buffers to RDRAM (fixes some Game artifacts like Paper Mario Intro) .",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False",
    },
    {
        CORE_NAME "-EnableCopyColorToRDRAM",
        "Color buffer to RDRAM",
        "(GLN64) Color buffer copy to RDRAM (Off will trade compatibility for Performance).",
        {
            {"Off", NULL},
            {"Sync", NULL},
#ifndef HAVE_OPENGLES2
            {"Async", "DoubleBuffer"},
            {"TripleBuffer", "TripleBuffer"},
#endif // HAVE_OPENGLES2
            {NULL, NULL},
        },
#ifndef HAVE_OPENGLES2
        "Async"
#else
        "Sync"
#endif // HAVE_OPENGLES2
    },
    {
        CORE_NAME "-EnableCopyDepthToRDRAM",
        "Depth buffer to RDRAM",
        "(GLN64) Depth buffer copy to RDRAM (Off will trade compatibility for Performance).",
        {
            {"Off", NULL},
            {"Software", NULL},
            {"FromMem", NULL},
            {NULL, NULL},
        },
        "Software"
    },
    {
        CORE_NAME "-BackgroundMode",
        "Background Mode",
        "(GLN64) Render backgrounds mode (HLE only). One piece (fast), Stripped (precise)",
        {
            {"Stripped", NULL},
            {"OnePiece", NULL},
            {NULL, NULL},
        },
        "OnePiece"
    },
    {
        CORE_NAME "-EnableHWLighting",
        "Hardware per-pixel lighting",
        "(GLN64) Standard per-vertex lighting when disabled. Slightly different rendering.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-CorrectTexrectCoords",
        "Continuous texrect coords",
        "(GLN64) Make texrect coordinates continuous to avoid black lines between them.",
        {
            {"Off", NULL},
            {"Auto", NULL},
            {"Force", NULL},
            {NULL, NULL},
        },
        "Off"
    },
    {
        CORE_NAME "-EnableNativeResTexrects",
        "Native res. 2D texrects",
        "(GLN64) Render 2D texrects in native resolution to fix misalignment between parts of 2D image (example: Mario Kart driver selection portraits).",
        {
            {"Disabled", NULL},
            {"Unoptimized", NULL},
            {"Optimized", NULL},
            {NULL, NULL},
        },
        "Disabled"
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
#ifdef HAVE_OPENGLES
        "True"
#else
        "False"
#endif
    },
    {
        CORE_NAME "-EnableFragmentDepthWrite",
        "GPU shader depth write",
        "(GLN64) Enable writing of fragment depth. Some mobile GPUs do not support it, thus it's optional. Leave enabled.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
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
        "(GLN64) Enable N64 depth compare instead of OpenGL standard one. Experimental, Fast mode will have more glitches.",
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
        "(GLN64) Use persistent storage for compiled shaders.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "True"
    },
#endif
    {
        CORE_NAME "-EnableTextureCache",
        "Cache Textures",
        "(GLN64) Save texture cache to hard disk.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "True"
    },
    {
        CORE_NAME "-EnableOverscan",
        "Overscan",
        "(GLN64) Crop black borders from the overscan region around the screen.",
        {
            {"Disabled", NULL},
            {"Enabled", NULL},
            {NULL, NULL},
        },
        "Enabled"
    },
    {
        CORE_NAME "-OverscanTop",
        "Overscan Offset (Top)",
        "(GLN64) Overscan Top Offset.",
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
        "0"
    },
    {
        CORE_NAME "-OverscanLeft",
        "Overscan Offset (Left)",
        "(GLN64) Overscan Left Offset.",
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
        "0"
    },
    {
        CORE_NAME "-OverscanRight",
        "Overscan Offset (Right)",
        "(GLN64) Overscan Right Offset.",
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
        "0"
    },
    {
        CORE_NAME "-OverscanBottom",
        "Overscan Offset (Bottom)",
        "(GLN64) Overscan Bottom Offset.",
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
        "0"
    },
    {
        CORE_NAME "-MaxTxCacheSize",
        "Max texture cache size",
        "(GLN64) Set Max texture cache size (in elements). Reduce it if you experience black textures leading to a crash.",
        {
            {"1500", NULL},
            {"4000", NULL},
            {"8000", NULL},
            {NULL, NULL},
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
        "(GLN64) Select Texture Filtering mode.",
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
        "None"
    },
    {
        CORE_NAME "-txEnhancementMode",
        "Texture Enhancement",
        "(GLN64) Various Texture Filters ('As-Is' will just cache).",
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
        "None"
    },
    {
        CORE_NAME "-txFilterIgnoreBG",
        "Don't filter background textures",
        "(GLN64) Ignore filtering for Background Textures.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "True"
    },
    {
        CORE_NAME "-txHiresEnable",
        "Use High-Res textures",
        "(GLN64) Enable High-Res Texture packs if available.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-txCacheCompression",
        "Use High-Res Texture Cache Compression",
        "(GLN64) Compress created texture caches.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "True"
    },
    {
        CORE_NAME "-txHiresFullAlphaChannel",
        "Use High-Res Full Alpha Channel",
        "(GLN64) This should be enabled unless it's a old RICE Texture pack.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-EnableEnhancedTextureStorage",
        "Use enhanced Texture Storage",
        "(GLN64) Use in addition to Texture cache, will use lazy loading and trade memory consumption against loading speeds.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-EnableHiResAltCRC",
        "Use alternative method for High-Res Checksums",
        "(GLN64) Use an alternative method for High-Res paletted textures CRC calculations.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-EnableEnhancedHighResStorage",
        "Use enhanced Hi-Res Storage",
        "(GLN64) Use in addition to High-Res textures, will use lazy loading and trade memory consumption against loading speeds.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False"
    },
#ifdef HAVE_PARALLEL_RDP
    {
        CORE_NAME "-parallel-rdp-synchronous",
        "(paraLLEl-RDP) Synchronous RDP",
        "Enable full accuracy for CPU accessed frame buffers.",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
        }
    },
    {
        CORE_NAME "-parallel-rdp-overscan",
        "(ParaLLEl-RDP) Crop overscan",
        "Crop pixels around edge of screen.",
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
        }
    },
    {
        CORE_NAME "-parallel-rdp-divot-filter",
        "(ParaLLEl-RDP) VI Divot filter",
        "Allow VI divot filter, cleans up stray black pixels.",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
        }

    },
    {
        CORE_NAME "-parallel-rdp-gamma-dither",
        "(ParaLLEl-RDP) VI Gamma dither",
        "Allow VI gamma dither.",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
        }

    },
    {
        CORE_NAME "-parallel-rdp-vi-aa",
        "(ParaLLEl-RDP) VI anti-aliasing",
        "Allow VI anti-aliased fetch filter, smooths polygon edges.",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
        }

    },
    {
        CORE_NAME "-parallel-rdp-vi-bilinear",
        "(ParaLLEl-RDP) VI bilinear",
        "Allow VI bilinear scaling on scanout.",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
        }

    },
    {
        CORE_NAME "-parallel-rdp-dither-filter",
        "(ParaLLEl-RDP) VI dither filter",
        "Allow VI de-dither filter, recovers significant color depth.",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
        }
    },
    {
        CORE_NAME "-parallel-rdp-upscaling",
        "(ParaLLEl-RDP) Upscaling factor (restart)",
        "Apply internal upscaling factor.",
        {
            { "1x", NULL },
            { "2x", NULL },
            { "4x", NULL },
            { "8x", NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-super-sampled-read-back",
        "(ParaLLEl-RDP) SSAA framebuffer effects (restart)",
        "Super sample framebuffer effects. May introduce artifacts.",
        {
            { "False", "Disabled" },
            { "True", "Enabled" },
        }
    },
    {
        CORE_NAME "-parallel-rdp-super-sampled-read-back-dither",
        "(ParaLLEl-RDP) Dither SSAA framebuffer effects (restart)",
        "Dither super sampled framebuffer effects.",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
        }
    },
    {
        CORE_NAME "-parallel-rdp-downscaling",
        "(ParaLLEl-RDP) Downsampling factor",
        "Downscales output after VI, equivalent to SSAA.",
        {
            { "disable", NULL },
            { "1/2", NULL },
            { "1/4", NULL },
            { "1/8", NULL },
        }
    },
    {
        CORE_NAME "-parallel-rdp-native-texture-lod",
        "(ParaLLEl-RDP) Native texture LOD",
        "Use native texture LOD computation when upscaling, effectively a LOD bias.",
        {
            { "False", "Disabled" },
            { "True", "Enabled" },
        }
    },
    {
        CORE_NAME "-parallel-rdp-native-tex-rect",
        "(ParaLLEl-RDP) Native resolution TEX_RECT",
        "TEX_RECT primitives should generally be rendered at native resolution to avoid seams.",
        {
            { "True", "Enabled" },
            { "False", "Disabled" },
        }
    },
    {
        CORE_NAME "-parallel-rdp-deinterlace-method",
        "(ParaLLEl-RDP) Deinterlacing method",
        "Weave should only be used with 1x scaling factor and special CRT shaders.",
        {
            { "Bob", NULL },
            { "Weave", NULL },
        }
    },
#endif
#ifdef HAVE_THR_AL
    {
        CORE_NAME "-angrylion-vioverlay",
        "VI Overlay",
        "(AL) Select VI Overlay filtering. 'Filtered' is the original system rendering.",
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
        "Filtered"
    },
    {
        CORE_NAME "-angrylion-sync",
        "Thread sync level",
        "(AL) Select Sync level (trades accuracy for performance).",
        {
            {"Low", NULL},
            {"Medium", NULL},
            {"High", NULL},
            {NULL, NULL},
        },
        "Low"
    },
    {
        CORE_NAME "-angrylion-multithread",
        "Multi-threading",
        "(AL) Default 'all threads' is prefered to have it match your Physical CPU Core count. '1' should behave as the original angrylion, possibly fixing some bugs.",
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
        "all threads"
    },
    {
        CORE_NAME "-angrylion-overscan",
        "Hide overscan",
        "(AL) Hide overscan borders.",
        {
            {"disabled", NULL},
            {"enabled", NULL},
            {NULL, NULL},
        },
        "disabled"
    },
#endif
    {
        CORE_NAME "-FrameDuping",
        "Frame Duplication",
        "Enable Frame duplication to improve smoothing on low-end. Different from frameskip.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
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
        "Fullspeed will enforce Count per Op 1 and FBEmu settings, this will break some games!",
        {
            {"Original", NULL},
            {"Fullspeed", NULL},
            {NULL, NULL},
        },
        "Original"
    },
    {
        CORE_NAME "-virefresh",
        "VI Refresh (Overclock)",
        "Select a VI Refresh clock, Auto does not impact behaviour, other values override CountPerScanline.",
        {
            {"Auto", NULL},
            {"1500", NULL},
            {"2200", NULL},
            {NULL, NULL},
        },
        "Auto"
    },
    {
        CORE_NAME "-astick-deadzone",
        "Analog Deadzone (percent)",
        "Size of the non responsive area around an analog stick.",
        {
            {"0", NULL},
            {"5", NULL},
            {"10", NULL},
            {"15", NULL},
            {"20", NULL},
            {"25", NULL},
            {"30", NULL},
            {NULL, NULL},
        },
        "15"
    },
    {
        CORE_NAME "-astick-sensitivity",
        "Analog Sensitivity (percent)",
        "Adjust how far the stick needs to be moved to reach its max value.",
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
            {NULL, NULL},
        },
        "100"
    },
    {
        CORE_NAME "-r-cbutton",
        "Right C Button",
        "Select Right C Button mapping.",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            {NULL, NULL},
        },
        "C1"
    },
    {
        CORE_NAME "-l-cbutton",
        "Left C Button",
        "Select Left C Button mapping.",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            {NULL, NULL},
        },
        "C2"
    },
    {
        CORE_NAME "-d-cbutton",
        "Down C Button",
        "Select Down C Button mapping.",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            {NULL, NULL},
        },
        "C3"
    },
    {
        CORE_NAME "-u-cbutton",
        "Up C Button",
        "Select Up C Button mapping.",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            {NULL, NULL},
        },
        "C4"
    },
    {
        CORE_NAME "-alt-map",
        "Independent C-button Controls",
        "Use an alternate control scheme, useful for some 3rdparty controllers.",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-ForceDisableExtraMem",
        "Disable Expansion Pak",
        "Force disable Expansion Pak (might improve performance for some games while reducing emulation accuracy, will break games that require it).",
        {
            {"False", NULL},
            {"True", NULL},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-IgnoreTLBExceptions",
        "Ignore emulated TLB Exceptions",
        "(HACK) Ignore emulated TLB Exceptions, this might fix some broken romhacks and works around the OoT dynarec freeze. This option might be removed in the future.",
        {
            {"False", "Don't Ignore"},
            {"OnlyNotEnabled", "Ignore TLB Exceptions if not using TLB"},
            {"AlwaysIgnoreTLB", "Always Ignore TLB Exceptions"},
            {NULL, NULL},
        },
        "False"
    },
    {
        CORE_NAME "-pak1",
        "Player 1 Pak",
        "Select Player 1 Controller Pak.",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {NULL, NULL},
        },
        "memory"
    },
    {
        CORE_NAME "-pak2",
        "Player 2 Pak",
        "Select Player 2 Controller Pak.",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {NULL, NULL},
        },
        "none"
    },
    {
        CORE_NAME "-pak3",
        "Player 3 Pak",
        "Select Player 3 Controller Pak.",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {NULL, NULL},
        },
        "none"
    },
    {
        CORE_NAME "-pak4",
        "Player 4 Pak",
        "Select Player 4 Controller Pak.",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {NULL, NULL},
        },
        "none"
    },
    {
        CORE_NAME "-CountPerOp",
        "Count Per Op",
        "Count per Op is used to approximate the Counter reg, 0 will use the embedded Database (or default to 2). Changing this will break stuff!",
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            {NULL, NULL},
        },
        "0"
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

   if (environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version) && (version >= 1))
   {
#ifndef HAVE_NO_LANGEXTRA
      struct retro_core_options_intl core_options_intl;
      unsigned language = 0;

      core_options_intl.us    = option_defs_us;
      core_options_intl.local = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
          (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
         core_options_intl.local = option_defs_intl[language];

      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_intl);
#else
      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, &option_defs_us);
#endif
   }
   else
   {
      size_t i;
      size_t num_options               = 0;
      struct retro_variable *variables = NULL;
      char **values_buf                = NULL;

      /* Determine number of options */
      for (;;)
      {
         if (!option_defs_us[num_options].key)
            break;
         num_options++;
      }

      /* Allocate arrays */
      variables  = (struct retro_variable *)calloc(num_options + 1, sizeof(struct retro_variable));
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
            for (;;)
            {
               if (!values[num_values].value)
                  break;

               /* Check if this is the default value */
               if (default_value)
                  if (strcmp(values[num_values].value, default_value) == 0)
                     default_index = num_values;

               buf_len += strlen(values[num_values].value);
               num_values++;
            }

            /* Build values string */
            if (num_values > 0)
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

         variables[i].key   = key;
         variables[i].value = values_buf[i];
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
