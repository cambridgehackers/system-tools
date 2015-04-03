
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := peekmem
LOCAL_SRC_FILES := peekmem.c

#LOCAL_ARM_MODE := arm
#LOCAL_MODULE_TAGS := optional
#LOCAL_LDLIBS := -llog   
#LOCAL_CPPFLAGS := "-march=armv7-a"
#LOCAL_CFLAGS := -DZYNQ 
#LOCAL_CXXFLAGS := -DZYNQ

include $(BUILD_EXECUTABLE)
