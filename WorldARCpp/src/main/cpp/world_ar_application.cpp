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

#include "world_ar_application.h"

#include <array>

#include <android/asset_manager.h>
#include <jni.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

#include <rendering/world_render_manager.h>
#include "utils/util.h"

namespace gWorldAr {
    namespace {
        constexpr size_t K_MAX_NUMBER_OF_OBJECT_RENDERED = 10;
    }

    WorldArApplication::WorldArApplication(AAssetManager *assetManager) : mAssetManager(assetManager)
    {
        LOGI("WorldArApplication::OnCreate()");
    }

    WorldArApplication::~WorldArApplication()
    {
        if (mArSession != nullptr) {
            HwArSession_destroy(mArSession);
            HwArFrame_destroy(mArFrame);
        }
    }

    void WorldArApplication::OnPause()
    {
        LOGI("WorldArApplication::OnPause()");
        if (mArSession != nullptr) {
            HwArSession_pause(mArSession);
        }
    }

    bool WorldArApplication::IsArEngineApkInstalled(JNIEnv *env, jobject context)
    {
        return HwArEnginesApk_isAREngineApkReady(env, context);
    }

    void WorldArApplication::OnResume(JNIEnv *env, jobject context)
    {
        LOGI("WorldArApplication::OnResume()");
        if (mArSession == nullptr) {
            CHECK(HwArSession_create(env, context, &mArSession) == HWAR_SUCCESS);
            CHECK(mArSession);

            HwArConfig *arConfig = nullptr;
            HwArConfig_create(mArSession, &arConfig);

            CHECK(HwArSession_configure(mArSession, arConfig) == HWAR_SUCCESS);

            HwArConfig_destroy(arConfig);
            HwArFrame_create(mArSession, &mArFrame);
            HwArSession_setDisplayGeometry(mArSession, mDisplayRotation, mWidth, mHeight);
        }

        const HwArStatus status = HwArSession_resume(mArSession);
        CHECK(status == HWAR_SUCCESS);
    }

    void WorldArApplication::OnSurfaceCreated()
    {
        LOGI("WorldArApplication::OnSurfaceCreated()");
        mWorldRenderManager.Initialize(mAssetManager);
    }

    void WorldArApplication::OnDisplayGeometryChanged(int displayRotation,
                                                      int width, int height)
    {
        LOGI("WorldArApplication::OnDisplayGeometryChanged(%d, %d)", width, height);
        glViewport(0, 0, width, height);
        mDisplayRotation = displayRotation;
        mWidth = width;
        mHeight = height;
        if (mArSession != nullptr) {
            HwArSession_setDisplayGeometry(mArSession, displayRotation, width, height);
        }
    }

    void WorldArApplication::OnDrawFrame()
    {
        LOGI("WorldArApplication::OnDrawFrame()");
        mWorldRenderManager.OnDrawFrame(mArSession, mArFrame, mColoredAnchors);
    }

    bool WorldArApplication::GetHitResult(HwArHitResult *&arHitResult, bool &hasHitFlag,
                                          int32_t hitResultListSize, HwArTrackableType &trackableType,
                                          HwArHitResultList *hitResultList) const
    {
        for (int32_t i = 0; i < hitResultListSize; ++i) {
            HwArHitResult *arHit = nullptr;
            HwArHitResult_create(mArSession, &arHit);
            HwArHitResultList_getItem(mArSession, hitResultList, i, arHit);

            if (arHit == nullptr) {
                return false;
            }

            HwArTrackable *arTrackable = nullptr;
            HwArHitResult_acquireTrackable(mArSession, arHit, &arTrackable);
            HwArTrackableType ar_trackable_type = HWAR_TRACKABLE_NOT_VALID;
            HwArTrackable_getType(mArSession, arTrackable, &ar_trackable_type);

            // If a plane or directional point is encountered, an anchor point is created.
            if (HWAR_TRACKABLE_PLANE == ar_trackable_type) {
                HwArPose *arPose = nullptr;
                HwArPose_create(mArSession, nullptr, &arPose);
                HwArHitResult_getHitPose(mArSession, arHit, arPose);
                int32_t inPolygon = 0;
                HwArPlane *arPlane = HwArAsPlane(arTrackable);
                HwArPlane_isPoseInPolygon(mArSession, arPlane, arPose, &inPolygon);

                // Use the hit pose and camera pose to check whether the hit position comes
                // from the back of the plane.
                // If yes, no anchor needs to be created.
                HwArPose *cameraPose = nullptr;
                HwArPose_create(mArSession, nullptr, &cameraPose);
                HwArCamera *arCamera;
                HwArFrame_acquireCamera(mArSession, mArFrame, &arCamera);
                HwArCamera_getPose(mArSession, arCamera, cameraPose);
                HwArCamera_release(arCamera);
                float normal_distance_to_plane = util::CalculateDistanceToPlane(*mArSession, *arPose, *cameraPose);

                HwArPose_destroy(arPose);
                HwArPose_destroy(cameraPose);
                if (!inPolygon || normal_distance_to_plane < 0) {
                    continue;
                }

                arHitResult = arHit;
                trackableType = ar_trackable_type;
                hasHitFlag = true;
                break;
            } else if (HWAR_TRACKABLE_POINT == ar_trackable_type) {
                HwArPoint *ar_point = HwArAsPoint(arTrackable);
                HwArPointOrientationMode mode;
                HwArPoint_getOrientationMode(mArSession, ar_point, &mode);
                if (HWAR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL == mode) {
                    arHitResult = arHit;
                    trackableType = ar_trackable_type;
                    hasHitFlag = true;
                }
            }
        }
        return true;
    }

