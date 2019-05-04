#ifndef SIMPLESHADER_H
#define SIMPLESHADER_H

#include "BaseShader.h"
#include "../Pipeline/Light.h"
#include "../Math/Matrix4x4.h"
/**
 * @projectName   SoftRenderer
 * @brief         A simple shader.
 * @author        YangWC
 * @date          2019-04-30
 */
namespace SoftRenderer
{

class SimpleShader : public BaseShader
{
private:
    const Texture2D *m_unit;        // Texture unit.
    Matrix4x4 m_modelMatrix;        // Model matrix.
    Matrix4x4 m_viewMatrix;         // View matrix.
    Matrix4x4 m_projectMatrix;      // Projection matrix.

public:
    // ctor/dtor.
    SimpleShader();
    virtual ~SimpleShader() = default;

    // Shader stage.
    virtual VertexOut vertexShader(const Vertex &in);
    virtual Vector4D fragmentShader(const VertexOut &in);

    // Shader setting.
    virtual void bindShaderUnit(Texture2D *unit){m_unit = unit;}
    virtual void setModelMatrix(const Matrix4x4 &world){m_modelMatrix = world;}
    virtual void setViewMatrix(const Matrix4x4 &view){m_viewMatrix = view;}
    virtual void setProjectMatrix(const Matrix4x4 &project){m_projectMatrix = project;}
    virtual void setMaterial(const Material *material){}
    virtual void setLight(const Light *light) {}
    virtual void setEyePos(const Vector3D eye){}
};

}

#endif // SIMPLESHADER_H
