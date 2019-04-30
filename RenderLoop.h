#ifndef RENDERLOOP_H
#define RENDERLOOP_H

#include <QObject>

#include "SoftRenderer/Pipeline.h"

class RenderLoop : public QObject
{
    Q_OBJECT
public:
    explicit RenderLoop(int w, int h, QObject *parent = nullptr);
    ~RenderLoop();

    void stopIt() {stoped = true;}

signals:
    void frameOut(unsigned char *image);

public slots:
    void loop();

private:
    bool stoped;
    SoftRenderer::Pipeline *pipeline;
    int width, height, channel;
};

#endif // RENDERLOOP_H
