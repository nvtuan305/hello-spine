#include <Sticker.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <android/log.h>
#include <utils/TimeUtils.h>
#include <utils/StringUtils.h>
#include <utils/GLES2Utils.h>

#define LOG_TAG "STICKER_CPP"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

const char vertexShaderCode[] =
        "attribute vec2 a_Position;"
                "uniform mat4 u_MVPMatrix;"
                "attribute vec2 a_TexCoords;"
                "varying vec2 v_TexCoords;"
                "attribute vec4 a_Color;"
                "varying vec4 v_Color;"
                "void main() {"
                "    v_TexCoords = a_TexCoords;"
                "    v_Color = a_Color;"
                "    gl_Position = u_MVPMatrix * vec4(a_Position, 0.0f, 1.0f);"
                "}";

const char fragShaderCode[] =
        "precision mediump float;"
                "varying vec4 v_Color;"
                "varying vec2 v_TexCoords;"
                "uniform sampler2D u_Texture;"
                "void main() {"
                "    gl_FragColor = texture2D(u_Texture, v_TexCoords) * v_Color;"
                "}";

void _spAtlasPage_createTexture(spAtlasPage *self, const char *path) {

}

void _spAtlasPage_disposeTexture(spAtlasPage *self) {

}

char *_spUtil_readFile(const char *path, int *length) {
    return _spReadFile(path, length);
}

Sticker::Sticker(const char *atlasPath, const char *jsonPath, const char *imagePath,
                 const char *defaultAnimation) {
    mEglContext = eglGetCurrentContext();
    mCountPerVertex = 2; // (x, y)
    mCountPerTexCoord = 2; // (x, y)
    mCountPerColor = 4; // RGBA

    mProgram = 0;
    mPositionHandle = 0;
    mColorHandle = 0;
    mMvpMatrixHandle = 0;
    mTexDataHandle = 0;
    mTexCoordsHandle = 0;
    mTexSampler2DHandle = 0;

    mAngle = 0.0f;
    mTrans = vec3(0.0f, 0.0f, 0.0f);

    mAtlasPath = getString(atlasPath);
    mJsonPath = getString(jsonPath);
    mImagePath = getString(imagePath);
    mDefaultAnimation = getString(defaultAnimation);

    mLastAnimationTime = 0; // Default
    mWorldVertices = new float[MAX_VERTEX_COUNT];
}

/**
 * Initialize sticker
 */
void Sticker::init() {
    initOpenGL(mImagePath);
    initSpine();
}

/**
 * Initialize OpenGL
 * @param texturePath path of the texture image
 */
void Sticker::initOpenGL(const char *texturePath) {
    // Create program only one time
    if (mProgram == 0) {
        mProgram = createProgram(vertexShaderCode, fragShaderCode);
        if (mProgram == 0) return;
    }

    // Get handle for variables in GPU
    mPositionHandle = (GLuint) glGetAttribLocation(mProgram, "a_Position");
    mColorHandle = (GLuint) glGetAttribLocation(mProgram, "a_Color");
    mTexCoordsHandle = (GLuint) glGetAttribLocation(mProgram, "a_TexCoords");
    mMvpMatrixHandle = (GLuint) glGetUniformLocation(mProgram, "u_MVPMatrix");
    mTexSampler2DHandle = (GLuint) glGetUniformLocation(mProgram, "u_Texture");
    mTexDataHandle = loadTexture(texturePath);

    // Set view matrix
    mViewMatrix = lookAt(vec3(0, 0, 3.0f),
                         vec3(0, 0, 0),
                         vec3(0, 1.0f, 0));

    // Generate new vertex buffer
    glGenBuffers(VB_COUNT, mVB);

    LOGD("Init OpenGL: SUCCESSFUL...................");
}

/**
 * Initialize spine: Skeleton and animation state
 */
