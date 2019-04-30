#include "SimpleShader.h"

namespace SoftRenderer
{

VertexOut SimpleShader::vertexShader(const Vertex &in)
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

Vector4D SimpleShader::fragmentShader(const VertexOut &in)
{
    Vector4D litColor;
    litColor = in.color;
    return litColor;
}

void SimpleShader::setModelMatrix(const Matrix4x4 &world)
{

}

void SimpleShader::setViewMatrix(const Matrix4x4 &view)
{

}

void SimpleShader::setProjectMatrix(const Matrix4x4 &project)
{

}



}
