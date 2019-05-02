#ifndef OBJMODEL_H
#define OBJMODEL_H

#include "Mesh.h"
#include "Math/Matrix4x4.h"

/**
 * @projectName   SoftRenderer
 * @brief         Obj file loader.
 * @author        YangWC
 * @date          2019-05-02
 */
namespace SoftRenderer
{

class ObjModel : public Mesh
{
private:
    Vector3D minPoint, maxPoint;
public:
    ObjModel(const std::string &path);
    virtual ~ObjModel();

    Matrix4x4 setSize(float sx, float sy, float sz) const;

private:
    void loadObjFile(const std::string &path);
};

}
#endif // OBJMODEL_H
