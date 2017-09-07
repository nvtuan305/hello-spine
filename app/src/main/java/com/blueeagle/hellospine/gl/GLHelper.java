package com.blueeagle.hellospine.gl;

/*
 * Created by tuan.nv on 9/7/2017.
 */

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;

public class GLHelper {

    /**
     * Check device is supported OpenGL version
     */
    public static boolean isSupportOpenGL(Context context, int version) {
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo ci = am.getDeviceConfigurationInfo();
        return ci.reqGlEsVersion >= version;
    }

}
