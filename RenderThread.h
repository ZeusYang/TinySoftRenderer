#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>

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
    int width, height, channel;
    unsigned char *pixels;
};

#endif // RENDERTHREAD_H
