#include <jni.h>
#include <Sticker.h>

#define LOG_TAG "GLES2JNIWRAPPER_CPP"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__

Sticker *mSticker = NULL;
const char *atlasPath = "/sdcard/Sticker/raptor/raptor.atlas";
const char *jsonPath = "/sdcard/Sticker/raptor/raptor.json";
const char *imagePath = "/sdcard/Sticker/raptor/raptor.png";
const char *defAnimation = "roar";

/*
 * ----------------------------------------------------------------------------------
 * JNIEXPORT function - START
 * ----------------------------------------------------------------------------------
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_blueeagle_hellospine_gl_StickerRendererManager_initStickerView(JNIEnv *env,
                                                                        jobject instance) {
    mSticker = new Sticker(atlasPath, jsonPath, imagePath, defAnimation);
    mSticker->init();

    // Set blend func
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_blueeagle_hellospine_gl_StickerRendererManager_onStickerSurfaceChanged(JNIEnv *env,
                                                                                jobject instance,
                                                                                jint width,
                                                                                jint height) {
    if (mSticker)
        mSticker->resize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_blueeagle_hellospine_gl_StickerRendererManager_onStickerDrawFrame(JNIEnv *env,
                                                                           jobject instance) {
    if (mSticker) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mSticker->draw();
    }
}

/*
 * ----------------------------------------------------------------------------------
 * JNIEXPORT function - END
 * ----------------------------------------------------------------------------------
 */