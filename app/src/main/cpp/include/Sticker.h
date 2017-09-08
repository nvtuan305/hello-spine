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

#define VB_COUNT 2
#define VB_POSITION 0
#define VB_TEX_COORDS 1
#define MAX_VERTEX_COUNT 6000

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
    GLuint mCountPerVertex;
    vector<float> mTexCoords;
    GLuint mCountPerTexCoords;

    GLuint mProgram;
    GLuint mVB[VB_COUNT];

    GLint mPositionHandle;
    GLint mMvpMatrixHandle;
    GLuint mTexDataHandle;
    GLint mTexCoordsHandle;
    GLint mTexSampler2DHandle;

    mat4 mModelMatrix;
    mat4 mViewMatrix;
    mat4 mProjectionMatrix;
    mat4 mMvpMatrix;
    vec3 mTrans;
    float mAngle;

    float *mWorldVertices;

    long mLastAnimationTime;

    virtual void initOpenGL(const char *texturePath);

    virtual void initSkeleton();

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

    virtual void calculateMvpMatrix();

    virtual void resize(int width, int height);

    virtual void draw();

    virtual void updateVertexAndTexCoordsData();

    virtual void updateVertexAndTexCoordsData_FromRegionAttachment(spRegionAttachment *attachment,
                                                                   spBone *bone);

    virtual void updateVertexAndTexCoordsData_FromMeshAttachment(spMeshAttachment *attachment,
                                                                 spSlot *slot);

    virtual void disposeSpineData();
};

#endif
