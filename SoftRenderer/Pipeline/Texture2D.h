#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <string>
#include "../Math/Vector2D.h"
#include "../Math/Vector4D.h"

/**
 * @projectName   SoftRenderer
 * @brief         Texture unit class.
 * @author        YangWC
 * @date          2019-05-01
 */

namespace SoftRenderer
{

class Texture2D
{
private:
    int m_width;                        // Width of picture.
    int m_height;                       // Height of picture.
    int m_channel;                      // Channel of picture.
    unsigned char *m_pixelBuffer;       // Pixels buffer.

public:
    // ctor/dtor.
    Texture2D():m_width(0), m_height(0), m_channel(0), m_pixelBuffer(nullptr){}
    ~Texture2D();

    // Texture loading.
    bool loadImage(const std::string &path);

    // Texture sampling.
    Vector4D sample(const Vector2D &texcoord) const;

};

}

#endif // TEXTURE2D_H
