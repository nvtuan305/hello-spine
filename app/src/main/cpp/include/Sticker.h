#ifndef HELLO_SPINE_STICKER_H
#define HELLO_SPINE_STICKER_H

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <glm/glm.hpp>
#include <spine/spine.h>
#include <spine/extension.h>
#include <vector>

using namespace std;
using namespace glm;

#define VB_COUNT 3
#define VB_POSITION 0
#define VB_COLORS 1
#define VB_TEX_COORDS 2
#define MAX_VERTEX_COUNT 8000

class Sticker {

private:
    spAtlas *mAtlas;
    spSkeletonData *mSkeletonData;
    spSkeleton *mSkeleton;
    spAnimationStateData *mAnimationStateData;
    spAnimationState *mAnimationState;

    char *mAtlasPath;
    char *mJsonPath;
    char *mImagePath;
    char *mDefaultAnimation;

    EGLContext mEglContext;
    vector<float> mVertexData;
    vector<float> mColors;
    vector<float> mTexCoords;
    GLuint mCountPerVertex;
    GLuint mCountPerColor;
    GLuint mCountPerTexCoord;

    GLuint mProgram;
    GLuint mVB[VB_COUNT];

    GLuint mPositionHandle;
    GLuint mColorHandle;
    GLuint mMvpMatrixHandle;
    GLuint mTexDataHandle;
    GLuint mTexCoordsHandle;
    GLuint mTexSampler2DHandle;

    mat4 mModelMatrix;
    mat4 mViewMatrix;
    mat4 mProjectionMatrix;
    mat4 mMvpMatrix;
    vec3 mTrans;
    float mAngle;

    float *mWorldVertices;
    long mLastAnimationTime;
    int mCurrentBlendMode = -1;

    virtual void initOpenGL(const char *texturePath);

    virtual void initSpine();

    virtual void bindBufferData();

    virtual void passDataToOpenGl();

public:
    Sticker(const char *atlasPath,
            const char *jsonPath,
            const char *imagePath,
            const char *defaultAnimation);

    ~Sticker();

    virtual void init();

    virtual void setAngleAndTranslation(float angle, glm::vec3 trans);

    virtual void resize(int width, int height);

    virtual void calculateMvpMatrix();

    virtual long calculateDeltaTime();

    virtual void draw();

    virtual void render();

    virtual void updateVertexAndTexCoordsData();

    virtual void updateBlendMode(int newMode);

    virtual void updateVertexAndTexCoordsData_fromRegionAttachment(spRegionAttachment *attachment,
                                                                   spSlot *slot);

    virtual void updateVertexAndTexCoordsData_fromMeshAttachment(spMeshAttachment *attachment,
                                                                 spSlot *slot);

    virtual void clearGLData();

    virtual void disposeSpineData();
};

#endif
