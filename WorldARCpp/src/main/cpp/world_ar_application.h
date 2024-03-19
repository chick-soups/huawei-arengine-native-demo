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

#ifndef C_ARENGINE_HELLOE_AR_HELLO_AR_APPLICATION_H
#define C_ARENGINE_HELLOE_AR_HELLO_AR_APPLICATION_H

#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <jni.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

#include "huawei_arengine_interface.h"
#include "rendering/world_background_renderer.h"
#include "rendering/world_point_cloud_renderer.h"
#include "rendering/world_render_manager.h"
#include "utils/util.h"

namespace gWorldAr {
    class WorldArApplication {
    public:
        WorldArApplication() = default;

        explicit WorldArApplication(AAssetManager *assetManager);

        ~WorldArApplication();

        /**
         * Call the onPause method of the activity in the UI thread.
         */
        void OnPause();

        /**
         * Check whether the AR Engine server has been installed on the current device.
         */
        static bool IsArEngineApkInstalled(JNIEnv *env, jobject context);

        /**
         * Call the OnResume method of the activity on the UI thread.
         */
        void OnResume(JNIEnv *env, jobject context);

        /**
         * When creating GLSsurfaceView, call OnSurfaceCreated on the OpenGL thread.
         */
        void OnSurfaceCreated();

        /**
         * Call OnDisplayGeometryChanged on the OpenGL thread when the rendering plane size changes
         * or the display rotates.
         *
         * @param displayRotation: Current display rotation.
         * @param width: Change the width of the plane view.
         * @param height: Change the height of the plane view.
         */
        void OnDisplayGeometryChanged(int displayRotation, int width, int height);

        /**
         * Call OnDrawFrame on the OpenGL thread to render the next frame.
         */
        void OnDrawFrame();

        /**
         * Call OnTouched on the OpenGL thread after the user taps the screen.
         *
         * @param x: Position of x (pixel).
         * @param y: Position of y (pixel).
         */
        void OnTouched(float eventX, float eventY);

        /**
         * If any plane is detected, true is returned.
         */
        bool HasDetectedPlanes();

    private:
        HwArSession *mArSession = nullptr;
        HwArFrame *mArFrame = nullptr;

        std::vector<ColoredAnchor> mColoredAnchors = {};
        int mWidth = 1;
        int mHeight = 1;
        int mDisplayRotation = 0;
        AAssetManager * const mAssetManager = nullptr;

        WorldRenderManager mWorldRenderManager = gWorldAr::WorldRenderManager();

        static void SetColor(float colorR, float colorG, float colorB, float colorA, ColoredAnchor &coloredAnchor);

        bool GetHitResult(HwArHitResult *&arHitResult,
                          bool &hasHitFlag,
                          int32_t hitResultListSize,
                          HwArTrackableType &trackableType,
                          HwArHitResultList *hitResultList) const;

        void SetAnchorColour(HwArAnchor *anchor, HwArTrackableType trackableType);
    };
}
#endif