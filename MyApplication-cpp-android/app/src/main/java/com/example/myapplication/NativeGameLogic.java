package com.example.myapplication;

import android.content.res.AssetManager;

public class NativeGameLogic {
    static {
        System.loadLibrary("native-lib");
    }

    public static native void on_surface_created();

    public static native void on_surface_changed(int width, int height);

    public static native void on_draw_frame();

    public static native void on_initial_setup(AssetManager assetManager);
}
