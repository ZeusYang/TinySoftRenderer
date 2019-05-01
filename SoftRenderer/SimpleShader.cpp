#include "SimpleShader.h"

namespace SoftRenderer
{

SimpleShader::SimpleShader()
{
    m_modelMatrix.loadIdentity();
}

VertexOut SimpleShader::vertexShader(const Vertex &in)
{
    VertexOut result;
    result.posTrans = m_modelMatrix * in.position;
    result.posH = m_projectMatrix * m_viewMatrix * m_modelMatrix * in.position;
    result.color = in.color;
    result.normal = in.normal;
    result.oneDivZ = 1.0f / result.posH.w;
    result.texcoord = in.texcoord;
    return result;
}

Vector4D SimpleShader::fragmentShader(const VertexOut &in)
{
    Vector4D litColor;
    litColor = in.color;
    return litColor;
}

void SimpleShader::setModelMatrix(const Matrix4x4 &world)
{
    m_modelMatrix = world;
}

void SimpleShader::setViewMatrix(const Matrix4x4 &view)
{
    m_viewMatrix = view;
}

void SimpleShader::setProjectMatrix(const Matrix4x4 &project)
{
    m_projectMatrix = project;
}

}
