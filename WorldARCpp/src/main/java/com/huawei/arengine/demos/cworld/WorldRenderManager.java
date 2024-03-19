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
import android.graphics.Color;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * This class provides rendering management related to the world scene, including
 * label rendering and virtual object rendering management.
 *
 * @author HW
 * @since 2020-09-21
 */
public class WorldRenderManager implements GLSurfaceView.Renderer {
    private static final float GL_CLEAR_COLOR_RED = 0.1f;

    private static final float GL_CLEAR_COLOR_GREEN = 0.1f;

    private static final float GL_CLEAR_COLOR_BLUE = 0.1f;

    private static final float GL_CLEAR_COLOR_ALPHA = 1.0f;

    private TextDisplay mTextDisplay = new TextDisplay();

    private Activity mActivity;

    private TextView mTextView;

    private TextView mSearchingTextView;

    private int frames = 0;

    private long lastInterval;

    private float fps;

    private long mNativeApplication;

    private DisplayRotationManager mDisplayRotationManager;

    /**
     * The constructor passes context and activity. This method will be called when {@link Activity#onCreate}.
     *
     * @param activity Activity
     */
    public WorldRenderManager(Activity activity) {
        mActivity = activity;
        mTextView = activity.findViewById(R.id.textView);
        mSearchingTextView = activity.findViewById(R.id.searchingTextView);
    }

    /**
     * Set the native handler at the Java layer, which is used for interactions with native resources.
     *
     * @param nativeApplication nativeApplication handler.
     */
    public void setNativeApplication(long nativeApplication) {
        mNativeApplication = nativeApplication;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(GL_CLEAR_COLOR_RED, GL_CLEAR_COLOR_GREEN, GL_CLEAR_COLOR_BLUE, GL_CLEAR_COLOR_ALPHA);
        JniInterface.onGlSurfaceCreated(mNativeApplication);
        mTextDisplay.setListener(this::showWorldTypeTextView);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        mDisplayRotationManager.updateViewportRotation(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        // Synchronized to avoid racing onDestroy.
        synchronized (this) {
            if (mNativeApplication == 0) {
                return;
            }
            if (mDisplayRotationManager.getDeviceRotation()) {
                mDisplayRotationManager.updateArSessionDisplayGeometry(mNativeApplication);
            }

            if (JniInterface.hasDetectedPlanes(mNativeApplication)) {
                hideLoadingMessage();
            }

            StringBuilder screenText = new StringBuilder();
            updateMessageData(screenText);
            mTextDisplay.onDrawFrame(screenText);
            JniInterface.onGlSurfaceDrawFrame(mNativeApplication);
        }
    }

    /**
     * Update the information to be displayed on the screen.
     *
     * @param screenText String buffer.
     */
    private void updateMessageData(StringBuilder screenText) {
        float fpsResult = doFpsCalculate();
        screenText.append("FPS=").append(fpsResult).append(System.lineSeparator());
    }

    /**
     * Set the DisplayRotationManage object, which will be used in onSurfaceChanged and onDrawFrame.
     *
     * @param displayRotationManager DisplayRotationManage is customized object.
     */
    public void setDisplayRotationManage(@NonNull DisplayRotationManager displayRotationManager) {
        mDisplayRotationManager = displayRotationManager;
    }

    /**
     * Create a thread for text display in the UI thread. This thread will be called back in TextureDisplay.
     *
     * @param text Gesture information displayed on the screen.
     * @param positionX The left padding in pixels.
     * @param positionY The right padding in pixels.
     */
    private void showWorldTypeTextView(final String text, final float positionX, final float positionY) {
        mActivity.runOnUiThread(() -> {
            mTextView.setTextColor(Color.WHITE);

            // Set the font size to be displayed on the screen.
            mTextView.setTextSize(10f);
            if (text != null) {
                mTextView.setText(text);
                mTextView.setPadding((int) positionX, (int) positionY, 0, 0);
            } else {
                mTextView.setText("");
            }
        });
    }

    private float doFpsCalculate() {
        ++frames;
        long timeNow = System.currentTimeMillis();

        // Convert millisecond to second.
        if (((timeNow - lastInterval) / 1000.0f) > 0.5f) {
            fps = frames / ((timeNow - lastInterval) / 1000.0f);
            frames = 0;
            lastInterval = timeNow;
        }
        return fps;
    }

    private void hideLoadingMessage() {
        mActivity.runOnUiThread(() -> {
            if (mSearchingTextView != null) {
                mSearchingTextView.setVisibility(View.GONE);
                mSearchingTextView = null;
            }
        });
    }
}