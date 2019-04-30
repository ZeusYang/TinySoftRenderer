#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>

#include "SoftRenderer/Pipeline.h"

/**
 * @projectName   SoftRenderer
 * @brief         the thread that actually run renderer.
 * @author        YangWC
 * @date          2019-04-29
 */
class RenderThread : public QThread
{
    Q_OBJECT
public:
    RenderThread(int w, int h, QObject *parent = nullptr);

    ~RenderThread();

    void stopIt()
    {
        stoped = true;
    }

signals:
    void frameOut(unsigned char *image);

protected:
    void run() override;

private:
    bool stoped;
    SoftRenderer::Pipeline *pipeline;
    int width, height, channel;
};

#endif // RENDERTHREAD_H
