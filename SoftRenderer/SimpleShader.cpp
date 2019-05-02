#include "SimpleShader.h"

#include <QDebug>

#include "Texture2D.h"

namespace SoftRenderer
{

SimpleShader::SimpleShader()
{
    m_unit = nullptr;
    m_modelMatrix.loadIdentity();
    m_viewMatrix.loadIdentity();
    m_projectMatrix.loadIdentity();
}

VertexOut SimpleShader::vertexShader(const Vertex &in)
{
    VertexOut result;
    result.posTrans = m_modelMatrix * in.position;
    result.posH = m_projectMatrix * m_viewMatrix * result.posTrans;
    result.color = in.color;
    result.normal = in.normal;
    result.texcoord = in.texcoord;

    // oneDivZ to correct mapping.
    result.oneDivZ = 1.0 / result.posH.w;
    result.posTrans *= result.oneDivZ;
    result.texcoord *= result.oneDivZ;
    result.color *= result.oneDivZ;
    return result;
}

Vector4D SimpleShader::fragmentShader(const VertexOut &in)
{
    Vector4D litColor;
    litColor = in.color;
    if(m_unit)
    {
        litColor = m_unit->sample(in.texcoord);
    }
    return litColor;
}

void SimpleShader::bindShaderUnit(Texture2D *unit)
{
    m_unit = unit;
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
