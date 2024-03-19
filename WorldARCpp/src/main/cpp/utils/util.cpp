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

#include "utils/util.h"

#include <sstream>
#include <string>

#include <unistd.h>

#include "jni_interface.h"

namespace gWorldAr {
    namespace util {
        void CheckGlError(const char *operation)
        {
            for (GLint error = glGetError(); error; error = glGetError()) {
                LOGE("after %s() glError (0x%x)\n", operation, error);
                abort();
            }
        }

        // Convenient features used in the following creation program.
        static GLuint LoadShader(GLenum shader_type, const char *shader_source)
        {
            GLuint shader = glCreateShader(shader_type);
            if (!shader) {
                return shader;
            }

            glShaderSource(shader, 1, &shader_source, nullptr);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

            if (!compiled) {
                GLint infoLen = 0;

                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (!infoLen) {
                    return shader;
                }

                char *buf = reinterpret_cast<char *>(malloc(infoLen));
                if (!buf) {
                    return shader;
                }

                glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                LOGI("Util::LoadShader Could not compile shader %d:\n%s\n", shader_type,
                     buf);
                free(buf);
                glDeleteShader(shader);
                shader = 0;
            }

            return shader;
        }

        static void HandleErrorGlProgramLink(GLint linkStatus, GLuint &outProgram)
        {
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(outProgram, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char *buf = reinterpret_cast<char *>(malloc(bufLength));
                    if (buf) {
                        glGetProgramInfoLog(outProgram, bufLength, nullptr, buf);
                        LOGI("Util::HandleErrorGlProgramLink Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(outProgram);
                outProgram = 0;
            }
        }

        GLuint CreateProgram(const char *vertexSource, const char *fragmentSource)
        {
            GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vertexSource);
            if (!vertexShader) {
                return 0;
            }

            GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragmentSource);
            if (!fragmentShader) {
                return 0;
            }

            GLuint program = glCreateProgram();
            if (program) {
                glAttachShader(program, vertexShader);
                CheckGlError("Util::CreateProgram glAttachShader");
                glAttachShader(program, fragmentShader);
                CheckGlError("Util::CreateProgram glAttachShader");
                glLinkProgram(program);
                GLint linkStatus = GL_FALSE;
                glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

                HandleErrorGlProgramLink(linkStatus, program);
            }
            return program;
        }

        bool LoadPngFromAssetManager(int target, const std::string &path)
        {
            JNIEnv *env = GetJniEnv();
            if (env == nullptr) {
                return false;
            }

            // Place all JNI values in the structure that is statically initialized when the method
            // is first called.
            // This makes it thread-safe when the likelihood of multiple threads calling the method is low.
            static struct JNIData {
                jclass helperClass;
                jmethodID loadImageMethod;
                jmethodID loadTextureMethod;
            } jniIds = [env]() -> JNIData {
                constexpr char kHelperClassName[] =
                    "com/huawei/arengine/demos/cworld/JniInterface";
                constexpr char kLoadImageMethodName[] = "loadImage";
                constexpr char kLoadImageMethodSignature[] =
                    "(Ljava/lang/String;)Landroid/graphics/Bitmap;";
                constexpr char kLoadTextureMethodName[] = "loadTexture";
                constexpr char kLoadTextureMethodSignature[] =
                    "(ILandroid/graphics/Bitmap;)V";
                jclass helperClass = FindClass(kHelperClassName);
                if (helperClass) {
                    helperClass = static_cast<jclass>(env->NewGlobalRef(helperClass));
                    jmethodID loadImageMethod = env->GetStaticMethodID(
                        helperClass, kLoadImageMethodName, kLoadImageMethodSignature);
                    jmethodID loadTextureMethod = env->GetStaticMethodID(
                        helperClass, kLoadTextureMethodName, kLoadTextureMethodSignature);
                    return {helperClass, loadImageMethod, loadTextureMethod};
                }
                LOGI("Util::LoadPngFromAssetManager Could not find Java helper class %s",
                     kHelperClassName);
                return {};
            }();
            if (!jniIds.helperClass) {
                return false;
            }

            jstring jPath = env->NewStringUTF(path.c_str());

            jobject imageObj = env->CallStaticObjectMethod(
                jniIds.helperClass, jniIds.loadImageMethod, jPath);

            if (jPath) {
                env->DeleteLocalRef(jPath);
            }

            env->CallStaticVoidMethod(jniIds.helperClass, jniIds.loadTextureMethod, target, imageObj);
            return true;
        }

