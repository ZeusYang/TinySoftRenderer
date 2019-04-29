#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Math/Vector4D.h"

/**
 * @projectName   SoftRenderer
 * @brief         Frame buffer class.
 * @author        YangWC
 * @date          2019-04-29
 */

namespace SoftRenderer
{

class FrameBuffer
{
private:
    int m_width, m_height, m_channel;
    unsigned char *m_colorBuffer;

public:
    FrameBuffer(int width, int height);
    ~FrameBuffer();

    int getWidth(){return m_width;}

    int getHeight(){return m_height;}

    void drawPixel(unsigned int x, unsigned int y, const Vector4D &color);

};

}

#endif // FRAMEBUFFER_H
