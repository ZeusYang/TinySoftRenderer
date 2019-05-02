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

struct Profile
{
public:
    unsigned int num_triangles;
    unsigned int num_vertices;
    Profile():num_triangles(0),num_vertices(0) {}
    void setZero(){num_triangles = num_vertices = 0;}
};

class Texture2D;
class Pipeline
{
private:
    Profile m_profile;                          // show some infomations.
    Vector3D m_eyePos;                          // camera position.
    RenderMode m_mode;                          // wire or fill.
    int m_width, m_height;                      // width and height of viewport.
    BaseShader *m_shader;                       // shaders including vertex shader and fragment shader.
    FrameBuffer *m_frontBuffer;                 // the frame buffer that is going to be shown.
    FrameBuffer *m_backBuffer;                  // the frame buffer that is goint to be written.
    Matrix4x4 viewPortMatrix;                   // viewport transformation matrix.
    const std::vector<Vertex> *m_vertices;      // vertex buffer.
    const std::vector<unsigned int> *m_indices; // index buffer.
    std::vector<Texture2D*> m_textureUnits;     // texture units.

public:
    Pipeline(int width, int height);
    ~Pipeline();

    void initialize();

    void beginFrame();

    void endFrame();

    unsigned int loadTexture(const std::string &path);

    bool bindTexture(const unsigned int &unit);

    bool unBindTexture(const unsigned int &unit);

    void setViewPort(int left, int top, int width, int height);

    void setModelMatrix(Matrix4x4 modelMatrix);

    void setViewMatrix(Vector3D eye, Vector3D target, Vector3D up);

    void setProjectMatrix(float fovy, float aspect, float near, float far);

    void clearBuffer(const Vector4D &color);

    void setVertexBuffer(const std::vector<Vertex> *vertices){m_vertices = vertices;}

    void setIndexBuffer(const std::vector<unsigned int> *indices){m_indices = indices;}

    void setShaderMode(ShadingMode mode);

    void drawIndex(RenderMode mode);

    void swapBuffer();

    unsigned char *output(){return m_frontBuffer->getColorBuffer();}

    Profile getProfile(){return m_profile;}


private:

    void perspectiveDivision(VertexOut &target);

    VertexOut lerp(const VertexOut &n1, const VertexOut &n2, double weight);

    bool lineCliping(const VertexOut &from, const VertexOut &to);

    bool triangleCliping(const VertexOut &v1, const VertexOut &v2, const VertexOut &v3);

    bool backFaceCulling(const Vector4D &v1, const Vector4D &v2, const Vector4D &v3);

    void bresenhamLineRasterization(const VertexOut &from, const VertexOut &to);

    void scanLinePerRow(const VertexOut &left, const VertexOut &right);

    void rasterTopTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3);

    void rasterBottomTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3);

    void edgeWalkingFillRasterization(const VertexOut &v1, const VertexOut &v2, const VertexOut &v3);

};

}

#endif // PIPELINE_H