        static bool ParseNormal(char lineHeader[], std::vector<GLfloat> &tempNormals)
        {
            // Parses the normal vertex status.
            GLfloat normal[3];
            if (lineHeader[0] != 'v' || lineHeader[1] != 'n') {
                return false;
            }
            std::string tempDataString(lineHeader, lineHeader + strlen(lineHeader));
            std::vector<std::string> stringVector;
            std::string tempResult;
            std::stringstream readData(tempDataString);
            while (readData >> tempResult) {
                stringVector.push_back(tempResult);
            }
            // Obj vn data size is 4, include vn symbol and 3 float coordinate position value.
            if (stringVector.size() < 4) {
                return false;
            }
            // Vertex normal dimension is 3.
            for (int index = 0; index < 3; index++) {
                // 'vn' takes a place.
                normal[index] = atof(stringVector[index + 1].c_str());
                tempNormals.push_back(normal[index]);
            }
            return true;
        }

        static bool ParseTexture(char lineHeader[], std::vector<GLfloat> &tempUvs)
        {
            // Parses texture coordinates.
            GLfloat uv[2];
            if (lineHeader[0] != 'v' || lineHeader[1] != 't') {
                return false;
            }
            std::string tempDataString(lineHeader, lineHeader + strlen(lineHeader));
            std::vector<std::string> stringVector;
            std::string tempResult;
            std::stringstream readData(tempDataString);
            while (readData >> tempResult) {
                stringVector.push_back(tempResult);
            }
            // Obj vt data size is 3, include vt symbol and 2 float coordinate position value.
            if (stringVector.size() < 3) {
                return false;
            }
            // U V two quantities.
            for (int index = 0; index < 2; index++) {
                // 'vt' occupies one place.
                uv[index] = atof(stringVector[index + 1].c_str());
                tempUvs.push_back(uv[index]);
            }
            return true;
        }

        static bool ParseVertex(char lineHeader[], std::vector<GLfloat> &tempPositions)
        {
            // Resolve the vertices.
            GLfloat vertex[3];
            if (lineHeader[0] != 'v' || lineHeader[1] != ' ') {
                return false;
            }
            std::string tempDataString(lineHeader, lineHeader + strlen(lineHeader));
            std::vector<std::string> stringVector;
            std::string tempResult;
            std::stringstream readData(tempDataString);
            while (readData >> tempResult) {
                stringVector.push_back(tempResult);
            }
            // Obj v data size is 4, include v symbol and 3 float coordinate position value.
            if (stringVector.size() < 4) {
                return false;
            }
            // Vertex normal dimension is 3.
            for (int index = 0; index < 3; index++) {
                // 'v' take a position.
                vertex[index] = atof(stringVector[index + 1].c_str());
                tempPositions.push_back(vertex[index]);
            }
            return true;
        }

        static bool WriteOneNormalAndVertValues(FileData fileData,
                                                unsigned int vertexIndex[],
                                                unsigned int normalIndex[],
                                                unsigned int textureIndex[],
                                                bool isNormalAndUvAvailable[])
        {
            // Char* for call system API.
            char *perVertInfo;
            int perVertInforCount = 0;
            bool isVertexNormalOnlyFace = (strstr(fileData.perVertInfoList[fileData.i], "//") != nullptr);
            // Char* for call system API.
            char *perVertInfoIter = fileData.perVertInfoList[fileData.i];
            perVertInfo = strtok_r(perVertInfoIter, "/", &perVertInfoIter);
            bool flag = perVertInfo;
            while (flag) {
                // Write only normal and vertical values.
                switch (perVertInforCount) {
                    case 0: // The vertex index is the first write in turn.
                        // Write the vertex index.
                        vertexIndex[fileData.i] = atoi(perVertInfo);  // NOLINT
                        break;
                    case 1: // Write sequentially, the texture index will be written the second time.
                        // Write the texture index.
                        if (isVertexNormalOnlyFace) {
                            normalIndex[fileData.i] = atoi(perVertInfo);  // NOLINT
                            isNormalAndUvAvailable[0] = true;
                        } else {
                            textureIndex[fileData.i] = atoi(perVertInfo);  // NOLINT
                            isNormalAndUvAvailable[1] = true;
                        }
                        break;
                        // Whether to write a normal index.
                    case 2: // Cyclic write: normal index write for the third time.
                        // Write a common index.
                        if (!isVertexNormalOnlyFace) {
                            normalIndex[fileData.i] = atoi(perVertInfo);  // NOLINT
                            isNormalAndUvAvailable[0] = true;
                            break;
                        }

                        // Deliberately falling into the default error condition due to the vertices,
                        // the normal plane has only two values.
                    default:
                        // The format is incorrect.
                        LOGE("Format of 'f int/int/int int/int/int int/int/int "
                             "(int/int/int)' "
                             "or 'f int//int int//int int//int (int//int)' required for "
                             "each face");
                        return false;
                }
                perVertInforCount++;
                flag = perVertInfo = strtok_r(perVertInfoIter, "/", &perVertInfoIter);
            }
            return true;
        }

