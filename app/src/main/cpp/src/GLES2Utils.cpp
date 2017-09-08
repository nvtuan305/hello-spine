#include <GLES2Utils.h>
#include <android/log.h>
#include <stdlib.h>
#include <stb/stb_image.h>

#define LOG_TAG "GLES2Utils"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

/*
 * Check OpenGL error after call {functionName}
 */
bool checkGlError(const char *functionName) {
    GLint error = glGetError();
    if (error != GL_NO_ERROR) {
        LOGE("GLES error after %s: 0x%08x\n", functionName, error);
        return true;
    }
    return false;
}

/*
 * Check compiling shader error
 */
bool checkCompileShaderError(GLenum shaderType, GLuint shaderHandle) {
    GLint compiled = GL_FALSE;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLogLen);

        if (infoLogLen > 0) {
            GLchar *infoLog = (GLchar *) malloc(infoLogLen);
            glGetShaderInfoLog(shaderHandle, infoLogLen, NULL, infoLog);
            LOGE("Could not compile %s shader: \n%s\n",
                 shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT",
                 infoLog);
            free(infoLog);
        }
        return true;
    }
    return false;
}

/*
 * Create a shader
 */
GLuint loadShader(GLenum shaderType, const char *src) {
    GLuint shaderHandle = glCreateShader(shaderType);
    // Check create shader error
    if (!shaderHandle) {
        checkGlError("glCreateShader");
        return 0;
    }

    glShaderSource(shaderHandle, 1, &src, NULL);
    glCompileShader(shaderHandle);

    if (checkCompileShaderError(shaderType, shaderHandle)) {
        glDeleteShader(shaderHandle);
        return 0;
    }

    return shaderHandle;
}

/**
 * Load a image texture
 *
 * @param imagePath image file path
 * @return texture handle
 */
GLuint loadTexture(const char *imagePath) {
    GLuint textureHandle = 0;
    glGenTextures(1, &textureHandle);
    checkGlError("glGenTextures - Gen a new texture");

    if (textureHandle != 0) {
        glBindTexture(GL_TEXTURE_2D, textureHandle);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrchanel;
        unsigned char *image = stbi_load(imagePath, &width, &height, &nrchanel, STBI_rgb_alpha);
        if (image) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            LOGD("stbi_load image SUCCESSFUL.....");
        } else {
            LOGE("stbi_load image FAILED.....");
        }

        checkGlError("glTexImage2D - Gen a new texture");
        stbi_image_free(image);
    }

    if (textureHandle == 0) {
        LOGE("Load texture error");
    }

    LOGD("Load texture successful: %s", imagePath);
    return textureHandle;
}

/**
 * Load a solid texture
 *
 * @param rgba the solid color with RGBA type, value of R-G-B-A in [0-255]
 * @return handle to texture
 */
GLuint loadTextureColor(GLubyte rgba[]) {
    GLuint textureHandle[1];
    glGenTextures(1, &textureHandle[0]);
    checkGlError("glGenTextures - Gen solid color texture");

    if (textureHandle[0] != 0) {
        glBindTexture(GL_TEXTURE_2D, textureHandle[0]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        checkGlError("glTexImage2D - Gen solid color texture");
    }

    if (textureHandle[0] == 0) {
        LOGE("Load solid color texture error");
    }

    return textureHandle[0];
}

GLuint createProgram(const char *vertexShaderCode, const char *fragShaderCode) {
    GLuint vtxShaderHandle = loadShader(GL_VERTEX_SHADER, vertexShaderCode);
    GLuint fragShaderHandle = loadShader(GL_FRAGMENT_SHADER, fragShaderCode);

    if (vtxShaderHandle == 0 || fragShaderHandle == 0) {
        return 0;
    }

    LOGD("Compiling shader code: SUCCESSFUL...................");

    GLuint program = glCreateProgram();
    if (!program) {
        checkGlError("glCreateProgram");
        glDeleteShader(vtxShaderHandle);
        glDeleteShader(fragShaderHandle);
        return 0;
    }

    glAttachShader(program, vtxShaderHandle);
    glAttachShader(program, fragShaderHandle);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        LOGE("Could not link program");
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen) {
            GLchar *infoLog = (GLchar *) malloc(infoLen);
            glGetProgramInfoLog(program, infoLen, NULL, infoLog);
            LOGE("Could not link program: \n%s\n", infoLog);
            free(infoLog);
        }

        glDeleteProgram(program);
        return 0;
    }

    LOGD("Linking program: SUCCESSFUL...................");
    return program;
}