#ifndef GOURAUDSHADER_H
#define GOURAUDSHADER_H

#include "Light.h"
#include "BaseShader.h"

/**
 * @projectName   SoftRenderer
 * @brief         Gouraud shading.
 * @author        YangWC
 * @date          2019-05-02
 */

namespace SoftRenderer
{

class GouraudShader : public BaseShader
{
private:
    const Light     *m_light;
    const Material  *m_material;
    const Texture2D *m_unit;
    Vector3D  m_eyePos;
    Matrix4x4 m_modelMatrix;
    Matrix4x4 m_viewMatrix;
    Matrix4x4 m_projectMatrix;
    Matrix4x4 m_invModelMatrix;

public:
    GouraudShader();

    virtual ~GouraudShader() = default;

    virtual VertexOut vertexShader(const Vertex &in);
    virtual Vector4D fragmentShader(const VertexOut &in);
    virtual void bindShaderUnit(Texture2D *unit){m_unit = unit;}
    virtual void setModelMatrix(const Matrix4x4 &world)
    {m_modelMatrix = world;m_invModelMatrix = m_modelMatrix.getInverseTranspose();}
    virtual void setViewMatrix(const Matrix4x4 &view){m_viewMatrix = view;}
    virtual void setProjectMatrix(const Matrix4x4 &project){m_projectMatrix = project;}
    virtual void setMaterial(const Material *material){m_material = material;}
    virtual void setLight(const Light *light){m_light = light;}
    virtual void setEyePos(const Vector3D eye){m_eyePos = eye;}
};

}

#endif // GOURAUDSHADER_H