        static bool WriteNormalAndVertexValues(std::vector<char *> perVertexInfoList,
                                               unsigned int vertexIndex[],
                                               unsigned int normalIndex[],
                                               unsigned int textureIndex[],
                                               bool isNormalAndUvAvailable[])
        {
            for (int i = 0; i < perVertexInfoList.size(); ++i) {
                FileData fileData;
                fileData.perVertInfoList = perVertexInfoList;
                fileData.i = i;
                if (!WriteOneNormalAndVertValues(
                    fileData, vertexIndex, normalIndex, textureIndex, isNormalAndUvAvailable)) {
                    return false;
                }
            }
            return true;
        }

        static bool WriteIndex(char &input,
                               std::vector<GLushort> &vertexIndices,
                               std::vector<GLushort> &normalIndices,
                               std::vector<GLushort> &uvIndices)
        {
            // The actual face information starts from the second digit.
            unsigned int vertexIndex[4] = {}; // The dimension of the coordinate is 4(xyzw).
            unsigned int normalIndex[4] = {}; // The dimension of normal vector is 4.
            unsigned int textureIndex[4] = {}; // The dimension of texture is 4.

            std::vector<char *> perVertInfoList;
            char *perVertInfoListCstr;
            char *faceLineIter = &input;
            bool flag = perVertInfoListCstr = strtok_r(faceLineIter, " ", &faceLineIter);
            while (flag) {
                // Divide each piece of face information into each position.
                perVertInfoList.push_back(perVertInfoListCstr);
                flag = perVertInfoListCstr = strtok_r(faceLineIter, " ", &faceLineIter);
            }
            bool isNormalAvailable = false;
            bool isUvAvailable = false;
            // Input variable of the WriteNormalAndVertValues method.
            bool isNormalAndUvAvailable[2] = {isNormalAvailable, isUvAvailable};

            if (!WriteNormalAndVertexValues(
                perVertInfoList, vertexIndex, normalIndex, textureIndex, isNormalAndUvAvailable)) {
                return false;
            }
            int verticesCount = perVertInfoList.size();
            for (int i = 2; i < verticesCount; ++i) {
                vertexIndices.push_back(vertexIndex[0] - 1);
                vertexIndices.push_back(vertexIndex[i - 1] - 1);
                vertexIndices.push_back(vertexIndex[i] - 1);
                if (isNormalAndUvAvailable[0]) {
                    normalIndices.push_back(normalIndex[0] - 1);
                    normalIndices.push_back(normalIndex[i - 1] - 1);
                    normalIndices.push_back(normalIndex[i] - 1);
                }
                if (isNormalAndUvAvailable[1]) {
                    uvIndices.push_back(textureIndex[0] - 1);
                    uvIndices.push_back(textureIndex[i - 1] - 1);
                    uvIndices.push_back(textureIndex[i] - 1);
                }
            }
            return true;
        }

        static bool WriteDrawOutData(DrawTempData drawTempData,
                                     std::vector<GLfloat> &outVertices,
                                     std::vector<GLfloat> &outNormals,
                                     std::vector<GLfloat> &outUv,
                                     std::vector<GLushort> &outIndices)
        {
            bool isNormalAvailable = (!drawTempData.normalIndices.empty());
            bool isUvAvailable = (!drawTempData.uvIndices.empty());
            if (isNormalAvailable && drawTempData.normalIndices.size() != drawTempData.vertexIndices.size()) {
                LOGE("Object normal indices does not equal to vertex indices.");
                return false;
            }
            if (isUvAvailable && drawTempData.uvIndices.size() != drawTempData.vertexIndices.size()) {
                LOGE("Object UV indices does not equal to vertex indices.");
                return false;
            }

            for (unsigned int i = 0; i < drawTempData.vertexIndices.size(); i++) {
                unsigned int vertex_index = drawTempData.vertexIndices[i];
                // The vertex dimension is 3.
                outVertices.push_back(drawTempData.tempPositions[vertex_index * 3]);
                outVertices.push_back(drawTempData.tempPositions[vertex_index * 3 + 1]);
                outVertices.push_back(drawTempData.tempPositions[vertex_index * 3 + 2]);
                outIndices.push_back(i);
                if (isNormalAvailable) {
                    unsigned int normal_index = drawTempData.normalIndices[i];
                    // The vertex normal has three dimensions.
                    outNormals.push_back(drawTempData.tempNormals[normal_index * 3]);
                    outNormals.push_back(drawTempData.tempNormals[normal_index * 3 + 1]);
                    outNormals.push_back(drawTempData.tempNormals[normal_index * 3 + 2]);
                }
                if (isUvAvailable) {
                    unsigned int uv_index = drawTempData.uvIndices[i];
                    // U-axis and V-axis of the texture coordinate.
                    outUv.push_back(drawTempData.tempUvs[uv_index * 2]);
                    outUv.push_back(drawTempData.tempUvs[uv_index * 2 + 1]);
                }
            }
            return true;
        }

