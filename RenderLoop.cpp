#include "RenderLoop.h"

#include <QDebug>

#include <time.h>
#include "SoftRenderer/Mesh.h"
#include "SoftRenderer/ObjModel.h"

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
    // pipeline
    pipeline->initialize();
    pipeline->setShaderMode(ShadingMode::Phong);

    // lighting
    Material material;
    material.setMaterial(Vector3D(0.1,0.1,0.1),
                         Vector3D(0.5,0.5,0.5),
                         Vector3D(0.8,0.8,0.8),
                         2.0);
    pipeline->setMaterial(&material);
    pipeline->setDirectionLight(Vector3D(-1,-2,-1),
                                Vector3D(0.05,0.05,0.05),
                                Vector3D(0.6,0.6,0.6),
                                Vector3D(0.4,0.4,0.4));

    // ObjModel
    ObjModel diablo("./res/diablo3_pose.obj");

    // mesh
    Mesh cube, floor;
    cube.asBox(1.0,1.0,1.0);
    floor.asFloor(4.0,-1.5f);

    // transformation.
    double angle = 0.0;
    Matrix4x4 rotat, diabloMatrix, cubes[3], floorM;
    cubes[0].setTranslation(Vector3D(3.0f, -1.0f,-1.0f));
    cubes[1].setTranslation(Vector3D(4.0f, -1.0f,-1.0f));
    cubes[2].setTranslation(Vector3D(3.5f, 0.0f,-1.0f));
    diabloMatrix = diablo.setSize(2.3,2.3,2.3);

    pipeline->setViewMatrix(Vector3D(-3.0f, 2.0f, 5.0f),Vector3D(0.0f,0.0f,0.0f),Vector3D(0.0f,1.0f,.0f));
    pipeline->setProjectMatrix(45.0f, static_cast<float>(width)/height,0.1f, 50.0f);

    // calculate time stamp.
    clock_t start, finish;
    fps = 0;

    // load textures.
    unsigned int cubeUnit = pipeline->loadTexture("./res/cube.jpg");
    unsigned int floorUnit = pipeline->loadTexture("./res/floor.jpg");
    unsigned int diablo3 = pipeline->loadTexture("./res/diablo3_pose_diffuse.jpg");

    // render loop.
    while(!stoped)
    {
        start = clock();

        pipeline->beginFrame();

        //pipeline->clearBuffer(Vector4D(0.502f,0.698f,0.800f,1.0f));
        pipeline->clearBuffer(Vector4D(0.0,0.0,0.0,1.0f));

        // render cube.
        {
            pipeline->bindTexture(cubeUnit);
            pipeline->setVertexBuffer(&cube.m_vertices);
            pipeline->setIndexBuffer(&cube.m_indices);

            pipeline->bindTexture(cubeUnit);
            pipeline->setModelMatrix(cubes[0]);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->setModelMatrix(cubes[1]);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->setModelMatrix(cubes[2]);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->unBindTexture(cubeUnit);
        }

        // render diablo model.
        {
            pipeline->bindTexture(diablo3);
            pipeline->setModelMatrix(rotat * diabloMatrix);
            pipeline->setVertexBuffer(&diablo.m_vertices);
            pipeline->setIndexBuffer(&diablo.m_indices);
            pipeline->drawIndex(RenderMode::fill);
            pipeline->unBindTexture(diablo3);
        }

        // render floor.
        {
            pipeline->bindTexture(floorUnit);
            pipeline->setModelMatrix(floorM);
            pipeline->setVertexBuffer(&floor.m_vertices);
            pipeline->setIndexBuffer(&floor.m_indices);
            pipeline->drawIndex(RenderMode::fill);
            pipeline->unBindTexture(floorUnit);
        }

        pipeline->swapBuffer();

        pipeline->endFrame();

        finish = clock();
        deltaFrameTime = (double)(finish-start)/CLOCKS_PER_SEC;

        // rotation.
        angle += 45 * deltaFrameTime;
        angle = fmod(angle, 360.0);
        rotat.setRotationY(angle);

        // send the frame.
        emit frameOut(pipeline->output(), pipeline->getProfile().num_triangles,
                      pipeline->getProfile().num_vertices);
        ++ fps;
    }
}
