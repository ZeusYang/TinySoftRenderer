#include "RenderThread.h"

#include <QDebug>

using namespace SoftRenderer;

RenderThread::RenderThread(int w, int h, QObject *parent)
    :QThread(parent),width(w),height(h)
{
    stoped = false;
    pipeline = new Pipeline(width, height);
}

RenderThread::~RenderThread()
{
    if(pipeline) delete pipeline;
    pipeline = nullptr;
}

void RenderThread::run()
{
    pipeline->initialize();
    while(!stoped)
    {
        pipeline->clearBuffer(Vector4D(0.502f,0.698f,0.800f,1.0f));

        emit frameOut(pipeline->output());
    }
}
