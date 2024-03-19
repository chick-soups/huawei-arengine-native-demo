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

#ifndef C_ARENGINE_HELLOE_AR_UTIL_H
#define C_ARENGINE_HELLOE_AR_UTIL_H

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include <android/asset_manager.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

#include "huawei_arengine_interface.h"

#ifndef LOGI
#define LOGI(...) \
    __android_log_print(ANDROID_LOG_INFO, "worldArCPP", __VA_ARGS__)
#endif

#ifndef LOGE
#define LOGE(...) \
    __android_log_print(ANDROID_LOG_ERROR, "worldArCPP", __VA_ARGS__)
#endif

#ifndef CHECK
#define CHECK(condition)                                                   \
    if (!(condition)) {                                                      \
        LOGE("*** CHECK FAILED at %d: %s", __LINE__, #condition); \
        abort();                                                               \
    }
#endif

namespace gWorldAr {
    // Utilities for C hello AR project.
    namespace util {
        class Util {
        public:
            explicit Util(const HwArSession *session)
            {
                HwArPose_create(session, nullptr, &pose);
            }

            ~Util()
            {
                HwArPose_destroy(pose);
            }

            HwArPose *GetArPose() const
            {
                return pose;
            }

            // Delete copy constructors.
            Util(const Util &) = delete;

            void operator=(const Util &) = delete;

        private:
            HwArPose *pose = nullptr;
        };

        using FileInfor = struct {
            AAssetManager *mgr;
            std::string fileName;
        };

        using FileData = struct {
            std::vector<char *> perVertInfoList;
            int i;
        };

        using DrawTempData = struct {
            std::vector<GLfloat> tempPositions;
            std::vector<GLfloat> tempNormals;
            std::vector<GLfloat> tempUvs;
            std::vector<GLushort> vertexIndices;
            std::vector<GLushort> normalIndices;
            std::vector<GLushort> uvIndices;
        };

        /**
         * Checks for GL errors and stops if any.
         *
         * @param operation Name of the GL function call.
         */
        void CheckGlError(const char *operation);

        /**
         * Create Shader Program ID.
         *
         * @param vertexSource Vertex source, vertex coloring source.
         * @param fragmentSource Fragment Source, Fragment Shader Source.
         * @return GL Status Value.
         */
        GLuint CreateProgram(const char *vertexSource, const char *fragmentSource);

        void GetTransformMatrixFromAnchor(HwArSession *arSession,
                                          const HwArAnchor *arAnchor,
                                          glm::mat4 *outModelMat);

        glm::vec3 GetPlaneNormal(const HwArSession &arSession, const HwArPose &planePose);

        /**
         * Calculate the normal distance from the camera to the plane.
         * The y-axis of a given plane should be parallel to the normal of the plane.
         * Such as a center position of a plane or a hit test position.
         */
        float CalculateDistanceToPlane(const HwArSession &arSession,
                                       const HwArPose &planePose,
                                       const HwArPose &cameraPose);

        bool LoadPngFromAssetManager(int target, const std::string &path);

        /**
         * Load the obj file from the assets folder in the application.
         *
         * @param fileInformation Pointer to the AAssetManager,the name of the obj file.
         * @param outVertices Output vertex.
         * @param outNormals Output normal.
         * @param outUv UV coordinate of the output texture.
         * @param outIndices Output triangular exponent.
         * @return True if obj is loaded correctly, false otherwise.
         */
        bool LoadObjFile(FileInfor fileInformation,
                         std::vector<GLfloat> &outVertices,
                         std::vector<GLfloat> &outNormals,
                         std::vector<GLfloat> &outUv,
                         std::vector<GLushort> &outIndices);
    }
}
#endif