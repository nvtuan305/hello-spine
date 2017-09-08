#include <Sticker.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <android/log.h>
#include <time.h>
#include <GLES2Utils.h>

#define LOG_TAG "STICKER_CPP"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

const char mVertexShaderCode[] =
        "attribute vec2 a_Position;"
                "uniform mat4 u_MVPMatrix;"
                "attribute vec2 a_TexCoords;"
                "varying vec2 v_TexCoords;"
                "void main() {"
                "    v_TexCoords = a_TexCoords;"
                "    gl_Position = u_MVPMatrix * vec4(a_Position, 0.0f, 1.0f);"
                "}";

const char mFragShaderCode[] =
        "precision mediump float;"
                "varying vec2 v_TexCoords;"
                "uniform sampler2D u_Texture;"
                "void main() {"
                "    gl_FragColor = texture2D(u_Texture, v_TexCoords);"
                "}";

char *getString(const char *str) {
    if (str == NULL)
        return NULL;

    size_t length = strlen(str);
    char *result = new char[length + 1];
    strncpy(result, str, length);
    result[length] = '\0';

    return result;
}

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
    mCountPerTexCoords = 2; // (x, y)

    mProgram = 0;
    mPositionHandle = 0;
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

Sticker::~Sticker() {
    mVertexData.clear();
    mTexCoords.clear();

    if (eglGetCurrentContext() == mEglContext) {
        for (int i = 0; i < VB_COUNT; ++i) {
            glDeleteBuffers(1, &mVB[i]);
        }
        glDeleteProgram(mProgram);
    }
}

void Sticker::init() {
    initOpenGL(mImagePath);
    initSkeleton();
}

void Sticker::initOpenGL(const char *texturePath) {
    // Create program only one time
    if (mProgram == 0) {
        mProgram = createProgram(mVertexShaderCode, mFragShaderCode);
        if (mProgram == 0) return;
    }

    mPositionHandle = glGetAttribLocation(mProgram, "a_Position");
    mTexCoordsHandle = glGetAttribLocation(mProgram, "a_TexCoords");
    mMvpMatrixHandle = glGetUniformLocation(mProgram, "u_MVPMatrix");
    mTexSampler2DHandle = glGetUniformLocation(mProgram, "u_Texture");

    mTexDataHandle = loadTexture(texturePath);
    mViewMatrix = lookAt(
            vec3(0, 0, 3.0f),
            vec3(0, 0, 0),
            vec3(0, 1.0f, 0)
    );
    LOGD("Init OpenGL: SUCCESSFUL...................");
}

void Sticker::initSkeleton() {
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
    mSkeleton->x = 0;
    mSkeleton->y = 0;
    spSkeleton_updateWorldTransform(mSkeleton);
    LOGD("Init Spine: SUCCESSFUL...................");
}

void Sticker::disposeSpineData() {
    if (mAtlas) {
        spAtlas_dispose(mAtlas);
        LOGD("Dispose atlas");
    }

    if (mSkeletonData) {
        spSkeletonData_dispose(mSkeletonData);
        LOGD("Dispose skeleton data");
    }

    if (mSkeleton) {
        spSkeleton_dispose(mSkeleton);
        LOGD("Dispose skeleton");
    }

    if (mAnimationStateData) {
        spAnimationStateData_dispose(mAnimationStateData);
        LOGD("Dispose animation state data");
    }

    if (mAnimationState) {
        spAnimationState_dispose(mAnimationState);
        LOGD("Dispose animation state");
    }

    mAtlas = NULL;
    mSkeletonData = NULL;
    mSkeleton = NULL;
    mAnimationStateData = NULL;
    mAnimationState = NULL;
}

void Sticker::bindBufferData() {
    // Delete buffer before generating new buffer
    glDeleteBuffers(VB_COUNT, mVB);

    glGenBuffers(VB_COUNT, mVB);
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_POSITION]);
    glBufferData(GL_ARRAY_BUFFER, mVertexData.size() * sizeof(GLfloat), &mVertexData[0],
                 GL_STATIC_DRAW);
    checkGlError("glBufferData - vertex data");

    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_TEX_COORDS]);
    glBufferData(GL_ARRAY_BUFFER, mTexCoords.size() * sizeof(GLfloat), &mTexCoords[0],
                 GL_STATIC_DRAW);
    checkGlError("glBufferData - texture coordinates data");
}

void Sticker::setAngleAndTranslation(float angle, vec3 trans) {
    mAngle = angle;
    mTrans = trans;
}

void Sticker::resize(int width, int height) {
    glViewport(0, 0, width, height);
    float ratio = (float) width / (float) height;
    //mProjectionMatrix = frustum(-ratio * 800.0f, ratio * 800.0f, -800.0f, 800.0f, 2.0f, 5.0f);
    mProjectionMatrix =  ortho(-ratio * 1000.0f, ratio * 1000.0f, -1000.0f, 1000.0f, 2.0f, 5.0f);
}

void Sticker::passDataToOpenGl() {
    glUseProgram(mProgram);
    checkGlError("glUseProgram");

    // Pass vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_POSITION]);
    glEnableVertexAttribArray(mPositionHandle);
    glVertexAttribPointer(mPositionHandle, mCountPerVertex, GL_FLOAT, GL_FALSE,
                          mCountPerVertex * sizeof(GLfloat), 0);
    checkGlError("glVertexAttribPointer - pass vertex data");

    // Pass texture coordinate data
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_TEX_COORDS]);
    glEnableVertexAttribArray(mTexCoordsHandle);
    glVertexAttribPointer(mTexCoordsHandle, mCountPerTexCoords, GL_FLOAT, GL_FALSE,
                          mCountPerTexCoords * sizeof(GLfloat), 0);
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

