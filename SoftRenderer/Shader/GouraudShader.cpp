#include "GouraudShader.h"

#include <QDebug>
#include "../Pipeline/Texture2D.h"

namespace SoftRenderer
{

GouraudShader::GouraudShader()
{
    m_unit = nullptr;
    m_light = nullptr;
    m_material = nullptr;
    m_modelMatrix.loadIdentity();
    m_viewMatrix.loadIdentity();
    m_projectMatrix.loadIdentity();
}

VertexOut GouraudShader::vertexShader(const Vertex &in)
{
    VertexOut result;
    result.posTrans = m_modelMatrix * in.position;
    result.posH = m_projectMatrix * m_viewMatrix * result.posTrans;
    result.color = in.color;
    result.texcoord = in.texcoord;
    result.normal = m_invModelMatrix * Vector4D(in.normal);

    // Gouraud shading.
    if(m_unit)
        result.color = m_unit->sample(result.texcoord);
    Vector3D _amb, _diff, _spec;
    if(m_light)
    {
        Vector3D eyeDir = m_eyePos - result.posTrans;
        eyeDir.normalize();
        m_light->lighting(*m_material,
                          result.posTrans,
                          result.normal,
                          eyeDir,
                          _amb,
                          _diff,
                          _spec);

        result.color.x *= (_amb.x + _diff.x + _spec.x);
        result.color.y *= (_amb.y + _diff.y + _spec.y);
        result.color.z *= (_amb.z + _diff.z + _spec.z);
        result.color.w = 1.0f;
    }

    // oneDivZ to correct lerp.
    result.oneDivZ = 1.0 / result.posH.w;
    result.posTrans *= result.oneDivZ;
    result.texcoord *= result.oneDivZ;
    result.color *= result.oneDivZ;
    return result;

}

Vector4D GouraudShader::fragmentShader(const VertexOut &in)
{
    Vector4D litColor = in.color;
    return litColor;
}


}
