#include "RenderLoop.h"

#include <QDebug>

#include <time.h>
#include "SoftRenderer/Transform3D.h"
#include "SoftRenderer/Math/Quaternion.h"
#include "SoftRenderer/Light.h"
#include "SoftRenderer/Mesh.h"
#include "SoftRenderer/ObjModel.h"

using namespace SoftRenderer;

RenderLoop::RenderLoop(int w, int h, QObject *parent)
    : QObject(parent), width(w), height(h), channel(4)
{
    fps = 0;
    deltaFrameTime = 0;
    stoped = false;
    fpsCamera = nullptr;
    tPSCamera = nullptr;
    pipeline = new Pipeline(width, height);
}

RenderLoop::~RenderLoop()
{
    if(pipeline) delete pipeline;
    if(fpsCamera) delete fpsCamera;
    if(tPSCamera) delete tPSCamera;
    fpsCamera = nullptr;
    tPSCamera = nullptr;
    pipeline = nullptr;
}

void RenderLoop::loop()
{
    // pipeline
    pipeline->initialize();
    pipeline->setShaderMode(ShadingMode::Phong);
    unsigned int cubeUnit = pipeline->loadTexture("./res/cube.jpg");
    unsigned int floorUnit = pipeline->loadTexture("./res/floor.jpg");
    unsigned int treeUnit = pipeline->loadTexture("./res/tree.png");
    unsigned int diablo3 = pipeline->loadTexture("./res/diablo3_pose_diffuse.jpg");
    unsigned int lowPolygonTree = pipeline->loadTexture("./res/lowPolyTree.png");
    unsigned int playerUnit = pipeline->loadTexture("./res/player.png");

    // mesh
    ObjModel player("./res/player.obj");
    ObjModel diablo("./res/diablo3_pose.obj");
    ObjModel tree("./res/tree.obj");
    ObjModel tree1("./res/lowPolyTree.obj");
    Mesh cube, floor;
    cube.asBox(1.0,1.0,1.0);
    floor.asFloor(15.3,-1.5f);
    Material material;
    material.setMaterial(Vector3D(0.1,0.1,0.1),
                         Vector3D(0.5,0.5,0.5),
                         Vector3D(0.8,0.8,0.8),
                         16.0);
    pipeline->setMaterial(&material);

    // camera
    fpsCamera = new FPSCamera(Vector3D(-0.0f, 5.0f, 14.0f));
    fpsCamera->rotate(FPSCamera::LocalRight, -30.0f);
    tPSCamera = new TPSCamera(Vector3D(+4,-1.0, +4));

    // illumination.
    // directional light
//    pipeline->setDirectionLight(
//                Vector3D(0.05,0.05,0.05),
//                Vector3D(0.9,0.1,0.1),
//                Vector3D(0.9,0.1,0.1),
//                Vector3D(+1,-2,-1));
    // point light.
    pipeline->setPointLight(
                Vector3D(0.2,0.2,0.2),
                Vector3D(0.9,0.1,0.1),
                Vector3D(0.9,0.1,0.1),
                Vector3D(0.0,3.0,0.0),
                Vector3D(1.0f,0.07f,0.017f));
    // spot light.
    pipeline->setSpotLight(
                Vector3D(0.1,0.1,0.1),
                Vector3D(0.9,0.1,0.1),
                Vector3D(0.9,0.1,0.1),
                40.0f,
                Vector3D(0.0,5.0,0.0),
                Vector3D(0.0,-3.0,0.0),
                Vector3D(1.0f,0.07f,0.017f));

    // transformation.
    double angle = 0.0;
    Quaternion test;
    Transform3D diabloTransform, floorTransform, cubeTransform;
    Transform3D tree0Transform, tree1Transform, playerTransform;
    diabloTransform.setScale(diablo.setSizeToVector(2.3, 2.3, 2.3));
    cubeTransform.setTranslation(Vector3D(3.0f,-1.0f,+2.0f));
    tree0Transform.setScale(tree.setSizeToVector(2.0,2.0,2.0));
    tree0Transform.setTranslation(Vector3D(-4,-1.5f,-4));
    tree1Transform.setScale(tree.setSizeToVector(0.15,0.15,0.15));
    tree1Transform.setTranslation(Vector3D(-4,-1.5f,+4));
    playerTransform.setScale(player.setSizeToVector(1.0,1.0,1.0));
    playerTransform.setTranslation(Vector3D(+4,-1.0, +4));

    pipeline->setProjectMatrix(45.0f, static_cast<float>(width)/height,0.1f, 40.0f);

    // calculate time stamp.
    fps = 0;
    clock_t start, finish;
    pipeline->setViewMatrix(fpsCamera->getPosition(), fpsCamera->getViewMatrix());

    // render loop.
    while(!stoped)
    {
        start = clock();

        pipeline->beginFrame();

        //pipeline->clearBuffer(Vector4D(0.502f,0.698f,0.800f,1.0f));
        pipeline->clearBuffer(Vector4D(0.0,0.0,0.0,1.0f));

        //pipeline->setViewMatrix(fpsCamera->getPosition(), fpsCamera->getViewMatrix());
        pipeline->setViewMatrix(tPSCamera->getPosition(), tPSCamera->getViewMatrix());
        // render cube.
        {
            pipeline->bindTexture(cubeUnit);
            pipeline->setVertexBuffer(&cube.m_vertices);
            pipeline->setIndexBuffer(&cube.m_indices);
            pipeline->bindTexture(cubeUnit);
            pipeline->setModelMatrix(cubeTransform.toMatrix());
            pipeline->drawIndex(RenderMode::fill);
            pipeline->unBindTexture(cubeUnit);
        }

        // render diablo model.
        {
            pipeline->bindTexture(diablo3);
            pipeline->setModelMatrix(diabloTransform.toMatrix());
            pipeline->setVertexBuffer(&diablo.m_vertices);
            pipeline->setIndexBuffer(&diablo.m_indices);
            pipeline->drawIndex(RenderMode::fill);
            pipeline->unBindTexture(diablo3);
        }

        // render floor.
        {
            pipeline->bindTexture(floorUnit);
            pipeline->setModelMatrix(floorTransform.toMatrix());
            pipeline->setVertexBuffer(&floor.m_vertices);
            pipeline->setIndexBuffer(&floor.m_indices);
            pipeline->drawIndex(RenderMode::fill);
            pipeline->unBindTexture(floorUnit);
        }

        // render tree
        {
            pipeline->bindTexture(treeUnit);
            pipeline->setModelMatrix(tree0Transform.toMatrix());
            pipeline->setVertexBuffer(&tree.m_vertices);
            pipeline->setIndexBuffer(&tree.m_indices);
            pipeline->drawIndex(RenderMode::fill);

            pipeline->bindTexture(lowPolygonTree);
            pipeline->setModelMatrix(tree1Transform.toMatrix());
            pipeline->setVertexBuffer(&tree1.m_vertices);
            pipeline->setIndexBuffer(&tree1.m_indices);
            pipeline->drawIndex(RenderMode::fill);
            pipeline->unBindTexture(0);
        }

        // render player
        {
            pipeline->bindTexture(playerUnit);
            pipeline->setModelMatrix(tPSCamera->getPlayerMatrix()
                                     * player.setSizeToMatrix(1.0,1.0,1.0));
            pipeline->setVertexBuffer(&player.m_vertices);
            pipeline->setIndexBuffer(&player.m_indices);
            pipeline->drawIndex(RenderMode::fill);
            pipeline->unBindTexture(0);
        }

        pipeline->swapBuffer();

        pipeline->endFrame();

        finish = clock();
        deltaFrameTime = (double)(finish-start)/CLOCKS_PER_SEC;

        // rotation.
        angle += 45 * deltaFrameTime;
        angle = fmod(angle, 360.0);
        test.setRotationAxis(Vector3D(0,1,0), angle);
        diabloTransform.setRotation(test);

        // send the frame.
        emit frameOut(pipeline->output(), pipeline->getProfile().num_triangles,
                      pipeline->getProfile().num_vertices);
        ++ fps;
    }
}

void RenderLoop::receiveKeyEvent(char key)
{
    fpsCamera->onKeyPress(key);
    tPSCamera->onKeyPress(key);
}

void RenderLoop::receiveMouseWheelEvent(double delta)
{
    tPSCamera->onWheelMove(delta);
}

void RenderLoop::receiveMouseEvent(double deltaX, double deltaY, std::string button)
{
    fpsCamera->onMouseMove(deltaX, deltaY, button);
    tPSCamera->onMouseMove(deltaX, deltaY, button);
}
