#include "Pipeline.h"
#include "SimpleShader.h"

#include <QDebug>

namespace SoftRenderer
{

Pipeline::Pipeline(int width, int height)
    :m_width(width), m_height(height)
    , m_shader(nullptr), m_device(nullptr)
{

}

Pipeline::~Pipeline()
{
    if(m_shader)delete m_shader;
    if(m_device)delete m_device;
    m_shader = nullptr;
    m_device = nullptr;
}

void Pipeline::initialize()
{
    if(m_device)
        delete m_device;
    if(m_shader)
        delete m_shader;
    viewPortMatrix.setViewPort(0,0,m_width,m_height);
    m_device = new FrameBuffer(m_width, m_height);
    m_shader = new SimpleShader();
}

void Pipeline::drawIndex(RenderMode mode)
{
    if(m_indices.empty())return;

    for(unsigned int i = 0;i < m_indices.size()/3;++ i)
    {
        //! primitives assembly to triangle.
        Vertex p1,p2,p3;
        {
            p1 = m_vertices[3*i+0];
            p2 = m_vertices[3*i+1];
            p3 = m_vertices[3*i+1];
        }

        //! vertex shader stage.
        VertexOut v1,v2,v3;
        {
            v1 = m_shader->vertexShader(p1);
            v2 = m_shader->vertexShader(p2);
            v3 = m_shader->vertexShader(p3);
        }

        //! rasterization.
        {
            v1.posH = viewPortMatrix * v1.posH;
            v2.posH = viewPortMatrix * v2.posH;
            v3.posH = viewPortMatrix * v3.posH;
//            qDebug() << v1.posH.x << " " << v1.posH.y << " " << v1.posH.z;
//            qDebug() << v2.posH.x << " " << v2.posH.y << " " << v2.posH.z;
//            qDebug() << v3.posH.x << " " << v3.posH.y << " " << v3.posH.z;
        }

        //! fragment shader stage.
        {
            m_device->drawPixel(v1.posH.x,v1.posH.y,v1.color);
            m_device->drawPixel(v2.posH.x,v2.posH.y,v2.color);
            m_device->drawPixel(v3.posH.x,v3.posH.y,v3.color);
        }
    }
}

void Pipeline::clearBuffer(const Vector4D &color, bool depth)
{
    m_device->clearColorBuffer(color);
}

void Pipeline::setShaderMode(ShadingMode mode)
{
    if(m_shader)delete m_shader;
    if(mode == ShadingMode::simple)
        m_shader = new SimpleShader();
    else if(mode == ShadingMode::phong)
        ;
}


}
