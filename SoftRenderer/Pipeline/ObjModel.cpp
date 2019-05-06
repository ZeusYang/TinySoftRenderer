#include "ObjModel.h"

#include <fstream>
#include <sstream>

using namespace std;

namespace SoftRenderer
{

ObjModel::ObjModel(const std::string &path)
{
    loadObjFile(path);
}

ObjModel::~ObjModel()
{
}

Vector3D ObjModel::setSizeToVector(float sx, float sy, float sz) const
{
    float length = fabs(maxPoint.x - minPoint.x);
    float scaleFactor = 1.0f/length;
    return Vector3D(scaleFactor*sx, scaleFactor*sy, scaleFactor*sz);
}

Matrix4x4 ObjModel::setSizeToMatrix(float sx, float sy, float sz) const
{
    float length = fabs(maxPoint.x - minPoint.x);
    float scaleFactor = 1.0f/length;
    Matrix4x4 result;
    result.setScale(Vector3D(scaleFactor*sx, scaleFactor*sy, scaleFactor*sz));
    return result;
}

void ObjModel::loadObjFile(const std::string &path)
{
    // obj loader.
    ifstream in;
    in.open(path, ifstream::in);
    if(in.fail())
    {
        std::cout << "Fail to load obj->" << path << endl;
    }
    string line;
    minPoint = Vector3D(+10000000000,+10000000000,+10000000000);
    maxPoint = Vector3D(-10000000000,-10000000000,-10000000000);
    vector<Vector3D> vertices;
    vector<Vector3D> normals;
    vector<Vector2D> texcoords;
    while(!in.eof())
    {
        getline(in, line);
        istringstream iss(line.c_str());
        char trash;
        //vertex
        if(!line.compare(0, 2, "v "))
        {
            iss >> trash;
            Vector3D vertex;
            iss >> vertex.x;
            iss >> vertex.y;
            iss >> vertex.z;
            vertices.push_back(vertex);
            if(minPoint.x > vertex.x)minPoint.x = vertex.x;
            if(minPoint.y > vertex.y)minPoint.y = vertex.y;
            if(minPoint.z > vertex.z)minPoint.z = vertex.z;
            if(maxPoint.x < vertex.x)maxPoint.x = vertex.x;
            if(maxPoint.y < vertex.y)maxPoint.y = vertex.y;
            if(maxPoint.z < vertex.z)maxPoint.z = vertex.z;
        }
        // normal
        else if(!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            Vector3D normal;
            iss >> normal.x;
            iss >> normal.y;
            iss >> normal.z;
            normal.normalize();
            normals.push_back(normal);
        }
        // texcoord
        else if(!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            Vector2D texcoord;
            iss >> texcoord.x;
            iss >> texcoord.y;
            texcoords.push_back(texcoord);
        }
        // face
        else if(!line.compare(0, 2, "f "))
        {
            iss >> trash;
            int index[3];
            while(iss >> index[0] >> trash >> index[1] >> trash >> index[2])
            {
                Vertex data;
                data.position = vertices[index[0] - 1];
                data.texcoord = texcoords[index[1] - 1];
                data.normal = normals[index[2] - 1];
                data.color = Vector4D(1.0,1.0,1.0,1.0);
                m_indices.push_back(m_vertices.size());
                m_vertices.push_back(data);
            }
        }
    }
    in.close();
}



}