void Sticker::initSpine() {
    // Read atlas from atlas file
    mAtlas = spAtlas_createFromFile(mAtlasPath, 0);
    if (!mAtlas) {
        LOGE("Read atlas file: FAILED..........");
        disposeSpineData();
        return;
    }
    LOGD("Read atlas file: SUCCESSFUL..........");

    // Create a skeleton json object from atlas
    spSkeletonJson *json = spSkeletonJson_create(mAtlas);
    // Read skeleton data from json and json path
    mSkeletonData = spSkeletonJson_readSkeletonDataFile(json, mJsonPath);
    if (!mSkeletonData) {
        LOGE("Read skeleton data from json file: FAILED................");
        spSkeletonJson_dispose(json);
        disposeSpineData();
        return;
    }
    // Dispose json object because we don't need it after loading
    spSkeletonJson_dispose(json);
    LOGD("Read skeleton data from json file: SUCCESSFUL..........");

    // Create a skeleton
    mSkeleton = spSkeleton_create(mSkeletonData);
    if (!mSkeleton) {
        LOGE("Create skeleton: FAILED..........");
        disposeSpineData();
        return;
    }
    LOGD("Create skeleton: SUCCESSFUL..........");

    // Create animation state from skeleton data
    mAnimationStateData = spAnimationStateData_create(mSkeletonData);
    if (!mAnimationStateData) {
        LOGE("Create animation state from skeleton data: FAILED..........");
        disposeSpineData();
        return;
    }
    // Mixing time between two animation is 0.5s
    mAnimationStateData->defaultMix = 0.5f;
    LOGD("Create animation state from skeleton data: SUCCESSFUL..........");

    // Create animation state from animation state data
    mAnimationState = spAnimationState_create(mAnimationStateData);
    if (!mAnimationState) {
        LOGE("Create animation state from animation state data: FAILED..........");
        disposeSpineData();
        return;
    }
    // Set default animation
    spAnimationState_setAnimationByName(mAnimationState, 0, mDefaultAnimation, 1);
    LOGD("Create animation state from animation state data: SUCCESSFUL............");

    // Set default position
    mSkeleton->x = 30;
    mSkeleton->y = -400;
    spSkeleton_updateWorldTransform(mSkeleton);

    LOGD("Init Spine: SUCCESSFUL...................");
}

/**
 * Set rotation angle and translation vector
 *
 * @param angle rotation angle
 * @param trans translation vector
 */
void Sticker::setAngleAndTranslation(float angle, vec3 trans) {
    mAngle = angle;
    mTrans = trans;
}

/**
 * Recalculate projection matrix when configuration changed
 *
 * @param width new width of the view
 * @param height new height of the view
 */
void Sticker::resize(int width, int height) {
    glViewport(0, 0, width, height);
    float ratio = (float) width / (float) height;

    // TODO Calculate position of camera (the left/right/top/bottom planes)
    // Default max size of sticker 1400 x 1400
    float maxSize = 1400.0f;
    mProjectionMatrix = ortho(-ratio * maxSize, ratio * maxSize, -maxSize,
                              maxSize, 2.0f, 5.0f);
}

/**
 * Draw sticker at the current time
 */
void Sticker::draw() {
    if (!mSkeleton) {
        LOGE("updateVertexAndTexCoordsData - skeleton is NULL");
        return;
    }

    float deltaTime = calculateDeltaTime() * 1.0f / 1000.0f;

    // Update animation state by delta time
    spAnimationState_update(mAnimationState, deltaTime);
    LOGD("Update animation state at %f seconds", deltaTime);

    // Apply the animation state to skeleton
    spAnimationState_apply(mAnimationState, mSkeleton);
    LOGD("Apply the animation state to skeleton at %f seconds", deltaTime);

    // Calculate world transform for rendering
    spSkeleton_updateWorldTransform(mSkeleton);
    LOGD("Calculate world transform for rendering at %f seconds", deltaTime);

    // Update new vertices and texture coordinate
    LOGD("Updating new vertices and texture coordinate...");
    updateVertexAndTexCoordsData();
}

/**
 * Get the delta time from the current time to the last drawn time
 */
long Sticker::calculateDeltaTime() {
    long currentTime = getCurrentSystemTimeInMilli();
    long deltaTime = this->mLastAnimationTime == 0L ? 0L : currentTime - this->mLastAnimationTime;
    mLastAnimationTime = currentTime;
    return deltaTime;
}

/**
 * Update vextex data and texture coordinate data
 */
void Sticker::updateVertexAndTexCoordsData() {
    if (!mSkeleton) {
        LOGE("updateVertexAndTexCoordsData - skeleton is NULL");
        return;
    }

    // Clear old data
    clearGLData();

    for (int i = 0; i < mSkeleton->slotsCount; ++i) {
        spSlot *slot = mSkeleton->drawOrder[i];
        if (!slot) continue;

        spAttachment *attachment = slot->attachment;
        if (!attachment) continue;
        LOGD("Processing attachment of SLOT[%d]...", i);

        // Update blend mode if necessary
        int blendMode = slot->data->blendMode;
        if (mCurrentBlendMode != blendMode) {
            updateBlendMode(blendMode);
            render();
            clearGLData();
            LOGD("New blend mode: %d", mCurrentBlendMode);
        }

        switch (attachment->type) {
            case SP_ATTACHMENT_REGION:
                updateVertexAndTexCoordsData_fromRegionAttachment(
                        (spRegionAttachment *) attachment, slot);
                break;

            case SP_ATTACHMENT_MESH:
                updateVertexAndTexCoordsData_fromMeshAttachment(
                        (spMeshAttachment *) attachment, slot);
                break;

            default:
                LOGE("Other attachment: %s", attachment->type);
                break;
        }
    }

    render();
}