        bool LoadObjFile(FileInfor fileInfor,
                         std::vector<GLfloat> &outVertices,
                         std::vector<GLfloat> &outNormals,
                         std::vector<GLfloat> &outUv,
                         std::vector<GLushort> &outIndices)
        {
            DrawTempData drawTempData;

            // If the file has not been decompressed, it is loaded into internal storage.
            // Note that AAsset_openFileDescriptor doesn't support compressed files (.obj).
            AAsset *asset = AAssetManager_open(fileInfor.mgr, fileInfor.fileName.c_str(), AASSET_MODE_STREAMING);
            if (asset == nullptr) {
                LOGE("Error opening asset %s", fileInfor.fileName.c_str());
                return false;
            }

            off_t fileSize = AAsset_getLength(asset);
            std::string file_buffer;
            file_buffer.resize(fileSize);
            int ret = AAsset_read(asset, &file_buffer.front(), fileSize);
            if (ret < 0 || ret == EOF) {
                LOGE("Failed to open file: %s", fileInfor.fileName.c_str());
                AAsset_close(asset);
                return false;
            }
            AAsset_close(asset);

            std::stringstream fileStringStream(file_buffer);
            while (!fileStringStream.eof()) {
                // Sets the maximum number of characters to be read (128).
                char line_header[128] = {};
                fileStringStream.getline(line_header, 128);
                if (line_header[0] == 'v' && line_header[1] == 'n') {
                    if (!ParseNormal(line_header, drawTempData.tempNormals)) {
                        return false;
                    }
                } else if (line_header[0] == 'v' && line_header[1] == 't') {
                    if (!ParseTexture(line_header, drawTempData.tempUvs)) {
                        return false;
                    }
                } else if (line_header[0] == 'v') {
                    if (!ParseVertex(line_header, drawTempData.tempPositions)) {
                        return false;
                    }
                } else if (line_header[0] == 'f') {
                    if (!WriteIndex(
                        line_header[1], drawTempData.vertexIndices,
                        drawTempData.normalIndices, drawTempData.uvIndices)) {
                        return false;
                    }
                }
            }
            if (!WriteDrawOutData(drawTempData, outVertices, outNormals, outUv, outIndices)) {
                return false;
            }
            return true;
        }

        void GetTransformMatrixFromAnchor(HwArSession *arSession,
                                          const HwArAnchor *arAnchor,
                                          glm::mat4 *outModelMat)
        {
            if (outModelMat == nullptr) {
                LOGE("Util::GetTransformMatrixFromAnchor model_mat is null.");
                return;
            }
            util::Util pose(arSession);
            HwArAnchor_getPose(arSession, arAnchor, pose.GetArPose());
            HwArPose_getMatrix(arSession, pose.GetArPose(), glm::value_ptr(*outModelMat));
        }

        glm::vec3 GetPlaneNormal(const HwArSession &arSession,
                                 const HwArPose &planePose)
        {
            // Rotation and translation parameters of four elements extracted from the pose object,
            // size of 8.
            float planePoseRaw[7] = {0.f};
            HwArPose_getPoseRaw(&arSession, &planePose, planePoseRaw);
            // The four elements of posture.
            glm::quat plane_quaternion(planePoseRaw[3], planePoseRaw[0],
                                       planePoseRaw[1], planePoseRaw[2]);

            // Obtains the normal vector, which is defined as the positive Y position in the local frame.
            return glm::rotate(plane_quaternion, glm::vec3(0., 1.f, 0.));
        }

        float CalculateDistanceToPlane(const HwArSession &arSession,
                                       const HwArPose &planePose,
                                       const HwArPose &cameraPose)
        {
            // Rotation and translation parameters of the four elements extracted from the pose object.
            float planePoseRaw[7] = {
                0.f
            };
            HwArPose_getPoseRaw(&arSession, &planePose, planePoseRaw);
            glm::vec3 planePosition(planePoseRaw[4], planePoseRaw[5],
                                    planePoseRaw[6]);
            glm::vec3 normal = GetPlaneNormal(arSession, planePose);

            // The last three elements are the conversion values.
            float cameraPoseRaw[7] = {0.f};
            HwArPose_getPoseRaw(&arSession, &cameraPose, cameraPoseRaw);
            glm::vec3 camera_P_plane(cameraPoseRaw[4] - planePosition.x,
                                     cameraPoseRaw[5] - planePosition.y,
                                     cameraPoseRaw[6] - planePosition.z);
            return glm::dot(normal, camera_P_plane);
        }
    }
}