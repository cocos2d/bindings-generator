LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := scriptingcore-spidermonkey

LOCAL_MODULE_FILENAME := libscriptingcore-spidermonkey

LOCAL_SRC_FILES := ScriptingCore.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_WHOLE_STATIC_LIBRARIES := spidermonkey_static

LOCAL_LDLIBS := -landroid
LOCAL_LDLIBS += -llog

include $(BUILD_STATIC_LIBRARY)

$(call import-module,spidermonkey/android)
