#ifndef BASESHADER_H
#define BASESHADER_H

#include "Mesh.h"
#include "Math/Matrix4x4.h"

/**
 * @projectName   SoftRenderer
 * @brief         The virtual shader.
 * @author        YangWC
 * @date          2019-04-29
 */
namespace SoftRenderer
{

class Texture2D;
class BaseShader
{
public:
    BaseShader() = default;
    virtual ~BaseShader() = default;

    virtual VertexOut vertexShader(const Vertex &in) = 0;
    virtual Vector4D fragmentShader(const VertexOut &in) = 0;
    virtual void bindShaderUnit(Texture2D *unit) = 0;
    virtual void setModelMatrix(const Matrix4x4 &world) = 0;
    virtual void setViewMatrix(const Matrix4x4 &view) = 0;
    virtual void setProjectMatrix(const Matrix4x4 &project) = 0;
};

}

#endif // BASESHADER_H
