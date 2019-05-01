#ifndef SIMPLESHADER_H
#define SIMPLESHADER_H

#include "BaseShader.h"
#include "Math/Matrix4x4.h"

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
    Matrix4x4 m_modelMatrix;
    Matrix4x4 m_viewMatrix;
    Matrix4x4 m_projectMatrix;

public:
    SimpleShader();
    virtual ~SimpleShader() = default;

    virtual VertexOut vertexShader(const Vertex &in);
    virtual Vector4D fragmentShader(const VertexOut &in);
    virtual void setModelMatrix(const Matrix4x4 &world);
    virtual void setViewMatrix(const Matrix4x4 &view);
    virtual void setProjectMatrix(const Matrix4x4 &project);
};

}

#endif // SIMPLESHADER_H
