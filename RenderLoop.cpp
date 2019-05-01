#include "RenderLoop.h"

#include <QDebug>

#include <time.h>
#include "SoftRenderer/Mesh.h"

using namespace SoftRenderer;

RenderLoop::RenderLoop(int w, int h, QObject *parent)
    : QObject(parent), width(w), height(h), channel(4)
{
    fps = 0;
    lastFrameTime = 0;
    deltaFrameTime = 0;
    stoped = false;
    pipeline = new Pipeline(width, height);
}

RenderLoop::~RenderLoop()
{
    if(pipeline) delete pipeline;
    pipeline = nullptr;
}

void RenderLoop::loop()
{
    // mesh
    Mesh line;
    line.asTriangle(Vector3D(0.f,1.f,0.f),Vector3D(-1.f,-.5f,0.f),Vector3D(+1.f,-.5f,0.f));
    //line.asTriangle(Vector3D(0.f,-1.f,0.f),Vector3D(1.f,.5f,0.f),Vector3D(-1.f,.5f,0.f));
    //line.asTriangle(Vector3D(0.f,1.f,0.f),Vector3D(-1.f,0.5f,0.f),Vector3D(1.f,-.5f,0.f));
    //line.asTriangle(Vector3D(-0.8f,0.8f,0.f),Vector3D(0.2f,0.3f,0.f),Vector3D(1.f,-.5f,0.f));

    // pipeline
    pipeline->initialize();
    pipeline->setVertexBuffer(line.vertices);
    pipeline->setIndexBuffer(line.indices);

    // transformation.
    double angle = 0.0;
    Matrix4x4 scales, rotat;
    scales.setScale(Vector3D(5.0f,5.0f,5.0f));
    pipeline->setModelMatrix(scales);
    pipeline->setViewMatrix(Vector3D(0.0f,0.0f,20.0f),Vector3D(0.0f,0.0f,0.0f),Vector3D(0.0f,1.0f,.0f));
    pipeline->setProjectMatrix(45.0f, static_cast<float>(width)/height,0.1f, 100.0f);

    // calculate time stamp.
    clock_t start, finish;

    // fps counting.
    fps = 0;
    while(!stoped)
    {
        start = clock();

        pipeline->clearBuffer(Vector4D(0.502f,0.698f,0.800f,1.0f));

        pipeline->drawIndex(RenderMode::fill);

        pipeline->swapBuffer();

        emit frameOut(pipeline->output());
        ++ fps;

        finish = clock();

        deltaFrameTime = (double)(finish-start)/CLOCKS_PER_SEC;
        angle += 45 * deltaFrameTime;
        angle = fmod(angle, 360.0);
        rotat.setRotationY(angle);
        pipeline->setModelMatrix(rotat * scales);
    }
}
