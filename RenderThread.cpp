#include "RenderThread.h"

#include <QDebug>

RenderThread::RenderThread(int w, int h, QObject *parent)
    :QThread(parent),width(w),height(h)
{
    stoped = false;
    channel = 4;
    pixels = new unsigned char[width * height * channel];
}

RenderThread::~RenderThread()
{
    if(pixels)
        delete pixels;
    pixels = nullptr;
}

void RenderThread::run()
{
    while(!stoped)
    {
        for(int row = 0;row < height;++ row)
        {
            for(int col = 0;col < width;++ col)
            {
                pixels[row*width*channel+col*channel + 0] = static_cast<unsigned char>(255);
                pixels[row*width*channel+col*channel + 1] = static_cast<unsigned char>(0);
                pixels[row*width*channel+col*channel + 2] = static_cast<unsigned char>(0);
                pixels[row*width*channel+col*channel + 3] = static_cast<unsigned char>(255);
            }
        }

        emit frameOut(pixels);
    }
}
