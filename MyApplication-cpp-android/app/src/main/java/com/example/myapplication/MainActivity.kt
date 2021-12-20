package com.example.myapplication

import android.app.Activity
import android.app.ActivityManager
import android.content.Context
import android.opengl.GLSurfaceView
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {
    private lateinit var surfaceView: GLSurfaceView

    private var isSurfaceViewSetted = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        var manager = this.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        var info = manager.deviceConfigurationInfo

        val supportsES2 = info.reqGlEsVersion >= 0x2000

        if (supportsES2 || isProbablyEmulator()) {
            surfaceView = GLSurfaceView(this)

            if (isProbablyEmulator()) {
                surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0)
            }

            surfaceView.setEGLContextClientVersion(2)
            surfaceView.setRenderer(RendererWrapper())

            setContentView(surfaceView)
        } else {
            setContentView(R.layout.activity_main)
            sample_text.text = stringFromJNI()
        }

        NativeGameLogic.on_initial_setup(assets)
    }

    private fun isProbablyEmulator(): Boolean {
        return false;
    }

    override fun onPause() {
        super.onPause()

        if (isSurfaceViewSetted) {
            surfaceView.onPause()
        }
    }


    override fun onResume() {
        super.onResume()

        if (isSurfaceViewSetted) {
            surfaceView.onResume()
        }
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}
