#ifndef RENDERLOOP_H
#define RENDERLOOP_H

#include <QObject>

#include "SoftRenderer/Pipeline/Pipeline.h"
#include "SoftRenderer/Camera/FPSCamera.h"
#include "SoftRenderer/Camera/TPSCamera.h"

class RenderLoop : public QObject
{
    Q_OBJECT
public:
    explicit RenderLoop(int w, int h, QObject *parent = nullptr);
    ~RenderLoop();

    void stopIt() {stoped = true;}

    void setFpsZero(){fps = 0;}

    int getFps(){return fps;}

    void receiveKeyEvent(char key);
    void receiveMouseWheelEvent(double delta);
    void receiveMouseEvent(double deltaX, double deltaY, std::string button);

signals:
    void frameOut(unsigned char *image, const unsigned int &num_triangles,
                  const unsigned int &num_vertices);

public slots:
    void loop();

private:
    bool stoped;
    int fps;
    double deltaFrameTime;
    SoftRenderer::FPSCamera *fpsCamera;
    SoftRenderer::TPSCamera *tPSCamera;
    SoftRenderer::Pipeline *pipeline;
    int width, height, channel;
};

#endif // RENDERLOOP_H
