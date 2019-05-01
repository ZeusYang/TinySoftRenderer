#include "RenderLoop.h"

#include <QDebug>

#include <time.h>
#include "SoftRenderer/Mesh.h"

using namespace SoftRenderer;

RenderLoop::RenderLoop(int w, int h, QObject *parent)
    : QObject(parent), width(w), height(h), channel(4)
{
    fps = 0;
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
    Mesh cube, floor;
    cube.asBox(1.0,1.0,1.0);
    floor.asFloor(8.0,-0.5f);

    // pipeline
    pipeline->initialize();

    // transformation.
    double angle = 0.0;
    Matrix4x4 cubeRotat, floorMat;
    Matrix4x4 cubes[4], scales;
    //scales.setScale(Vector3D(5.0,5.0,5.0));
    //floorMat.setRotationX(90.0);
    cubes[0].setTranslation(Vector3D(1.0f, 0.0f,-1.0f));
    cubes[1].setTranslation(Vector3D(2.0f, 0.0f,-1.0f));
    cubes[2].setTranslation(Vector3D(1.5f, 1.0f,-1.0f));
    cubes[3].setTranslation(Vector3D(-1.5f,0.0f,+2.0f));

    pipeline->setViewMatrix(Vector3D(+3.0f,2.0f,7.0f),Vector3D(0.0f,0.0f,0.0f),Vector3D(0.0f,1.0f,.0f));
    pipeline->setProjectMatrix(45.0f, static_cast<float>(width)/height,0.1f, 50.0f);

    // calculate time stamp.
    clock_t start, finish;

    // fps counting.
    fps = 0;

    // load textures.
    unsigned int cubeUnit = pipeline->loadTexture("./res/cube.jpg");
    unsigned int cube1Unit = pipeline->loadTexture("./res/cube1.bmp");
    unsigned int cube2Unit = pipeline->loadTexture("./res/marble.jpg");
    unsigned int floorUnit = pipeline->loadTexture("./res/cube.jpg");

    // render loop.
    while(!stoped)
    {
        start = clock();

        pipeline->beginFrame();

        pipeline->clearBuffer(Vector4D(0.502f,0.698f,0.800f,1.0f));

        // render cube.
        {
            pipeline->bindTexture(cubeUnit);
            pipeline->setVertexBuffer(cube.vertices);
            pipeline->setIndexBuffer(cube.indices);

            pipeline->setModelMatrix(cubeRotat * scales);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->bindTexture(cube1Unit);
            pipeline->setModelMatrix(cubes[0]);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->setModelMatrix(cubes[1]);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->setModelMatrix(cubes[2]);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->bindTexture(cube2Unit);
            pipeline->setModelMatrix(cubes[3]);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->unBindTexture(cubeUnit);
        }

        // render floor.
        {
            pipeline->bindTexture(floorUnit);
            pipeline->setModelMatrix(floorMat);
            pipeline->setVertexBuffer(floor.vertices);
            pipeline->setIndexBuffer(floor.indices);
            pipeline->drawIndex(RenderMode::fill);
            pipeline->unBindTexture(floorUnit);
        }

        pipeline->swapBuffer();

        pipeline->endFrame();

        finish = clock();
        deltaFrameTime = (double)(finish-start)/CLOCKS_PER_SEC;

        // cube rotation.
        angle += 45 * deltaFrameTime;
        angle = fmod(angle, 360.0);
        cubeRotat.setRotationY(angle);

        // send the frame.
        emit frameOut(pipeline->output(), pipeline->getProfile().num_triangles,
                      pipeline->getProfile().num_vertices);
        ++ fps;
    }
}
