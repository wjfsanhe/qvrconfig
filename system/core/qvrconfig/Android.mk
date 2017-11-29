LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)

LOCAL_SRC_FILES := qvrconfig.c
LOCAL_MODULE := qvrconfig
LOCAL_CFLAGS := -Wall -Wno-unused-parameter -Werror
#LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_STATIC_LIBRARIES :=  libz libutils libcutils liblog libm libc

include $(BUILD_EXECUTABLE)
