package com.blueeagle.hellospine.gl;

/*
 * Created by tuan.nv on 9/7/2017.
 */

public class StickerRendererManager {

    static {
        System.loadLibrary("sticker-lib");
    }

    public void init() {
        initStickerView();
    }

    public void stickerSurfaceChanged(int width, int height) {
        onStickerSurfaceChanged(width, height);
    }

    public void drawStickerFrame() {
        onStickerDrawFrame();
    }

    /*
     * ----------------------------------------------------------------------
     * Native method declaration
     * ----------------------------------------------------------------------
     */
    private native void initStickerView();

    private native void onStickerSurfaceChanged(int width, int height);

    private native void onStickerDrawFrame();
}
