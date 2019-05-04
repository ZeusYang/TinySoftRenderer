#include "PhongShader.h"

#include "../Pipeline/Light.h"
#include "../Pipeline/Texture2D.h"

namespace SoftRenderer
{

PhongShader::PhongShader()
{
    m_unit = nullptr;
    m_light = nullptr;
    m_material = nullptr;
    m_modelMatrix.loadIdentity();
    m_viewMatrix.loadIdentity();
    m_projectMatrix.loadIdentity();
    m_invModelMatrix.loadIdentity();
}

VertexOut PhongShader::vertexShader(const Vertex &in)
{
    VertexOut result;
    result.posTrans = m_modelMatrix * in.position;
    result.posH = m_projectMatrix * m_viewMatrix * result.posTrans;
    result.color = in.color;
    result.texcoord = in.texcoord;
    result.normal = m_invModelMatrix * Vector4D(in.normal);

    // oneDivZ to correct lerp.
    result.oneDivZ = 1.0 / result.posH.w;
    result.posTrans *= result.oneDivZ;
    result.texcoord *= result.oneDivZ;
    result.color *= result.oneDivZ;
    return result;
}

Vector4D PhongShader::fragmentShader(const VertexOut &in)
{
    Vector4D litColor = in.color;

    // Gouraud shading.
    if(m_unit)
        litColor = m_unit->sample(in.texcoord);
    Vector3D _amb, _diff, _spec;
    if(m_light)
    {
        Vector3D eyeDir = m_eyePos - in.posTrans;
        eyeDir.normalize();
        m_light->lighting(*m_material,
                          in.posTrans,
                          in.normal,
                          eyeDir,
                          _amb,
                          _diff,
                          _spec);

        litColor.x *= (_amb.x + _diff.x + _spec.x);
        litColor.y *= (_amb.y + _diff.y + _spec.y);
        litColor.z *= (_amb.z + _diff.z + _spec.z);
        litColor.w = 1.0f;
    }
    return litColor;
}

}