/**
 * Update new blend mode
 */
void Sticker::updateBlendMode(int newMode) {
    mCurrentBlendMode = newMode;

    switch (mCurrentBlendMode) {
        case SP_BLEND_MODE_ADDITIVE: // Cr = Cs + Cd
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;

        case SP_BLEND_MODE_MULTIPLY: // Cr = Cs * Cd
            glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;

        case SP_BLEND_MODE_SCREEN: // Cr = Cs * (1 - Cd) + Cd
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;

        case SP_BLEND_MODE_NORMAL:
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                                GL_ONE_MINUS_SRC_ALPHA);
            break;

        default: // Transparency blending
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
    }
}

/**
 * Clear old GL data
 */
void Sticker::clearGLData() {
    mVertexData.clear();
    mColors.clear();
    mTexCoords.clear();
}

/**
 * Update vextex data and texture coordinate data from region attachment
 */
void Sticker::updateVertexAndTexCoordsData_fromRegionAttachment(spRegionAttachment *region,
                                                                spSlot *slot) {
    // LOGD("REGION_ATTACHMENT: Update vertices data...");
    spRegionAttachment_computeWorldVertices(region, slot->bone, mWorldVertices, 0, 2);
    float r = mSkeleton->color.r * slot->color.r * region->color.r;
    float g = mSkeleton->color.g * slot->color.g * region->color.g;
    float b = mSkeleton->color.b * slot->color.b * region->color.b;
    float a = mSkeleton->color.a * slot->color.a * region->color.a;

    unsigned int order[6] = {0, 1, 2, 3, 0, 2};
    int j;
    for (int i = 0; i < 6; i++) {
        mColors.push_back(r);
        mColors.push_back(g);
        mColors.push_back(b);
        mColors.push_back(a);

        j = order[i];
        mVertexData.push_back(mWorldVertices[j * 2]);
        mVertexData.push_back(mWorldVertices[j * 2 + 1]);
        mTexCoords.push_back(region->uvs[j * 2]);
        mTexCoords.push_back(region->uvs[j * 2 + 1]);
    }

    // LOGD("REGION_ATTACHMENT: Updated. Size = %d %d", mVertexData.size(), mTexCoords.size());
}

/**
 * Update vextex data and texture coordinate data from mesh attachment
 */
void Sticker::updateVertexAndTexCoordsData_fromMeshAttachment(spMeshAttachment *mesh,
                                                              spSlot *slot) {
    if (mesh->super.worldVerticesLength > MAX_VERTEX_COUNT)
        return;

    // LOGD("MESH_ATTACHMENT: Update vertices data...");
    spVertexAttachment_computeWorldVertices(SUPER(mesh), slot, 0, mesh->super.worldVerticesLength,
                                            mWorldVertices, 0, 2);
    float r = mSkeleton->color.r * slot->color.r * mesh->color.r;
    float g = mSkeleton->color.g * slot->color.g * mesh->color.g;
    float b = mSkeleton->color.b * slot->color.b * mesh->color.b;
    float a = mSkeleton->color.a * slot->color.a * mesh->color.a;

    for (int i = 0; i < mesh->trianglesCount; ++i) {
        int j = mesh->triangles[i] << 1;
        mColors.push_back(r);
        mColors.push_back(g);
        mColors.push_back(b);
        mColors.push_back(a);

        mVertexData.push_back(mWorldVertices[j]);
        mVertexData.push_back(mWorldVertices[j + 1]);
        mTexCoords.push_back(mesh->uvs[j]);
        mTexCoords.push_back(mesh->uvs[j + 1]);
    }
    // LOGD("MESH_ATTACHMENT: Updated. Size = %d %d", mVertexData.size(), mTexCoords.size());
}

/**
 * Bind buffer data
 */
