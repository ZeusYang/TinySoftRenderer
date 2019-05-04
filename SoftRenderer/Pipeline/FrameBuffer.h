#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include "../Math/Vector4D.h"

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
    std::vector<double> m_depthBuffer;          // Z-buffer.
    unsigned int m_width, m_height, m_channel;  // Property of viewport.
    std::vector<unsigned char> m_colorBuffer;   // Color buffer.

public:
    // ctor/dtor.
    FrameBuffer(int width, int height);
    ~FrameBuffer() = default;

    // Clear
    void clearColorAndDepthBuffer(const Vector4D &color);

    // Getter.
    int getWidth()const {return m_width;}
    int getHeight()const {return m_height;}
    unsigned char *getColorBuffer() {return m_colorBuffer.data();}
    double getDepth(const unsigned int &x, const unsigned int &y)const;

    // Drawer.
    void drawDepth(const unsigned int &x, const unsigned int &y, const double &value);
    void drawPixel(const unsigned int &x, const unsigned int &y, const Vector4D &color);

};

}

#endif // FRAMEBUFFER_H
