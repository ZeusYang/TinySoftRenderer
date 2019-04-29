#include "BaseShader.h"

namespace SoftRenderer
{

VertexOut BaseShader::vertexShader(const Vertex &in)
{
    VertexOut result;
    result.posTrans = in.position;
    result.posH = in.position;
    result.color = in.color;
    result.normal = in.normal;
    result.oneDivZ = 1.0;
    result.texcoord = in.texcoord;
    return result;
}

Vector4D BaseShader::fragmentShader(const VertexOut &in)
{
    Vector4D litColor;
    litColor = in.color;
    return litColor;
}

}
