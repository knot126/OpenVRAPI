LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE  := arm
LOCAL_MODULE    := vrapi
LOCAL_SRC_FILES := main.c
LOCAL_LDLIBS    := -ldl -llog -landroid -lGLESv3
# LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
