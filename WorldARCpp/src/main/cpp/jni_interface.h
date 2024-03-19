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

#ifndef C_ARENGINE_HELLOE_AR_JNI_INTERFACE_H
#define C_ARENGINE_HELLOE_AR_JNI_INTERFACE_H

#include <jni.h>

extern "C" {
/*
 * Helper function for accessing the JNI environment on the current thread.
 * In this example, the separation of threads upon thread exit is not considered.
 * This can lead to memory leaks,
 * so the production app should be separated when threads no longer need to access the JVM.
 */
JNIEnv *GetJniEnv();

jclass FindClass(const char *classname);
}
#endif
