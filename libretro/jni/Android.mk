LOCAL_PATH := $(call my-dir)

# Reset flags that the common makefile doesn't properly handle
DYNAFLAGS    :=
GLFLAGS      :=
HAVE_NEON    :=
SOURCES_C    :=
SOURCES_CXX  :=
SOURCES_ASM  :=
SOURCES_NASM :=

ROOT_DIR     := $(LOCAL_PATH)/../..
LIBRETRO_DIR := $(ROOT_DIR)/libretro
INCFLAGS     := -I$(ROOT_DIR)/GLideN64/src/GLideNHQ/inc

PLATFORM_EXT := android
platform     := android
LLE          := 1

ifeq ($(TARGET_ARCH),arm)
  WITH_DYNAREC := arm
  HAVE_NEON := 1
  STATIC_LIBS := $(ROOT_DIR)/custom/android/arm/libpng.a $(ROOT_DIR)/custom/android/arm/libui.so
endif

ifeq ($(TARGET_ARCH),arm64)
  WITH_DYNAREC := arm64
  STATIC_LIBS := $(ROOT_DIR)/custom/android/arm64/libpng.a
endif

ifeq ($(TARGET_ARCH),x86)
  WITH_DYNAREC := x86
  STATIC_LIBS := $(ROOT_DIR)/custom/android/x86/libpng.a $(ROOT_DIR)/custom/android/x86/libui.so
endif

ifeq ($(GLES3),1)
  GLLIB := -lGLESv3
else
  GLES  := 1
  GLLIB := -lGLESv2
endif

include $(ROOT_DIR)/Makefile.common

COREFLAGS += -D__LIBRETRO__ -DUSE_FILE32API -DM64P_PLUGIN_API -DM64P_CORE_PROTOTYPES -D_ENDUSER_RELEASE -DSINC_LOWER_QUALITY -DMUPENPLUSAPI -DTXFILTER_LIB -D__VEC4_OPT $(INCFLAGS) $(GLFLAGS) $(DYNAFLAGS) -DANDROID -DEGL_EGLEXT_PROTOTYPES -DOS_LINUX

include $(CLEAR_VARS)
LOCAL_MODULE := retro
LOCAL_SRC_FILES = $(SOURCES_CXX) $(SOURCES_C) $(SOURCES_ASM) $(SOURCES_NASM)
LOCAL_CPPFLAGS := -std=gnu++11 $(COREFLAGS)
LOCAL_CFLAGS := $(COREFLAGS)
LOCAL_LDFLAGS := -Wl,-Bsymbolic -shared
LOCAL_LDLIBS :=  -lz -llog -lEGL -latomic $(GLLIB) $(STATIC_LIBS)
LOCAL_ARM_NEON := true
LOCAL_CPP_FEATURES := exceptions
include $(BUILD_SHARED_LIBRARY)

