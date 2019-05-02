#include "Texture2D.h"

#include <QDebug>
#include <QString>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define INV_SCALE 0.003921568627451

namespace SoftRenderer
{

Texture2D::~Texture2D()
{
    stbi_image_free(m_pixelBuffer);
    m_pixelBuffer = nullptr;
}

Vector4D Texture2D::sample(const Vector2D &texcoord) const
{
    // just for rgb and rgba format.
    Vector4D result(0.0,0.0,0.0,1.0);
    if(m_pixelBuffer == nullptr)
        return result;
    unsigned int x = 0, y = 0;
    // for bilinear interpolation.
    double factorU = 0, factorV = 0;

    // calculate the corresponding coordinate.
    if(texcoord.x >= 0.0f && texcoord.x <= 1.0f && texcoord.y >= 0.0f && texcoord.y <= 1.0f)
    {
        double trueU = texcoord.x * (m_width - 1);
        double trueV = texcoord.y * (m_height - 1);
        x = static_cast<unsigned int>(trueU);
        y = static_cast<unsigned int>(trueV);
        factorU = trueU - x;
        factorV = trueV - y;
    }
    else
    {
        // repeating way.
        float u = texcoord.x,v = texcoord.y;
        if(texcoord.x > 1.0f)
            u = texcoord.x - static_cast<int>(texcoord.x);
        else if(texcoord.x < 0.0f)
            u = 1.0f - (static_cast<int>(texcoord.x) - texcoord.x);
        if(texcoord.y > 1.0f)
            v = texcoord.y - static_cast<int>(texcoord.y);
        else if(texcoord.y < 0.0f)
            v = 1.0f - (static_cast<int>(texcoord.y) - texcoord.y);

        double trueU = u * (m_width - 1);
        double trueV = v * (m_height - 1);
        x = static_cast<unsigned int>(trueU);
        y = static_cast<unsigned int>(trueV);
        factorU = trueU - x;
        factorV = trueV - y;
    }
    // texel fetching.
    Vector3D texels[4];
    int index[4];
    index[0] = (x * m_width + y) * m_channel;
    if(y + 1 >= m_height)
        index[1] = index[0];
    else
        index[1] = (x * m_width + y + 1) * m_channel;
    if(y + 1 >= m_height || x + 1 >= m_width)
        index[2] = index[0];
    else
        index[2] = ((x + 1) * m_width + y + 1) * m_channel;
    if(x + 1 >= m_width)
        index[3] = index[0];
    else
        index[3] = ((x + 1) * m_width + y) * m_channel;

    // left bottom
    texels[0].x = static_cast<float>(m_pixelBuffer[index[0] + 0]) * INV_SCALE;
    texels[0].y = static_cast<float>(m_pixelBuffer[index[0] + 1]) * INV_SCALE;
    texels[0].z = static_cast<float>(m_pixelBuffer[index[0] + 2]) * INV_SCALE;

    // left top
    texels[1].x = static_cast<float>(m_pixelBuffer[index[1] + 0]) * INV_SCALE;
    texels[1].y = static_cast<float>(m_pixelBuffer[index[1] + 1]) * INV_SCALE;
    texels[1].z = static_cast<float>(m_pixelBuffer[index[1] + 2]) * INV_SCALE;

    // right top
    texels[2].x = static_cast<float>(m_pixelBuffer[index[2] + 0]) * INV_SCALE;
    texels[2].y = static_cast<float>(m_pixelBuffer[index[2] + 1]) * INV_SCALE;
    texels[2].z = static_cast<float>(m_pixelBuffer[index[2] + 2]) * INV_SCALE;

    // right bottom
    texels[3].x = static_cast<float>(m_pixelBuffer[index[3] + 0]) * INV_SCALE;
    texels[3].y = static_cast<float>(m_pixelBuffer[index[3] + 1]) * INV_SCALE;
    texels[3].z = static_cast<float>(m_pixelBuffer[index[3] + 2]) * INV_SCALE;

    // bilinear interpolation.
    // horizational
    texels[0] = texels[0] * (1.0 - factorU) + texels[3] * factorU;
    texels[1] = texels[1] * (1.0 - factorU) + texels[2] * factorU;
    //vertical
    result = texels[0] * (1.0 - factorV) + texels[1] *factorV;

    return result;
}

bool Texture2D::loadImage(const std::string &path)
{
    if(m_pixelBuffer)delete m_pixelBuffer;
    m_pixelBuffer = nullptr;
    m_pixelBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_channel, 0);
    if(m_pixelBuffer == nullptr)
    {
        qDebug() << "Failed to load image->" << QString::fromStdString(path);
    }
    return  m_pixelBuffer != nullptr;
}


}
