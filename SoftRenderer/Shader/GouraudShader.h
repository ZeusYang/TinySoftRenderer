#ifndef GOURAUDSHADER_H
#define GOURAUDSHADER_H

#include "BaseShader.h"
#include "../Pipeline/Light.h"

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
    // Those are not created by shader.
    const Light     *m_light;       // Light.(just only one)
    const Material  *m_material;    // Mesh material.
    const Texture2D *m_unit;        // Texture unit.

    Vector3D  m_eyePos;             // Observer's position.
    Matrix4x4 m_modelMatrix;        // Model matrix.
    Matrix4x4 m_viewMatrix;         // View matrix.
    Matrix4x4 m_projectMatrix;      // Projection matrix.
    Matrix4x4 m_invModelMatrix;     // Inverse of model matrix for normal.

public:
    // ctor/dtor.
    GouraudShader();
    virtual ~GouraudShader() = default;

    // Shader stage.
    virtual VertexOut vertexShader(const Vertex &in);
    virtual Vector4D fragmentShader(const VertexOut &in);

    // Shader setting.
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
