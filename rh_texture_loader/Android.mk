LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := rh_texture_loader
LOCAL_SRC_FILES := rh_texture_loader.c lz4.c
include $(BUILD_STATIC_LIBRARY)


