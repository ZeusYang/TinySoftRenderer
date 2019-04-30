#ifndef PIPELINE_H
#define PIPELINE_H

#include "BaseShader.h"
#include "FrameBuffer.h"

/**
 * @projectName   SoftRenderer
 * @brief         Render pipeline class.
 * @author        YangWC
 * @date          2019-04-30
 */
namespace SoftRenderer
{

enum ShadingMode
{
    simple = 0,
    phong = 1
};

enum RenderMode
{
    wire = 0,
    fill =1
};

class Pipeline
{
private:
    int m_width, m_height;                  // width and height of viewport.
    BaseShader *m_shader;                   // shaders including vertex shader and fragment shader.
    FrameBuffer *m_frontBuffer;
    FrameBuffer *m_backBuffer;
    Matrix4x4 viewPortMatrix;               // viewport transformation matrix.
    std::vector<Vertex> m_vertices;         // vertex buffer.
    std::vector<unsigned int> m_indices;    // index buffer.

public:
    Pipeline(int width, int height);
    ~Pipeline();

    void initialize();

    void drawIndex(RenderMode mode);

    void clearBuffer(const Vector4D &color, bool depth = false);

    void setVertexBuffer(const std::vector<Vertex> &vertices){m_vertices = vertices;}

    void setIndexBuffer(const std::vector<unsigned int> &indices){m_indices = indices;}

    void setShaderMode(ShadingMode mode);

    unsigned char *output(){return m_frontBuffer->getColorBuffer();}

    void swapBuffer();

private:

    VertexOut lerp(const VertexOut &n1, const VertexOut &n2, double weight);

    void bresenhamLineRasterization(const VertexOut &from, const VertexOut &to);

    void scanLinePerRow(const VertexOut &left, const VertexOut &right);

    void rasterTopTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3);

    void rasterBottomTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3);

    void edgeWalkingFillRasterization(const VertexOut &v1, const VertexOut &v2, const VertexOut &v3);

    void rasterization();

};

}

#endif // PIPELINE_H
