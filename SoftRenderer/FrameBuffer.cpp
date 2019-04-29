#include "FrameBuffer.h"

namespace SoftRenderer
{

FrameBuffer::FrameBuffer(int width, int height)
    :m_channel(4)
{
    m_colorBuffer = new unsigned char[m_width*m_height*m_channel];
}

FrameBuffer::~FrameBuffer()
{
    if(m_colorBuffer)
        delete m_colorBuffer;
    m_colorBuffer = nullptr;
}

void FrameBuffer::drawPixel(unsigned int x, unsigned int y, const Vector4D &color)
{
    unsigned int index = y*m_width*m_channel + x*m_channel;
    m_colorBuffer[index + 0] = static_cast<unsigned char>(color.x*255);
    m_colorBuffer[index + 1] = static_cast<unsigned char>(color.y*255);
    m_colorBuffer[index + 2] = static_cast<unsigned char>(color.z*255);
    m_colorBuffer[index + 3] = static_cast<unsigned char>(color.w*255);
}

}
