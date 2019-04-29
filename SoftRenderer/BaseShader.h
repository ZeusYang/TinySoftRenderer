#ifndef BASESHADER_H
#define BASESHADER_H

#include "Mesh.h"

/**
 * @projectName   SoftRenderer
 * @brief         The basic shader.
 * @author        YangWC
 * @date          2019-04-29
 */
namespace SoftRenderer
{

class BaseShader
{
public:
    BaseShader() = default;
    virtual ~BaseShader() = default;

    virtual VertexOut vertexShader(const Vertex &in);
    virtual Vector4D fragmentShader(const VertexOut &in);

};

}

#endif // BASESHADER_H
