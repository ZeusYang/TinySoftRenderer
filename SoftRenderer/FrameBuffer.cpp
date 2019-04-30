#include "FrameBuffer.h"

#include <QDebug>
//#include "tbb/parallel_for.h"

//using namespace tbb;

namespace SoftRenderer
{

FrameBuffer::FrameBuffer(int width, int height)
    :m_channel(4), m_width(width), m_height(height)
{
    m_colorBuffer.resize(m_width*m_height*m_channel, 255);
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::clearColorBuffer(const Vector4D &color)
{
    // fill the color buffer.
    unsigned char red = static_cast<unsigned char>(255*color.x);
    unsigned char green = static_cast<unsigned char>(255*color.y);
    unsigned char blue = static_cast<unsigned char>(255*color.z);
    unsigned char alpha = static_cast<unsigned char>(255*color.w);

    for(int row = 0;row < m_height;++ row)
    {
        for(int col = 0;col < m_width;++ col)
        {
            m_colorBuffer[row*m_width*m_channel+col*m_channel + 0] = red;
            m_colorBuffer[row*m_width*m_channel+col*m_channel + 1] = green;
            m_colorBuffer[row*m_width*m_channel+col*m_channel + 2] = blue;
            m_colorBuffer[row*m_width*m_channel+col*m_channel + 3] = alpha;
        }
    }
}

void FrameBuffer::drawPixel(unsigned int x, unsigned int y, const Vector4D &color)
{
    if(x < 0 || x >= m_width || y < 0 || y >= m_height)
        return;
    unsigned char red = static_cast<unsigned char>(255*color.x);
    unsigned char green = static_cast<unsigned char>(255*color.y);
    unsigned char blue = static_cast<unsigned char>(255*color.z);
    unsigned char alpha = static_cast<unsigned char>(255*color.w);
    unsigned int index = y*m_width*m_channel + x*m_channel;
    m_colorBuffer[index + 0] = red;
    m_colorBuffer[index + 1] = green;
    m_colorBuffer[index + 2] = blue;
    m_colorBuffer[index + 3] = alpha;
}

}
