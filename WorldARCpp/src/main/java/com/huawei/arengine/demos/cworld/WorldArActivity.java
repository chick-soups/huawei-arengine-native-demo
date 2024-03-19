/**
 * Copyright 2022. Huawei Technologies Co., Ltd. All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

package com.huawei.arengine.demos.cworld;

import android.app.Activity;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Toast;

import androidx.annotation.NonNull;

/**
 * This is sample example that shows how to create an augmented reality (AR) application using the
 * AREngine C API.
 *
 * @author HW
 * @since 2020-09-18
 */
public class WorldArActivity extends Activity {
    private static final String TAG = WorldArActivity.class.getSimpleName();

    private static final int CONFIG_CHOOSER_RED_SIZE = 8;

    private static final int CONFIG_CHOOSER_GREEN_SIZE = 8;

    private static final int CONFIG_CHOOSER_BLUE_SIZE = 8;

    private static final int CONFIG_CHOOSER_ALPHA_SIZE = 8;

    private static final int CONFIG_CHOOSER_DEPTH_SIZE = 16;

    private static final int CONFIG_CHOOSER_STENCIL_SIZE = 0;

    private GLSurfaceView mSurfaceView;

    // Opaque native pointer to the native application instance.
    private long mNativeApplication;

    private GestureDetector mGestureDetector;

    private DisplayRotationManager mDisplayRotationManager;

    private boolean isRemindInstall = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        PermissionManager.checkPermission(this);
        initUi();
        initGestureDetector();
    }

    private void initUi() {
        mSurfaceView = findViewById(R.id.surfaceView);
        mDisplayRotationManager = new DisplayRotationManager(this);
        WorldRenderManager worldRenderManager = new WorldRenderManager(this);
        if (mSurfaceView != null) {
            // Set up renderer.
            mSurfaceView.setPreserveEGLContextOnPause(true);

            // Set the openGL version number.
            mSurfaceView.setEGLContextClientVersion(2);

            // Alpha used for plane blending.
            mSurfaceView.setEGLConfigChooser(CONFIG_CHOOSER_RED_SIZE, CONFIG_CHOOSER_GREEN_SIZE,
                CONFIG_CHOOSER_BLUE_SIZE, CONFIG_CHOOSER_ALPHA_SIZE,
                CONFIG_CHOOSER_DEPTH_SIZE, CONFIG_CHOOSER_STENCIL_SIZE);
            mSurfaceView.setRenderer(worldRenderManager);
            mSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        }

        JniInterface.setAssetManager(getAssets());
        mNativeApplication = JniInterface.createNativeApplication(getAssets());
        worldRenderManager.setDisplayRotationManage(mDisplayRotationManager);
        worldRenderManager.setNativeApplication(mNativeApplication);
    }

    /**
     * Set up tap listener.
     */
    private void initGestureDetector() {
        mGestureDetector = new GestureDetector(this, new GestureDetector.SimpleOnGestureListener() {
            @Override
            public boolean onSingleTapUp(final MotionEvent motionEvent) {
                mSurfaceView.queueEvent(
                    () -> JniInterface.onTouched(mNativeApplication, motionEvent.getX(), motionEvent.getY()));
                return true;
            }

            @Override
            public boolean onDown(MotionEvent motionEvent) {
                return true;
            }
        });

        mSurfaceView.setOnTouchListener((view, event) -> mGestureDetector.onTouchEvent(event));
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (!arEngineAbilityCheck()) {
            finish();
            return;
        }
        JniInterface.onResume(mNativeApplication, getApplicationContext());
        mSurfaceView.onResume();

        // Listen to display changed events to detect 180 rotation, which does not cause config change or view resize.
        mDisplayRotationManager.registerDisplayListener();
    }

    /**
     * Check whether HUAWEI AR Engine server (com.huawei.arengine.service) is installed on
     * the current device. If not, redirect the user to HUAWEI AppGallery for installation.
     *
     * @return whether HUAWEI AR Engine server is installed.
     */
    private boolean arEngineAbilityCheck() {
        boolean isInstallArEngineApk = JniInterface.isArEngineApkInstalled(mNativeApplication, getApplicationContext());
        if (!isInstallArEngineApk && isRemindInstall) {
            Toast.makeText(this, "Please agree to install.", Toast.LENGTH_LONG).show();
            finish();
        }
        Log.d(TAG, "Is Install AR Engine Apk: " + isInstallArEngineApk);
        if (!isInstallArEngineApk) {
            startActivity(new Intent(this, ConnectAppMarketActivity.class));
            isRemindInstall = true;
        }
        return JniInterface.isArEngineApkInstalled(mNativeApplication, getApplicationContext());
    }

    @Override
    public void onPause() {
        super.onPause();
        mSurfaceView.onPause();
        JniInterface.onPause(mNativeApplication);
        mDisplayRotationManager.unregisterDisplayListener();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        // Synchronized to avoid racing onDrawFrame.
        synchronized (this) {
            JniInterface.destroyNativeApplication(mNativeApplication);
            mNativeApplication = 0;
        }
    }

    @Override
    public void onWindowFocusChanged(boolean isHasFocus) {
        super.onWindowFocusChanged(isHasFocus);
        if (isHasFocus) {
            // Standard Android full-screen functionality.
            getWindow().getDecorView()
                .setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] results) {
        if (!PermissionManager.hasPermission(this)) {
            Toast.makeText(this, "This application needs camera permission.", Toast.LENGTH_LONG).show();
            finish();
        }
    }
}