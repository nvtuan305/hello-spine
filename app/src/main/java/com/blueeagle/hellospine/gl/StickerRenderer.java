package com.blueeagle.hellospine.gl;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/*
 * Created by tuan.nv on 9/7/2017.
 */

public class StickerRenderer implements GLSurfaceView.Renderer {

    private StickerRendererManager mStickerManager;

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        mStickerManager = new StickerRendererManager();
        mStickerManager.init();
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        if (mStickerManager != null)
            mStickerManager.stickerSurfaceChanged(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        if (mStickerManager != null)
            mStickerManager.drawStickerFrame();
    }
}
