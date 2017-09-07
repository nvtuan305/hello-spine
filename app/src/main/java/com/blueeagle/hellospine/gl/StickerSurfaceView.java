package com.blueeagle.hellospine.gl;

/*
 * Created by tuan.nv on 9/7/2017.
 */

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

public class StickerSurfaceView extends GLSurfaceView {

    private final String TAG = "StickerSurfaceView";
    private StickerRenderer mRenderer;

    public StickerSurfaceView(Context context) {
        super(context);
        init();
    }

    public StickerSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        // Set OpenGL version
        if (GLHelper.isSupportOpenGL(getContext(), 0x20000)) {
            Log.d(TAG, "This device is supported OpenGL v2.0");
            setEGLContextClientVersion(2);
        } else {
            Log.d(TAG, "This device is supported OpenGL v1.0");
            setEGLContextClientVersion(1);
        }

        // Set renderer for view
        mRenderer = new StickerRenderer();
        setRenderer(mRenderer);
    }
}
