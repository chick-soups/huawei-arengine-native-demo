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

#ifndef C_ARENGINE_WORLD_AR_BACKGROUND_RENDERER_H
#define C_ARENGINE_WORLD_AR_BACKGROUND_RENDERER_H

#include <cstdlib>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "utils/util.h"

namespace gWorldAr {
    class WorldBackgroundRenderer {
    public:
        WorldBackgroundRenderer() = default;

        ~WorldBackgroundRenderer() = default;

        /**
         * Initialize the OpenGL status. This method must be called in the OpenGL thread,
         * and other methods must be called before the methods below.
         */
        void InitializeBackGroundGlContent();

        /**
         * Draw a background image.
         * This method must be called to capture the event of geometric changes.
         *
         * @param session This API is used to query the sessions in the background drawing.
         * @param fram Information about each frame during background drawing.
         */
        void Draw(const HwArSession *session, const HwArFrame *frame);

        /**
         * Obtain the texture ID.
         *
         * @return Name of the texture generated by the GL_TEXTURE_EXTERNAL_OES target.
         */
        GLuint GetTextureId() const;

    private:
        const static int VERTICES_NUM = 4; // Number of vertices.

        GLuint shaderProgram = 0;
        GLuint textureId = 0;
        GLuint attributeVertices = 0;
        GLuint attributeUvs = 0;
        GLuint uniformTexture = 0;

        // In OpenGLES, the texture coordinate dimension is 2.
        float transformedUvs[VERTICES_NUM * 2] = {};
        bool uvsInitialized = false;
    };
}
#endif  // TANGO_GL_VIDEO_OVERLAY_H