    void WorldArApplication::SetAnchorColour(HwArAnchor *anchor, HwArTrackableType trackableType)
    {
        ColoredAnchor coloredAnchor{};
        coloredAnchor.anchor = anchor;
        switch (trackableType) {
            case HWAR_TRACKABLE_POINT:
                // Set the anchor color when the anchor is generated due to click on the point cloud.
                SetColor(66.0f, 133.0f, 244.0f, 255.0f, coloredAnchor);
                break;
            case HWAR_TRACKABLE_PLANE:
                // Set the anchor color when the anchor is generated due to click on the point cloud.
                SetColor(139.0f, 195.0f, 74.0f, 255.0f, coloredAnchor);
                break;
            default:
                // The virtual object is not displayed if it is not generated by click on the point cloud or plane.
                SetColor(0.0f, 0.0f, 0.0f, 0.0f, coloredAnchor);
                break;
        }
        mColoredAnchors.push_back(coloredAnchor);
    }

    bool WorldArApplication::HasDetectedPlanes()
    {
        return mWorldRenderManager.HasDetectedPlanes();
    }

    void WorldArApplication::OnTouched(float eventX, float eventY)
    {
        LOGI("WorldArApplication::OnTouched()");
        if (mArFrame != nullptr && mArSession != nullptr) {
            HwArHitResultList *hitResultList = nullptr;
            HwArHitResultList_create(mArSession, &hitResultList);
            CHECK(hitResultList);
            HwArFrame_hitTest(mArSession, mArFrame, eventX, eventY, hitResultList);

            int32_t hitResultListSize = 0;
            HwArHitResultList_getSize(mArSession, hitResultList, &hitResultListSize);

            // The hitTest method sorts the result list by the distance to the camera in ascending order.
            // When responding to user input, the first hit result is usually most relevant.
            HwArHitResult *arHitResult = nullptr;
            HwArTrackableType trackableType = HWAR_TRACKABLE_NOT_VALID;
            bool hasHitFlag = false;

            if (!GetHitResult(arHitResult, hasHitFlag, hitResultListSize, trackableType, hitResultList)) {
                return;
            }
            if (hasHitFlag != true) {
                return;
            }
            if (arHitResult) {
                // Note that the app should release the anchor pointer after using it.
                // Call ArAnchor_release(anchor) to release the anchor.
                HwArAnchor *anchor = nullptr;
                if (HwArHitResult_acquireNewAnchor(mArSession, arHitResult, &anchor) != HWAR_SUCCESS) {
                    LOGE("WorldArApplication::OnTouched ArHitResult_acquireNewAnchor error");
                    return;
                }
                HwArTrackingState trackingState = HWAR_TRACKING_STATE_STOPPED;
                HwArAnchor_getTrackingState(mArSession, anchor, &trackingState);
                if (trackingState != HWAR_TRACKING_STATE_TRACKING) {
                    HwArAnchor_release(anchor);
                    return;
                }
                if (mColoredAnchors.size() >= K_MAX_NUMBER_OF_OBJECT_RENDERED) {
                    HwArAnchor_detach(mArSession, mColoredAnchors[0].anchor);
                    HwArAnchor_release(mColoredAnchors[0].anchor);
                    mColoredAnchors.erase(mColoredAnchors.begin());
                }
                SetAnchorColour(anchor, trackableType);
                HwArHitResult_destroy(arHitResult);
                arHitResult = nullptr;

                HwArHitResultList_destroy(hitResultList);
                hitResultList = nullptr;
            }
        }
    }

    void WorldArApplication::SetColor(float colorR, float colorG, float colorB,
        float colorA, ColoredAnchor &coloredAnchor)
    {
        // Set the color.
        *(coloredAnchor.color) = colorR;
        *(coloredAnchor.color + 1) = colorG;
        *(coloredAnchor.color + 2) = colorB;
        *(coloredAnchor.color + 3) = colorA;
    }
}