#include "Pipeline.h"
#include "SimpleShader.h"

#include <QDebug>

namespace SoftRenderer
{

Pipeline::Pipeline(int width, int height)
    :m_width(width),m_height(height)
    ,m_shader(nullptr),m_frontBuffer(nullptr)
    ,m_backBuffer(nullptr)
{
}

Pipeline::~Pipeline()
{
    if(m_shader)delete m_shader;
    if(m_frontBuffer)delete m_frontBuffer;
    if(m_backBuffer)delete m_backBuffer;
    m_shader = nullptr;
    m_frontBuffer = nullptr;
    m_backBuffer = nullptr;
}

void Pipeline::initialize()
{
    if(m_frontBuffer)
        delete m_frontBuffer;
    if(m_backBuffer)
        delete m_backBuffer;
    if(m_shader)
        delete m_shader;
    viewPortMatrix.setViewPort(0,0,m_width,m_height);
    m_frontBuffer = new FrameBuffer(m_width, m_height);
    m_backBuffer = new FrameBuffer(m_width, m_height);
    m_shader = new SimpleShader();
}

void Pipeline::setViewPort(int left, int top, int width, int height)
{
    viewPortMatrix.setViewPort(left, top, width, height);
}

void Pipeline::setModelMatrix(Matrix4x4 modelMatrix)
{
    m_shader->setModelMatrix(modelMatrix);
}

void Pipeline::setViewMatrix(Vector3D eye, Vector3D target, Vector3D up)
{
    Matrix4x4 viewMatrix;
    viewMatrix.setLookAt(eye, target, up);
    m_shader->setViewMatrix(viewMatrix);
}

void Pipeline::setProjectMatrix(float fovy, float aspect, float near, float far)
{
    Matrix4x4 projectMatrix;
    projectMatrix.setPerspective(fovy, aspect, near, far);
    m_shader->setProjectMatrix(projectMatrix);
}

void Pipeline::drawIndex(RenderMode mode)
{
    if(m_indices.empty())return;

    for(unsigned int i = 0;i < m_indices.size()/3;++ i)
    {
        //! assembly to triangle primitive.
        Vertex p1,p2,p3;
        {
            p1 = m_vertices[3*i+0];
            p2 = m_vertices[3*i+1];
            p3 = m_vertices[3*i+2];
        }

        //! vertex shader stage.
        VertexOut v1,v2,v3;
        {
            v1 = m_shader->vertexShader(p1);
            v2 = m_shader->vertexShader(p2);
            v3 = m_shader->vertexShader(p3);
        }

        //! perspective division.
        {
            perspectiveDivision(v1);
            perspectiveDivision(v2);
            perspectiveDivision(v3);
        }

        //! view port transformation.
        {
            v1.posH = viewPortMatrix * v1.posH;
            v2.posH = viewPortMatrix * v2.posH;
            v3.posH = viewPortMatrix * v3.posH;
        }

        //! rasterization and fragment shader stage.
        {
            if(mode == RenderMode::wire)
            {
                bresenhamLineRasterization(v1,v2);
                bresenhamLineRasterization(v2,v3);
                bresenhamLineRasterization(v3,v1);
            }
            else if(mode == RenderMode::fill)
            {
                edgeWalkingFillRasterization(v1,v2,v3);
            }
        }

    }
}

void Pipeline::clearBuffer(const Vector4D &color, bool depth)
{
    (void)depth;
    m_backBuffer->clearColorBuffer(color);
}

void Pipeline::setShaderMode(ShadingMode mode)
{
    if(m_shader)delete m_shader;
    if(mode == ShadingMode::simple)
        m_shader = new SimpleShader();
    else if(mode == ShadingMode::phong)
        ;
}

void Pipeline::swapBuffer()
{
    FrameBuffer *tmp = m_frontBuffer;
    m_frontBuffer = m_backBuffer;
    m_backBuffer = tmp;
}

void Pipeline::perspectiveDivision(VertexOut &target)
{
    target.posH.x /= target.posH.w;
    target.posH.y /= target.posH.w;
    target.posH.z /= target.posH.w;
    target.posH.w = 1.0f;
}

VertexOut Pipeline::lerp(const VertexOut &n1, const VertexOut &n2, double weight)
{
    VertexOut result;
    result.posTrans = n1.posTrans.lerp(n2.posTrans, weight);
    result.posH = n1.posH.lerp(n2.posH, weight);
    result.color = n1.color.lerp(n2.color, weight);
    result.normal = n1.normal.lerp(n2.normal, weight);
    result.texcoord = n1.texcoord.lerp(n2.texcoord, weight);
    result.oneDivZ = n1.oneDivZ*(1-weight) + n2.oneDivZ;
    return result;
}

void Pipeline::bresenhamLineRasterization(const VertexOut &from, const VertexOut &to)
{
    int dx = to.posH.x - from.posH.x;
    int dy = to.posH.y - from.posH.y;
    int stepX = 1, stepY = 1;

    // judge the sign
    if(dx < 0)
    {
        stepX = -1;
        dx = -dx;
    }
    if(dy < 0)
    {
        stepY = -1;
        dy = -dy;
    }

    int d2x = 2*dx, d2y = 2*dy;
    int d2y_minus_d2x = d2y - d2x;
    int sx = from.posH.x;
    int sy = from.posH.y;

    VertexOut tmp;
    // slope < 1.
    if(dy <= dx)
    {
        int flag = d2y - dx;
        for(int i = 0;i <= dx;++ i)
        {
            // linear interpolation
            tmp = lerp(from, to, static_cast<double>(i)/dx);
            // fragment shader
            m_backBuffer->drawPixel(sx,sy,m_shader->fragmentShader(tmp));
            sx += stepX;
            if(flag <= 0)
                flag += d2y;
            else
            {
                sy += stepY;
                flag += d2y_minus_d2x;
            }
        }
    }
    // slope > 1.
    else
    {
        int flag = d2x - dy;
        for(int i = 0;i <= dy;++ i)
        {
            // linear interpolation
            tmp = lerp(from, to, static_cast<double>(i)/dy);
            // fragment shader
            m_backBuffer->drawPixel(sx,sy,m_shader->fragmentShader(tmp));
            sy += stepY;
            if(flag <= 0)
                flag += d2x;
            else
            {
                sx += stepX;
                flag -= d2y_minus_d2x;
            }
        }
    }
}

void Pipeline::scanLinePerRow(const VertexOut &left, const VertexOut &right)
{
    VertexOut current;
    int length = right.posH.x - left.posH.x + 1;
    for(int i = 0;i <= length;++i)
    {
        // linear interpolation
        double weight = static_cast<double>(i)/length;
        current = lerp(left, right, weight);
        current.posH.x = left.posH.x + i;
        current.posH.y = left.posH.y;
        // fragment shader
        m_backBuffer->drawPixel(current.posH.x, current.posH.y,
                                m_shader->fragmentShader(current));
    }
}

void Pipeline::rasterTopTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3)
{
    VertexOut left = v2;
    VertexOut right = v3;
    VertexOut dest = v1;
    VertexOut tmp, newleft, newright;
    if(left.posH.x > right.posH.x)
    {
        tmp = left;
        left = right;
        right = tmp;
    }
    int dy = left.posH.y - dest.posH.y + 1;

    for(int i = 0;i < dy;++i)
    {
        double weight = 0;
        if(dy != 0)
            weight = static_cast<double>(i)/dy;
        newleft = lerp(left, dest, weight);
        newright = lerp(right, dest, weight);
        newleft.posH.y = newright.posH.y = left.posH.y - i;
        scanLinePerRow(newleft, newright);
    }
}

