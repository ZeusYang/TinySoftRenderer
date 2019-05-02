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
    Vector4D litColor = in.color;
    if(m_unit)
    {
        litColor = m_unit->sample(in.texcoord);
    }
    return litColor;
}

}
