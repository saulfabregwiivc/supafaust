LOCAL_PATH := $(call my-dir)

CORE_DIR := $(LOCAL_PATH)/..

DEBUG                    := 0
FRONTEND_SUPPORTS_RGB565 := 1
NEED_BPP                 := 32
NEED_BLIP                := 1
NEED_STEREO_SOUND        := 1
IS_X86                   := 0
FLAGS                    :=

COREFLAGS                :=

ifeq ($(TARGET_ARCH),x86)
  IS_X86 := 1
endif

ifeq ($(TARGET_ARCH_ABI),armeabi)
COREFLAGS += -DMDFN_USE_COND_TIMEDWAIT_RELATIVE_NP
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
COREFLAGS += -DMDFN_USE_COND_TIMEDWAIT_RELATIVE_NP
endif

ifeq ($(TARGET_ARCH_ABI),mips)
COREFLAGS += -DMDFN_USE_COND_TIMEDWAIT_RELATIVE_NP
endif

ifeq ($(TARGET_ARCH_ABI),x86)
COREFLAGS += -DMDFN_USE_COND_TIMEDWAIT_RELATIVE_NP
endif

include $(CORE_DIR)/Makefile.common

COREFLAGS += -funroll-loops $(INCFLAGS) -DLSB_FIRST -DPSS_STYLE=1 -D__LIBRETRO__ -D_LOW_ACCURACY_ $(FLAGS)
COREFLAGS += -DWANT_SNES_EMU

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

include $(CLEAR_VARS)
LOCAL_MODULE       := retro
LOCAL_SRC_FILES    := $(SOURCES_CXX) $(SOURCES_C)
LOCAL_CFLAGS       := $(COREFLAGS)
LOCAL_CXXFLAGS     := $(COREFLAGS)
LOCAL_LDFLAGS      := -Wl,-version-script=$(CORE_DIR)/link.T
LOCAL_CPP_FEATURES := exceptions rtti
include $(BUILD_SHARED_LIBRARY)
