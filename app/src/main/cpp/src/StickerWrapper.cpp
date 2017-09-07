#include <jni.h>
#include <GLES2/gl2.h>
#include <android/log.h>

#define LOG_TAG "GLES2JNIWRAPPER_CPP"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)