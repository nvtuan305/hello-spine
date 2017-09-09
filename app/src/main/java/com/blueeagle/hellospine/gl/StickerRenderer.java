package com.blueeagle.hellospine.gl;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/*
 * Created by tuan.nv on 9/7/2017.
 */

public class StickerRenderer implements GLSurfaceView.Renderer {

    static {
        System.loadLibrary("sticker-lib");
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        initStickerView();
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        onStickerSurfaceChanged(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        onStickerDrawFrame();
    }

    public void onSurfaceViewDestroyed() {
        destroySticker();
    }

    /*
     * ----------------------------------------------------------------------
     * Native method declaration
     * ----------------------------------------------------------------------
     */
    private native void initStickerView();

    private native void onStickerSurfaceChanged(int width, int height);

    private native void onStickerDrawFrame();

    private native void destroySticker();
}