void Sticker::calculateMvpMatrix() {
    // model * view * projection
    mModelMatrix = mat4(1.0f);
    mModelMatrix = translate(mModelMatrix, mTrans);
    mModelMatrix = rotate(mModelMatrix, radians(mAngle), vec3(0, 0, 1.0f));
    mMvpMatrix = mProjectionMatrix * mViewMatrix * mModelMatrix;
}

float currentSystemSecond() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (float) (time.tv_sec + ((float) time.tv_nsec) / 1e9);
}

long currentSystemMillisecond() {
    return (long) (currentSystemSecond() * 1000);
}

void Sticker::draw() {
    long currentTime = currentSystemMillisecond();
    long deltaTime = mLastAnimationTime == 0L ? 0L : currentTime - mLastAnimationTime;
    mLastAnimationTime = currentTime;

    // Update animation state by delta time
    spAnimationState_update(mAnimationState, deltaTime * 1.0f / 1000.0f);
    LOGD("spAnimationState_update - delta time = %f", deltaTime * 1.0f / 1000.0f);

    // Apply the animation state to skeleton
    spAnimationState_apply(mAnimationState, mSkeleton);
    LOGD("Apply the animation state to skeleton");

    // Calculate world transform for rendering
    spSkeleton_updateWorldTransform(mSkeleton);
    LOGD("Calculate world transform for rendering");

    // Update new data
    LOGD("Updating new data...");
    updateVertexAndTexCoordsData();

    LOGD("Binding buffer data....");
    bindBufferData();

    // Pass data to OpenGL
    LOGD("Passing data to OpenGL....");
    passDataToOpenGl();

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, (unsigned) mVertexData.size() / 2);
    checkGlError("glDrawArrays");

    glDisableVertexAttribArray(mPositionHandle);
    glDisableVertexAttribArray(mTexCoordsHandle);
}

void Sticker::updateVertexAndTexCoordsData() {
    if (!mSkeleton) {
        LOGE("updateVertexAndTexCoordsData - skeleton is NULL");
        return;
    }

    // Clear old data
    mVertexData.clear();
    mTexCoords.clear();

    for (int i = 0; i < mSkeleton->slotsCount; ++i) {
        spSlot *slot = mSkeleton->drawOrder[i];
        if (!slot) continue;

        spAttachment *attachment = slot->attachment;
        if (!attachment) continue;
        LOGD("Processing attachment of SLOT[%d]...", i);

        switch (attachment->type) {
            case SP_ATTACHMENT_REGION:
                updateVertexAndTexCoordsData_FromRegionAttachment(
                        (spRegionAttachment *) attachment, slot->bone);
                break;

            case SP_ATTACHMENT_MESH:
                updateVertexAndTexCoordsData_FromMeshAttachment((spMeshAttachment *) attachment,
                                                                slot);
                break;

            default:
                break;
        }
    }
}

void Sticker::updateVertexAndTexCoordsData_FromRegionAttachment(spRegionAttachment *region,
                                                                spBone *bone) {
    //LOGD("REGION_ATTACHMENT: Update vertices data...");
    spRegionAttachment_computeWorldVertices(region, bone, mWorldVertices, 0, 2);
    // Create array draws: [0 - 1 - 2] [3 - 0 - 2]
    // Push vertex 0 - 1 - 2 - 3
    int i = 0;
    for (i = 0; i < 8; i = i + 2) {
        mVertexData.push_back(mWorldVertices[i]);
        mVertexData.push_back(mWorldVertices[i + 1]);
        mTexCoords.push_back(region->uvs[i]);
        mTexCoords.push_back(region->uvs[i + 1]);
    }

    // Push vertex 0
    i = 0;
    mVertexData.push_back(mWorldVertices[i]);
    mVertexData.push_back(mWorldVertices[i + 1]);
    mTexCoords.push_back(region->uvs[i]);
    mTexCoords.push_back(region->uvs[i + 1]);

    // Push vertex 2
    i = 2;
    mVertexData.push_back(mWorldVertices[i]);
    mVertexData.push_back(mWorldVertices[i + 1]);
    mTexCoords.push_back(region->uvs[i]);
    mTexCoords.push_back(region->uvs[i + 1]);
    //LOGD("REGION_ATTACHMENT: Updated. Size = %d %d", mVertexData.size(), mTexCoords.size());
}

void Sticker::updateVertexAndTexCoordsData_FromMeshAttachment(spMeshAttachment *mesh,
                                                              spSlot *slot) {
    if (mesh->super.worldVerticesLength > MAX_VERTEX_COUNT)
        return;

    //LOGD("MESH_ATTACHMENT: Update vertices data...");
    spVertexAttachment_computeWorldVertices(SUPER(mesh), slot, 0, mesh->super.worldVerticesLength,
                                            mWorldVertices, 0, 2);
    for (int i = 0; i < mesh->trianglesCount; ++i) {
        int j = mesh->triangles[i] << 1;
        mVertexData.push_back(mWorldVertices[j]);
        mVertexData.push_back(mWorldVertices[j + 1]);
        mTexCoords.push_back(mesh->uvs[j]);
        mTexCoords.push_back(mesh->uvs[j + 1]);
        //LOGD("MESH_ATTACHMENT: triangles[%d] = %f %f %f %f", i, mWorldVertices[j],
//             mWorldVertices[j + 1],
//             mesh->uvs[j], mesh->uvs[j + 1]);
    }
    //LOGD("MESH_ATTACHMENT: Updated. Size = %d %d", mVertexData.size(), mTexCoords.size());
}