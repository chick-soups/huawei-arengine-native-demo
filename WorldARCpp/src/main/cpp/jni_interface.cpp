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

#include "jni_interface.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>

#include "world_ar_application.h"

extern "C" {
namespace {
    // Maintain the reference to the JVM so that we can use it later.
    static JavaVM *g_vm = nullptr;

    inline jlong Jptr(gWorldAr::WorldArApplication *worldArApplication)
    {
        return reinterpret_cast<intptr_t>(worldArApplication);
    }

    inline gWorldAr::WorldArApplication *Native(jlong ptr)
    {
        return reinterpret_cast<gWorldAr::WorldArApplication *>(static_cast<uintptr_t>(ptr));
    }
}

jint JNI_OnLoad(JavaVM *vm, void *)
{
    g_vm = vm;
    return JNI_VERSION_1_6;
}

// Minimum version.
#if __ANDROID_API__ >= 26

#endif

JNIEXPORT jlong JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_createNativeApplication(
    JNIEnv *env, jclass, jobject javaAssetManager)
{
    AAssetManager *assetManager = AAssetManager_fromJava(env, javaAssetManager);
    return Jptr(new gWorldAr::WorldArApplication(assetManager));
}

JNIEXPORT void JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_destroyNativeApplication(
    JNIEnv *env, jclass, jlong nativeApplication)
{
    delete Native(nativeApplication);
}

JNIEXPORT void JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_onPause(
    JNIEnv *, jclass, jlong nativeApplication)
{
    Native(nativeApplication)->OnPause();
}

JNIEXPORT void JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_onResume(
    JNIEnv *env, jclass, jlong nativeApplication, jobject context)
{
    Native(nativeApplication)->OnResume(env, context);
}

JNIEXPORT jboolean JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_isArEngineApkInstalled(
    JNIEnv *env, jclass, jlong nativeApplication, jobject context)
{
    return jboolean(Native(nativeApplication)->IsArEngineApkInstalled(env, context));
}

JNIEXPORT void JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_onGlSurfaceCreated(
    JNIEnv *, jclass, jlong nativeApplication)
{
    Native(nativeApplication)->OnSurfaceCreated();
}

JNIEXPORT void JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_onDisplayGeometryChanged(
    JNIEnv *, jclass, jlong nativeApplication, int displayRotation, int width, int height)
{
    Native(nativeApplication)->OnDisplayGeometryChanged(displayRotation, width, height);
}

JNIEXPORT void JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_onGlSurfaceDrawFrame(
    JNIEnv *, jclass, jlong nativeApplication)
{
    Native(nativeApplication)->OnDrawFrame();
}

JNIEXPORT void JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_onTouched(
    JNIEnv *, jclass, jlong nativeApplication, jfloat eventX, jfloat eventY)
{
    Native(nativeApplication)->OnTouched(eventX, eventY);
}

JNIEXPORT jboolean JNICALL Java_com_huawei_arengine_demos_cworld_JniInterface_hasDetectedPlanes(
    JNIEnv *, jclass, jlong nativeApplication)
{
    return static_cast<jboolean>(Native(nativeApplication)->HasDetectedPlanes() ? JNI_TRUE : JNI_FALSE);
}

JNIEnv *GetJniEnv()
{
    JNIEnv *env = nullptr;
    jint result = g_vm->AttachCurrentThread(&env, nullptr);
    return (result == JNI_OK) ? env : nullptr;
}

jclass FindClass(const char *classname)
{
    JNIEnv *env = GetJniEnv();
    if (env == nullptr) {
        LOGE("jni_interface::FindClass getJniEnv failed.");
        return nullptr;
    }
    return env->FindClass(classname);
}
}
