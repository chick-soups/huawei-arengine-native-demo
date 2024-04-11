#include <jni.h>
#include <string>
#include "huawei_arengine_interface.h"
#include <android/log.h>
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#ifndef LOGI
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"selfdemo",__VA_ARGS__);
#endif
#ifndef LOGE
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"selfdemo",__VA_ARGS__);
#endif

HwArSession* arSession= nullptr;
HwArFrame* arFrame;


int32_t g_rotation;
int32_t g_width;
int32_t g_height;
bool uvInited;
float uvs[]{
        0,1,
        0,0,
        1,0,
        1,1
};
float tranformedUVs[8];
GLuint uvVBO;


extern "C"
JNIEXPORT void JNICALL
Java_com_example_selfdemo_MainActivity_OnActivityResume(JNIEnv *env, jobject thiz,
                                                        jobject context) {

    HwArSession_resume(arSession);

    // TODO: implement OnActivityResume()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_selfdemo_MainActivity_OnActivityPause(JNIEnv *env, jobject thiz, jobject context) {
    // TODO: implement OnActivityPause()
    if(arSession!= nullptr){
        HwArSession_pause(arSession);
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_selfdemo_MainActivity_OnActivityStop(JNIEnv *env, jobject thiz, jobject context) {
    if(arSession!= nullptr){
        HwArSession_stop(arSession);
        HwArSession_destroy(arSession);
        arSession= nullptr;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_selfdemo_MainActivity_OnActivityStart(JNIEnv *env, jobject thiz, jobject context) {

    bool isReady = HwArEnginesApk_isAREngineApkReady(env,context);
    if(!isReady){
        LOGE("arengine is not readly");
        return;
    }
    if(arSession== nullptr){
        HwArSession_create(env,context,&arSession);
        HwArConfig* arConfig;
        HwArConfig_create(arSession,&arConfig);
        HwArSession_configure(arSession,arConfig);
        HwArConfig_destroy(arConfig);
        HwArFrame_create(arSession,&arFrame);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_selfdemo_MainActivity_OnActivityDestory(JNIEnv *env, jobject thiz,
                                                         jobject context) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_selfdemo_MainActivity_UpdateViewPort(JNIEnv *env, jobject thiz, jint rotation,
                                                      jint width, jint height) {
    g_rotation=rotation;
    g_width=width;
    g_height=height;
    if(arSession!= nullptr){
        HwArSession_setDisplayGeometry(arSession,g_rotation,g_width,g_height);
        uvInited= false;
    }
    glViewport(0,0,width,height);



}

GLuint CompileShader(GLenum shaderType, const char *shaderSource) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length;
        glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&length);
        GLchar info[length];
        glGetShaderInfoLog(shader, length, NULL, info);
        LOGE("fail to compile shader.shader type:%x message:%s", shaderType, info);
        return 0;
    } else {
        return shader;
    }
}

GLuint program,texture,vao;
extern "C"
JNIEXPORT void JNICALL
Java_com_example_selfdemo_MainActivity_OnSurfaceCrteated(JNIEnv *env, jobject thiz) {
    const char *vender = (const char *) glGetString(GL_VENDOR);
    const char *version = (const char *) glGetString(GL_VERSION);
    const char *renderer = (const char *) glGetString(GL_RENDERER);
    const char *extensions = (const char *) glGetString(GL_EXTENSIONS);
    LOGI("vender:%s version:%s renderer:%s,extensions:%s", vender, version, renderer, extensions);
    const char* vertexShaderSource=
            "#version 320 es\n"
            "precision mediump float;\n"
            "layout(location=0) in vec3 position;\n"
            "layout(location=1) in vec2 uv;\n"
            "out vec2 textureCoord;\n"
            "void main()\n"
            "{\n"
            "  gl_Position=vec4(position,1.0);\n"
            "  textureCoord=uv;\n"
            "}\n";
    const char* fragmentShaderSource=
            "#version 320 es\n"
            "#extension GL_OES_EGL_image_external_essl3 : require\n"
            "precision mediump float;\n"
            "in vec2 textureCoord;\n"
            "uniform samplerExternalOES textureOES;\n"
            "out vec4 fragColor;\n"
            "void main()\n"
            "{\n"
            "  fragColor=texture(textureOES,textureCoord);"
            "}\n";

    float vertexes[]{
        -1,-1,0,
        -1,1,0,
        1,1,0,
        1,-1,0
    };
    int32_t indices[]{
        0,1,2,
        0,2,3
    };
    GLuint vbo,ebo;
    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&ebo);
    glGenBuffers(1,&vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertexes),vertexes,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GL_FLOAT)*3,(void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
    glBindVertexArray(0);

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER,vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER,fragmentShaderSource);
    program =  glCreateProgram();
    glAttachShader(program,vertexShader);
    glAttachShader(program,fragmentShader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if(!success){
        GLint length;
        glGetProgramiv(program,GL_INFO_LOG_LENGTH,&length);
        GLchar info[length];
        glGetProgramInfoLog(program,length,NULL,info);
        LOGE("fail to link program.Info:%s",info);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,texture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);
    HwArSession_setCameraTextureName(arSession,texture);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_selfdemo_MainActivity_OnSurfaceDraw(JNIEnv *env, jobject thiz) {

    if(arSession== nullptr){
        return;
    }
    HwArStatus status = HwArSession_update(arSession,arFrame);
    if(status!=HwArStatus::HWAR_SUCCESS){
        LOGI("fail to update ar session.Status:%d",status);
        return;
    }
    HwArCamera* arCamera;
    HwArFrame_acquireCamera(arSession,arFrame,&arCamera);
    HwArTrackingState trackingState;
    HwArCamera_getTrackingState(arSession,arCamera,&trackingState);
    HwArCamera_release(arCamera);
    if(trackingState!=HwArTrackingState::HWAR_TRACKING_STATE_TRACKING){
        LOGI("ar camera is not tracking.");
        return;
    }

    int32_t geometryChanged;
    HwArFrame_getDisplayGeometryChanged(arSession,arFrame,&geometryChanged);
    if(geometryChanged!=0||uvInited== false){
        HwArFrame_transformDisplayUvCoords(arSession,arFrame, std::size(uvs),
                                           uvs,tranformedUVs);
        uvInited=true;
        glDeleteBuffers(1,&uvVBO);
        glBindVertexArray(vao);
        glGenBuffers(1,&uvVBO);
        glBindBuffer(GL_ARRAY_BUFFER,uvVBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(tranformedUVs),tranformedUVs,GL_STATIC_DRAW);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GL_FLOAT)*2,(void *)0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);


    }

    glClearColor(0,0,0,1);
    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(program);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,texture);
    glDrawElements(GL_TRIANGLE_STRIP,6,GL_UNSIGNED_INT,0);
    glUseProgram(0);
    GLenum error = glGetError();
    if(error!=GL_NO_ERROR){
        LOGE("gl error:%x",error);
    }






}