void Sticker::bindBufferData() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_POSITION]);
    glBufferData(GL_ARRAY_BUFFER, mVertexData.size() * sizeof(float), &mVertexData[0],
                 GL_STATIC_DRAW);
    checkGlError("glBufferData - vertex data");

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_COLORS]);
    glBufferData(GL_ARRAY_BUFFER, mColors.size() * sizeof(float), &mColors[0], GL_STATIC_DRAW);
    checkGlError("glBufferData - color data");

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_TEX_COORDS]);
    glBufferData(GL_ARRAY_BUFFER, mTexCoords.size() * sizeof(float), &mTexCoords[0],
                 GL_STATIC_DRAW);
    checkGlError("glBufferData - texture coordinates data");
}

/**
 * Pass data to GPU
 */
void Sticker::passDataToOpenGl() {
    glUseProgram(mProgram);
    checkGlError("glUseProgram");

    // Pass vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_POSITION]);
    glEnableVertexAttribArray(mPositionHandle);
    glVertexAttribPointer(mPositionHandle, mCountPerVertex, GL_FLOAT, GL_FALSE,
                          mCountPerVertex * sizeof(GLfloat), 0);
    checkGlError("glVertexAttribPointer - pass vertex data");

    // Pass color  data
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_COLORS]);
    glEnableVertexAttribArray(mColorHandle);
    glVertexAttribPointer(mColorHandle, mCountPerColor, GL_FLOAT, GL_FALSE,
                          mCountPerColor * sizeof(float), 0);
    checkGlError("glVertexAttribPointer - pass color data");

    // Pass texture coordinate data
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_TEX_COORDS]);
    glEnableVertexAttribArray(mTexCoordsHandle);
    glVertexAttribPointer(mTexCoordsHandle, mCountPerTexCoord, GL_FLOAT, GL_FALSE,
                          mCountPerTexCoord * sizeof(GLfloat), 0);
    checkGlError("glVertexAttribPointer - pass texture coordinates data");

    // Pass texture data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexDataHandle);
    glUniform1i(mTexSampler2DHandle, 0);
    checkGlError("glUniform1i - pass texture data");

    // Pass MVP matrix data
    calculateMvpMatrix();
    glUniformMatrix4fv(mMvpMatrixHandle, 1, GL_FALSE, value_ptr(mMvpMatrix));
    checkGlError("glUniformMatrix4fv - pass MVP matrix data");
}

/**
 * Render sticker
 */
void Sticker::render() {
    // Bind data to GPU
    LOGD("Binding buffer data....");
    bindBufferData();

    // Pass data to OpenGL
    LOGD("Passing data to OpenGL....");
    passDataToOpenGl();

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, (unsigned) this->mVertexData.size() / 2);
    checkGlError("glDrawArrays");

    glDisableVertexAttribArray(this->mPositionHandle);
    glDisableVertexAttribArray(this->mTexCoordsHandle);
}

/**
 * Calculate MVP matrix
 */
void Sticker::calculateMvpMatrix() {
    mModelMatrix = mat4(1.0f);
    mModelMatrix = translate(mModelMatrix, mTrans);
    mModelMatrix = rotate(mModelMatrix, radians(mAngle), vec3(0, 0, 1.0f));
    mMvpMatrix = mProjectionMatrix * mViewMatrix * mModelMatrix;
}

Sticker::~Sticker() {
    if (eglGetCurrentContext() != mEglContext)
        return;

    LOGD("Clearing EGL data...........");
    glDeleteBuffers(VB_COUNT, mVB);
    glDeleteTextures(1, &mTexDataHandle);
    glDeleteProgram(mProgram);
    LOGD("Clear EGL data: SUCCESSFUL...........");

    clearGLData();

    if (mWorldVertices != NULL) {
        delete[] mWorldVertices;
        mWorldVertices = NULL;
    }

    disposeSpineData();
}

/**
 * Dispose spine data
 */
void Sticker::disposeSpineData() {
    if (mAnimationState) {
        spAnimationState_dispose(mAnimationState);
        LOGD("Dispose animation state");
    }

    if (mSkeleton) {
        spSkeleton_dispose(mSkeleton);
        LOGD("Dispose skeleton");
    }

    if (mAnimationStateData) {
        spAnimationStateData_dispose(mAnimationStateData);
        LOGD("Dispose animation state data");
    }

    if (mSkeletonData) {
        spSkeletonData_dispose(mSkeletonData);
        LOGD("Dispose skeleton data");
    }

    if (mAtlas) {
        spAtlas_dispose(mAtlas);
        LOGD("Dispose atlas");
    }

    mAtlas = NULL;
    mSkeletonData = NULL;
    mSkeleton = NULL;
    mAnimationStateData = NULL;
    mAnimationState = NULL;

    LOGD("Dispose Spine data: SUCCESSFUL...........");
}