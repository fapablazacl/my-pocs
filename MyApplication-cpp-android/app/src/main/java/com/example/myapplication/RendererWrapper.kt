package com.example.myapplication

import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class RendererWrapper : GLSurfaceView.Renderer {
    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        // GLES20.glClearColor(0.0f, 0.0f, 1.0f, 1.0f)
        NativeGameLogic.on_surface_created()
    }

    override fun onDrawFrame(p0: GL10?) {
        // GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)
        NativeGameLogic.on_draw_frame()
    }

    override fun onSurfaceChanged(p0: GL10?, p1: Int, p2: Int) {
        NativeGameLogic.on_surface_changed(p1, p2)
    }
}
