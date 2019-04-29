#ifndef MESH_H
#define MESH_H

#include <vector>
#include "Math/Vector2D.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"

/**
 * @projectName   SoftRenderer
 * @brief         Mesh class.
 * @author        YangWC
 * @date          2019-04-29
 */
namespace SoftRenderer
{
class Vertex
{
public:
    Vector4D position;
    Vector4D color;
    Vector2D texcoord;
    Vector3D normal;

    Vertex() = default;
    Vertex(Vector4D _pos, Vector4D _color, Vector2D _tex, Vector3D _normal)
        :position(_pos),color(_color),texcoord(_tex),normal(_normal) {}
    Vertex(const Vertex &rhs)
        :position(rhs.position),color(rhs.color),texcoord(rhs.texcoord),normal(rhs.normal){}
};

class VertexOut
{
public:
    Vector4D posTrans;  //世界变换后的坐标
    Vector4D posH;      //投影变换后的坐标
    Vector2D texcoord;  //纹理坐标
    Vector3D normal;	//法线
    Vector4D color;	    //颜色
    double oneDivZ;     //1/z用于深度测试
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Mesh() = default;

    ~Mesh() = default;

    Mesh(const Mesh& mesh)
        :vertices(mesh.vertices), indices(mesh.indices){}

    Mesh& operator=(const Mesh& mesh)
    {
        if (&mesh == this)
            return *this;
        vertices = mesh.vertices;
        indices = mesh.indices;
        return *this;
    }


    void setVertices(Vertex* _vs, int count)
    {
        vertices.resize(count);
        new(&vertices[0])std::vector<Vertex>(_vs, _vs + count);
    }

    void setIndices(int* _es, int count)
    {
        indices.resize(count);
        new(&indices)std::vector<unsigned int>(_es, _es + count);
    }

    void asBox(double width, double height, double depth);

    void asTriangle(const Vector3D p1, const Vector3D p2, const Vector3D p3);
};

}

#endif // MESH_H
