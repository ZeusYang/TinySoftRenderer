#ifndef RENDERLOOP_H
#define RENDERLOOP_H

#include <QObject>

#include "SoftRenderer/Pipeline.h"
#include "SoftRenderer/FPSCamera.h"

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

    void receiveMouseEvent(double deltaX, double deltaY);

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
    SoftRenderer::Pipeline *pipeline;
    int width, height, channel;
};

#endif // RENDERLOOP_H
