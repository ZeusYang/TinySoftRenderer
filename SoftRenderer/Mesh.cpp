#include "Mesh.h"

namespace SoftRenderer
{

void Mesh::asBox(double width, double height, double depth)
{
    m_vertices.resize(24);
    m_indices.resize(36);

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    float halfD = depth * 0.5f;
    //front
    m_vertices[0].position = Vector3D(halfW, halfH, halfD);
    m_vertices[0].normal = Vector3D(0.f, 0.f, 1.f);
    m_vertices[0].color = Vector4D(1.f, 0.f, 0.f, 1.f);
    m_vertices[0].texcoord = Vector2D(1.f, 1.f);
    m_vertices[1].position = Vector3D(-halfW, halfH, halfD);
    m_vertices[1].normal = Vector3D(0.f, 0.f, 1.f);
    m_vertices[1].color = Vector4D(0.f, 1.f, 0.f, 1.f);
    m_vertices[1].texcoord = Vector2D(0.f, 1.f);
    m_vertices[2].position = Vector3D(-halfW,-halfH, halfD);
    m_vertices[2].normal = Vector3D(0.f, 0.f, 1.f);
    m_vertices[2].color = Vector4D(0.f, 0.f, 1.f, 1.f);
    m_vertices[2].texcoord = Vector2D(0.f, 0.f);
    m_vertices[3].position = Vector3D(halfW, -halfH, halfD);
    m_vertices[3].normal = Vector3D(0.f, 0.f, 1.f);
    m_vertices[3].color = Vector4D(0.f, 1.f, 1.f, 1.f);
    m_vertices[3].texcoord = Vector2D(1.f, 0.f);
    //left
    m_vertices[4].position = Vector3D(-halfW, +halfH, halfD);
    m_vertices[4].normal = Vector3D(-1.f, 0.f, 0.f);
    m_vertices[4].color = Vector4D(0.f, 0.f, 1.f, 1.f);
    m_vertices[4].texcoord = Vector2D(1.f, 1.f);
    m_vertices[5].position = Vector3D(-halfW, +halfH, -halfD);
    m_vertices[5].normal = Vector3D(-1.f, 0.f, 0.f);
    m_vertices[5].color = Vector4D(1.f, 1.f, 0.f, 1.f);
    m_vertices[5].texcoord = Vector2D(0.f, 1.f);
    m_vertices[6].position = Vector3D(-halfW, -halfH, -halfD);
    m_vertices[6].normal = Vector3D(-1.f, 0.f, 0.f);
    m_vertices[6].color = Vector4D(0.f, 1.f, 0.f, 1.f);
    m_vertices[6].texcoord = Vector2D(0.f, 0.f);
    m_vertices[7].position = Vector3D(-halfW, -halfH, halfD);
    m_vertices[7].normal = Vector3D(-1.f, 0.f, 0.f);
    m_vertices[7].color = Vector4D(1.f, 1.f, 1.f, 1.f);
    m_vertices[7].texcoord = Vector2D(1.f, 0.f);
    //back
    m_vertices[8].position = Vector3D(-halfW, +halfH, -halfD);
    m_vertices[8].normal = Vector3D(0.f, 0.f, -1.f);
    m_vertices[8].color = Vector4D(1.f, 0.f, 1.f, 1.f);
    m_vertices[8].texcoord = Vector2D(0.f, 0.f);
    m_vertices[9].position = Vector3D(+halfW, +halfH, -halfD);
    m_vertices[9].normal = Vector3D(0.f, 0.f, -1.f);
    m_vertices[9].color = Vector4D(0.f, 1.f, 1.f, 1.f);
    m_vertices[9].texcoord = Vector2D(1.f, 0.f);
    m_vertices[10].position = Vector3D(+halfW, -halfH, -halfD);
    m_vertices[10].normal = Vector3D(0.f, 0.f, -1.f);
    m_vertices[10].color = Vector4D(1.f, 1.f, 0.f, 1.f);
    m_vertices[10].texcoord = Vector2D(1.f, 1.f);
    m_vertices[11].position = Vector3D(-halfW, -halfH, -halfD);
    m_vertices[11].normal = Vector3D(0.f, 0.f, -1.f);
    m_vertices[11].color = Vector4D(0.f, 0.f, 1.f, 1.f);
    m_vertices[11].texcoord = Vector2D(0.f, 1.f);
    //right
    m_vertices[12].position = Vector3D(halfW, +halfH, -halfD);
    m_vertices[12].normal = Vector3D(1.f, 0.f, 0.f);
    m_vertices[12].color = Vector4D(0.f, 1.f, 0.f, 1.f);
    m_vertices[12].texcoord = Vector2D(0.f, 0.f);
    m_vertices[13].position = Vector3D(halfW, +halfH, +halfD);
    m_vertices[13].normal = Vector3D(1.f, 0.f, 0.f);
    m_vertices[13].color = Vector4D(1.f, 0.f, 0.f, 1.f);
    m_vertices[13].texcoord = Vector2D(1.f, 0.f);
    m_vertices[14].position = Vector3D(halfW, -halfH, +halfD);
    m_vertices[14].normal = Vector3D(1.f, 0.f, 0.f);
    m_vertices[14].color = Vector4D(0.f, 1.f, 1.f, 1.f);
    m_vertices[14].texcoord = Vector2D(1.f, 1.f);
    m_vertices[15].position = Vector3D(halfW, -halfH, -halfD);
    m_vertices[15].normal = Vector3D(1.f, 0.f, 0.f);
    m_vertices[15].color = Vector4D(1.f, 0.f, 1.f, 1.f);
    m_vertices[15].texcoord = Vector2D(0.f, 1.f);
    //top
    m_vertices[16].position = Vector3D(+halfW, halfH, -halfD);
    m_vertices[16].normal = Vector3D(0.f, 1.f, 0.f);
    m_vertices[16].color = Vector4D(0.f, 0.f, 0.f, 1.f);
    m_vertices[16].texcoord = Vector2D(0.f, 0.f);
    m_vertices[17].position = Vector3D(-halfW, halfH, -halfD);
    m_vertices[17].normal = Vector3D(0.f, 1.f, 0.f);
    m_vertices[17].color = Vector4D(1.f, 1.f, 0.f, 1.f);
    m_vertices[17].texcoord = Vector2D(1.f, 0.f);
    m_vertices[18].position = Vector3D(-halfW, halfH, halfD);
    m_vertices[18].normal = Vector3D(0.f, 1.f, 0.f);
    m_vertices[18].color = Vector4D(0.f, 1.f, 1.f, 1.f);
    m_vertices[18].texcoord = Vector2D(1.f, 1.f);
    m_vertices[19].position = Vector3D(+halfW, halfH, halfD);
    m_vertices[19].normal = Vector3D(0.f, 1.f, 0.f);
    m_vertices[19].color = Vector4D(1.f, 0.f, 0.f, 1.f);
    m_vertices[19].texcoord = Vector2D(0.f, 1.f);
    //down
    m_vertices[20].position = Vector3D(+halfW, -halfH, -halfD);
    m_vertices[20].normal = Vector3D(0.f, -1.f, 0.f);
    m_vertices[20].color = Vector4D(0.f, 0.f, 1.f, 1.f);
    m_vertices[20].texcoord = Vector2D(0.f, 0.f);
    m_vertices[21].position = Vector3D(+halfW, -halfH, +halfD);
    m_vertices[21].normal = Vector3D(0.f, -1.f, 0.f);
    m_vertices[21].color = Vector4D(1.f, 1.f, 1.f, 1.f);
    m_vertices[21].texcoord = Vector2D(1.f, 0.f);
    m_vertices[22].position = Vector3D(-halfW, -halfH, +halfD);
    m_vertices[22].normal = Vector3D(0.f, -1.f, 0.f);
    m_vertices[22].color = Vector4D(0.f, 1.f, 0.f, 1.f);
    m_vertices[22].texcoord = Vector2D(1.f, 1.f);
    m_vertices[23].position = Vector3D(-halfW, -halfH, -halfD);
    m_vertices[23].normal = Vector3D(0.f, -1.f, 0.f);
    m_vertices[23].color = Vector4D(1.f, 0.f, 1.f, 1.f);
    m_vertices[23].texcoord = Vector2D(0.f, 1.f);

    //front
    m_indices[0] = 0;
    m_indices[1] = 1;
    m_indices[2] = 2;
    m_indices[3] = 0;
    m_indices[4] = 2;
    m_indices[5] = 3;
    //left
    m_indices[6] = 4;
    m_indices[7] = 5;
    m_indices[8] = 6;
    m_indices[9] = 4;
    m_indices[10] = 6;
    m_indices[11] = 7;
    //back
    m_indices[12] = 8;
    m_indices[13] = 9;
    m_indices[14] = 10;
    m_indices[15] = 8;
    m_indices[16] = 10;
    m_indices[17] = 11;
    //right
    m_indices[18] = 12;
    m_indices[19] = 13;
    m_indices[20] = 14;
    m_indices[21] = 12;
    m_indices[22] = 14;
    m_indices[23] = 15;
    //top
    m_indices[24] = 16;
    m_indices[25] = 17;
    m_indices[26] = 18;
    m_indices[27] = 16;
    m_indices[28] = 18;
    m_indices[29] = 19;
    //down
    m_indices[30] = 20;
    m_indices[31] = 21;
    m_indices[32] = 22;
    m_indices[33] = 20;
    m_indices[34] = 22;
    m_indices[35] = 23;
}

void Mesh::asFloor(double length, double height)
{
    m_vertices.resize(4);
    m_indices.resize(6);
    m_vertices[0].position = Vector3D(+length*0.5f, height, -length*0.5f);
    m_vertices[0].normal = Vector3D(0.f, 1.f, 0.f);
    m_vertices[0].color = Vector4D(1.f, 0.f, 0.f, 1.f);
    m_vertices[0].texcoord = Vector2D(0.f, 0.f);
    m_vertices[1].position = Vector3D(-length*0.5f, height, -length*0.5f);
    m_vertices[1].normal = Vector3D(0.f, 1.f, 0.f);
    m_vertices[1].color = Vector4D(0.f, 0.f, 0.f, 1.f);
    m_vertices[1].texcoord = Vector2D(1.f, 0.f);
    m_vertices[2].position = Vector3D(-length*0.5f, height, +length*0.5f);
    m_vertices[2].normal = Vector3D(0.f, 1.f, 0.f);
    m_vertices[2].color = Vector4D(0.f, 0.f, 1.f, 1.f);
    m_vertices[2].texcoord = Vector2D(1.f, 1.f);
    m_vertices[3].position = Vector3D(+length*0.5f, height, +length*0.5f);
    m_vertices[3].normal = Vector3D(0.f, 1.f, 0.f);
    m_vertices[3].color = Vector4D(0.f, 0.f, 0.f, 1.f);
    m_vertices[3].texcoord = Vector2D(0.f, 1.f);
    m_indices[0] = 0;
    m_indices[1] = 1;
    m_indices[2] = 2;
    m_indices[3] = 0;
    m_indices[4] = 2;
    m_indices[5] = 3;

}

void Mesh::asTriangle(Vector3D p1, Vector3D p2, Vector3D p3)
{
    m_vertices.resize(3);
    m_indices.resize(3);
    m_vertices[0].position = p1;
    m_vertices[0].normal = Vector3D(0.f, 0.f, 1.f);
    m_vertices[0].color = Vector4D(1.f, 0.f, 0.f, 1.f);
    m_vertices[0].texcoord = Vector2D(0.f, 0.f);
    m_vertices[1].position = p2;
    m_vertices[1].normal = Vector3D(0.f, 0.f, 1.f);
    m_vertices[1].color = Vector4D(0.f, 1.f, 0.f, 1.f);
    m_vertices[1].texcoord = Vector2D(1.f, 0.f);
    m_vertices[2].position = p3;
    m_vertices[2].normal = Vector3D(0.f, 0.f, 1.f);
    m_vertices[2].color = Vector4D(0.f, 0.f, 1.f, 1.f);
    m_vertices[2].texcoord = Vector2D(0.5f, 1.f);
    m_indices[0] = 0;
    m_indices[1] = 1;
    m_indices[2] = 2;
}

}
