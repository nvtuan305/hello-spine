#include <jni.h>
#include <Sticker.h>

Sticker *mSticker = NULL;
const char *atlasPath = "/sdcard/Sticker/HPBD/HPBD.atlas";
const char *jsonPath = "/sdcard/Sticker/HPBD/HPBD.json";
const char *imagePath = "/sdcard/Sticker/HPBD/HPBD.png";
const char *defAnimation = "animation";

/*
 * ----------------------------------------------------------------------------------
 * JNIEXPORT function - START
 * ----------------------------------------------------------------------------------
 */

extern "C"
JNIEXPORT void JNICALL
Java_com_blueeagle_hellospine_gl_StickerRenderer_initStickerView(JNIEnv *env,
                                                                        jobject instance) {
    if (mSticker) {
        delete mSticker;
        mSticker = NULL;
    }

    mSticker = new Sticker(atlasPath, jsonPath, imagePath, defAnimation);
    mSticker->init();

    // Set blend func
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_blueeagle_hellospine_gl_StickerRenderer_onStickerSurfaceChanged(JNIEnv *env,
                                                                                jobject instance,
                                                                                jint width,
                                                                                jint height) {
    if (mSticker)
        mSticker->resize(width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_blueeagle_hellospine_gl_StickerRenderer_onStickerDrawFrame(JNIEnv *env,
                                                                           jobject instance) {
    if (mSticker) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mSticker->draw();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_blueeagle_hellospine_gl_StickerRenderer_destroySticker(JNIEnv *env,
                                                                       jobject instance) {
    if (mSticker) {
        delete mSticker;
        mSticker = NULL;
    }
}

/*
 * ----------------------------------------------------------------------------------
 * JNIEXPORT function - END
 * ----------------------------------------------------------------------------------
 */