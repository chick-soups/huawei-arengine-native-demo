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

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.IOException;

/**
 * JNI interface to native layer.
 *
 * @author HW
 * @since 2020-09-21
 */
public class JniInterface {
    private static final String TAG = JniInterface.class.getSimpleName();

    /**
     * Asset administrator.
     */
    private static AssetManager mAssetManager;

    static {
        System.loadLibrary("worldAr_native");
    }

    private JniInterface() {
    }

    /**
     * Set asset management.
     *
     * @param assetManager AssetManager
     */
    public static void setAssetManager(@NonNull AssetManager assetManager) {
        mAssetManager = assetManager;
    }

    /**
     * Create a native app.
     *
     * @param assetManager Asset manager.
     * @return A native pointer to a native app instance.
     */
    public static native long createNativeApplication(AssetManager assetManager);

    /**
     * Destroy the local app.
     *
     * @param nativeApplication Native application.
     */
    public static native void destroyNativeApplication(long nativeApplication);

    /**
     * On pause.
     *
     * @param nativeApplication Native application.
     */
    public static native void onPause(long nativeApplication);

    /**
     * On resume.
     *
     * @param nativeApplication Native application.
     * @param context Context.
     */
    public static native void onResume(long nativeApplication, Context context);

    /**
     * Check whether the AR Engine server has been installed on the current device.
     *
     * @param nativeApplication Native application.
     * @param context Context.
     * @return Check Result.
     */
    public static native boolean isArEngineApkInstalled(long nativeApplication, Context context);

    /**
     * Allocate OpenGL resources for rendering.
     *
     * @param nativeApplication Native application.
     */
    public static native void onGlSurfaceCreated(long nativeApplication);

    /**
     * Called on the OpenGL thread before onGlSurfaceDrawFrame when the view port width, height, or
     * display rotation may have changed.
     *
     * @param nativeApplication NativeApplication.
     * @param displayRotation DisplayRotation.
     * @param width Width.
     * @param height Height.
     */
    public static native void onDisplayGeometryChanged(long nativeApplication, int displayRotation, int width,
        int height);

    /**
     * Main render loop, called on the OpenGL thread.
     *
     * @param nativeApplication Native application.
     */
    public static native void onGlSurfaceDrawFrame(long nativeApplication);

    /**
     * OnTouch event, which is called on the OpenGL thread.
     *
     * @param nativeApplication Native application
     * @param eventX Contact point X-axis value
     * @param eventY Contact point Y-axis value
     */
    public static native void onTouched(long nativeApplication, float eventX, float eventY);

    /**
     * Obtain the number of planes in the current session. Used to disable the "searching for surfaces" snackbar.
     *
     * @param nativeApplication Native application
     * @return Returns whether a plane is detected
     */
    public static native boolean hasDetectedPlanes(long nativeApplication);

    /**
     * Load image.
     *
     * @param imageName Image name.
     * @return Bitmap.
     */
    @UsedByWorldNative(value = "util.cpp")
    public static Bitmap loadImage(String imageName) {
        if (mAssetManager == null) {
            Log.e(TAG, "mAssetManager is null!");
            return null;
        }
        try {
            return BitmapFactory.decodeStream(mAssetManager.open(imageName));
        } catch (IOException e) {
            Log.e(TAG, "Cannot open image " + imageName);
            return null;
        }
    }

    /**
     * Load texture.
     *
     * @param target Target.
     * @param bitmap Bitmap.
     */
    @UsedByWorldNative(value = "util.cpp")
    public static void loadTexture(int target, Bitmap bitmap) {
        if (bitmap == null) {
            Log.e(TAG, "bitmap is null ");
            return;
        }
        GLUtils.texImage2D(target, 0, bitmap, 0);
    }
}