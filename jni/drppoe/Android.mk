# Copyright (C) 2014 The PPOE Project
# PPOE Module Makefile
#
# Created on: 2014年2月14日
# Author: Kingsley Yau
# Email: Kingsleyyau@gmail.com
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := drppoe

LOCAL_MODULE_FILENAME := libdrppoe

LOCAL_STATIC_LIBRARIES := drcommon

REAL_PATH := $(realpath $(LOCAL_PATH))
LOCAL_SRC_FILES := $(call all-cpp-files-under, $(REAL_PATH))

#include $(BUILD_SHARED_LIBRARY) 
include $(BUILD_STATIC_LIBRARY) 
