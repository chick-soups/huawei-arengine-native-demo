package com.example.selfdemo;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Display;
import android.view.SurfaceView;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity
implements GLSurfaceView.Renderer{

    private  GLSurfaceView glSurfaceView;

    // Used to load the 'selfdemo' library on application startup.
    static {
        System.loadLibrary("selfdemo");
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        if(checkSelfPermission(Manifest.permission.CAMERA)!= PackageManager.PERMISSION_GRANTED){
            requestPermissions(new String[]{Manifest.permission.CAMERA},1);
        }
        glSurfaceView=findViewById(R.id.glSurfaceView);
        glSurfaceView.setEGLContextClientVersion(3);
        glSurfaceView.setPreserveEGLContextOnPause(true);
        glSurfaceView.setRenderer(this);
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);






    }

    @Override
    protected void onStart() {
        super.onStart();
        glSurfaceView.onResume();
        OnActivityStart(getApplicationContext());
    }

    @Override
    protected void onResume() {
        super.onResume();
        OnActivityResume(getApplicationContext());
    }

    @Override
    protected void onPause() {
        super.onPause();

        OnActivityPause(getApplicationContext());
    }

    @Override
    protected void onStop() {
        super.onStop();
        glSurfaceView.onPause();
        OnActivityStop(getApplicationContext());
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        OnActivityDestory(getApplicationContext());
    }

    private  native void OnActivityStart(Context context);

    private native void OnActivityResume(Context context);
    private  native void OnActivityPause(Context context);
    private  native void OnActivityStop(Context context);
    private  native void OnActivityDestory(Context context);

    private  native void UpdateViewPort(int rotation,int width,int height);
    private  native void OnSurfaceCrteated();
    private  native void OnSurfaceDraw();




    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Display display=getWindowManager().getDefaultDisplay();
        int rotation = display.getRotation();
        int width = glSurfaceView.getWidth();
        int height=glSurfaceView.getHeight();
        UpdateViewPort(rotation,width,height);
        OnSurfaceCrteated();

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        Display display = getWindowManager().getDefaultDisplay();
        int rotation = display.getRotation();
        UpdateViewPort(rotation,width,height);

    }

    @Override
    public void onDrawFrame(GL10 gl) {
       OnSurfaceDraw();
    }
}