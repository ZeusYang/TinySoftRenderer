#include "RenderLoop.h"

#include <QDebug>

#include "SoftRenderer/Mesh.h"

using namespace SoftRenderer;

RenderLoop::RenderLoop(int w, int h, QObject *parent)
    : QObject(parent), width(w), height(h), channel(4)
{
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

    // pipeline
    pipeline->initialize();
    pipeline->setVertexBuffer(line.vertices);
    pipeline->setIndexBuffer(line.indices);
    while(!stoped)
    {
        pipeline->clearBuffer(Vector4D(0.502f,0.698f,0.800f,1.0f));

        pipeline->drawIndex(RenderMode::polygon);

        emit frameOut(pipeline->output());
    }
}