void Pipeline::rasterBottomTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3)
{
    VertexOut left = v1;
    VertexOut right = v2;
    VertexOut dest = v3;
    VertexOut tmp, newleft, newright;
    if(left.posH.x > right.posH.x)
    {
        tmp = left;
        left = right;
        right = tmp;
    }
    int dy = dest.posH.y - left.posH.y + 1;


    for(int i = 0;i < dy;++i)
    {
        double weight = 0;
        if(dy != 0)
            weight = static_cast<double>(i)/dy;
        newleft = lerp(left, dest, weight);
        newright = lerp(right, dest, weight);
        newleft.posH.y = newright.posH.y = left.posH.y + i;
        scanLinePerRow(newleft, newright);
    }
}

void Pipeline::edgeWalkingFillRasterization(const VertexOut &v1, const VertexOut &v2, const VertexOut &v3)
{
    // split the triangle into two part
    VertexOut tmp;
    VertexOut target[3] = {v1, v2,v3};
    if(target[0].posH.y > target[1].posH.y)
    {
        tmp = target[0];
        target[0] = target[1];
        target[1] = tmp;
    }
    if(target[0].posH.y > target[2].posH.y)
    {
        tmp = target[0];
        target[0] = target[2];
        target[2] = tmp;
    }
    if(target[1].posH.y > target[2].posH.y)
    {
        tmp = target[1];
        target[1] = target[2];
        target[2] = tmp;
    }

    // bottom triangle
    if(equal(target[0].posH.y,target[1].posH.y))
    {
        rasterBottomTriangle(target[0],target[1],target[2]);
    }
    // top triangle
    else if(equal(target[1].posH.y,target[2].posH.y))
    {
        rasterTopTriangle(target[0], target[1], target[2]);
    }
    // split it.
    else
    {
        double weight = static_cast<double>(target[1].posH.y-target[0].posH.y)/(target[2].posH.y-target[0].posH.y);
        VertexOut newPoint = lerp(target[0],target[2],weight);
        newPoint.posH.y = target[1].posH.y;
        rasterTopTriangle(target[0], newPoint, target[1]);
        rasterBottomTriangle(newPoint,target[1],target[2]);
    }

}


}
