#include "Pipeline.h"

#include "Light.h"
#include "Texture2D.h"
#include "../Shader/SimpleShader.h"
#include "../Shader/GouraudShader.h"
#include "../Shader/PhongShader.h"

namespace SoftRenderer
{

Pipeline::Pipeline(int width, int height)
{
    m_config.m_width = width;
    m_config.m_height = height;
    m_config.m_light = nullptr;
    m_config.m_shader = nullptr;
    m_config.m_frontBuffer = nullptr;
    m_config.m_backBuffer = nullptr;
    m_config.m_eyePos = Vector3D(0.0f,0.0f,0.0f);
}

Pipeline::~Pipeline()
{
    if(m_config.m_light)delete m_config.m_light;
    if(m_config.m_shader)delete m_config.m_shader;
    if(m_config.m_backBuffer)delete m_config.m_backBuffer;
    if(m_config.m_frontBuffer)delete m_config.m_frontBuffer;
    for(unsigned int x = 0;x < m_config.m_textureUnits.size();++x)
    {
        delete m_config.m_textureUnits[x];
        m_config.m_textureUnits[x] = nullptr;
    }
    m_config.m_light = nullptr;
    m_config.m_shader = nullptr;
    m_config.m_frontBuffer = nullptr;
    m_config.m_backBuffer = nullptr;
}

void Pipeline::initialize()
{
    if(m_config.m_shader)delete m_config.m_shader;
    if(m_config.m_backBuffer)delete m_config.m_backBuffer;
    if(m_config.m_frontBuffer)delete m_config.m_frontBuffer;
    m_config.m_shader = new SimpleShader();
    m_config.m_backBuffer = new FrameBuffer(m_config.m_width, m_config.m_height);
    m_config.m_frontBuffer = new FrameBuffer(m_config.m_width, m_config.m_height);
    m_config.m_viewPortMatrix.setViewPort(0,0,m_config.m_width,m_config.m_height);
    setDefaultConfig();
}

void Pipeline::beginFrame()
{
    m_config.m_profile.setZero();
}

void Pipeline::endFrame() {}

unsigned int Pipeline::loadTexture(const std::string &path)
{
    Texture2D *tex = new Texture2D();
    if(!tex->loadImage(path))
        return 0;
    m_config.m_textureUnits.push_back(tex);
    return static_cast<unsigned int>(m_config.m_textureUnits.size() - 1);
}

bool Pipeline::bindTexture(const unsigned int &unit)
{
    if(unit >= m_config.m_textureUnits.size())
        return false;
    m_config.m_shader->bindShaderUnit(m_config.m_textureUnits[unit]);
    return true;
}

bool Pipeline::unBindTexture(const unsigned int &unit)
{
    if(unit >= m_config.m_textureUnits.size())
        return false;
    m_config.m_shader->bindShaderUnit(nullptr);
    return true;
}

void Pipeline::setMaterial(const Material *material)
{
    m_config.m_material = material;
    m_config.m_shader->setMaterial(m_config.m_material);
}

void Pipeline::setModelMatrix(Matrix4x4 modelMatrix)
{
    m_config.m_shader->setModelMatrix(modelMatrix);
}

void Pipeline::setViewMatrix(Vector3D eye, const Matrix4x4 &viewMatrix)
{
    m_config.m_eyePos = eye;
    m_config.m_shader->setEyePos(eye);
    m_config.m_shader->setViewMatrix(viewMatrix);
}

void Pipeline::setViewMatrix(Vector3D eye, Vector3D target, Vector3D up)
{
    m_config.m_eyePos = eye;
    Matrix4x4 viewMatrix;
    viewMatrix.setLookAt(eye, target, up);
    m_config.m_shader->setEyePos(eye);
    m_config.m_shader->setViewMatrix(viewMatrix);
}

void Pipeline::setProjectMatrix(float fovy, float aspect, float near, float far)
{
    Matrix4x4 projectMatrix;
    projectMatrix.setPerspective(fovy, aspect, near, far);
    m_config.m_shader->setProjectMatrix(projectMatrix);
}

void Pipeline::setDirectionLight(Vector3D amb, Vector3D diff, Vector3D spec, Vector3D dir)
{
    if(m_config.m_light)delete m_config.m_light;
    DirectionalLight *tmp = new DirectionalLight();
    tmp->setDirectionalLight(amb, diff, spec, dir);
    m_config.m_light = reinterpret_cast<Light*>(tmp);
    m_config.m_shader->setLight(m_config.m_light);
}

void Pipeline::setPointLight(Vector3D _amb, Vector3D _diff, Vector3D _spec, Vector3D _pos, Vector3D _atte)
{
    if(m_config.m_light)delete m_config.m_light;
    PointLight *tmp = new PointLight();
    tmp->setPointLight(_amb, _diff, _spec, _pos, _atte);
    m_config.m_light = reinterpret_cast<Light*>(tmp);
    m_config.m_shader->setLight(m_config.m_light);
}

void Pipeline::setSpotLight(Vector3D _amb, Vector3D _diff, Vector3D _spec, double _cutoff,
                            Vector3D _pos, Vector3D _dir, Vector3D _atte)
{
    if(m_config.m_light)delete m_config.m_light;
    SpotLight *tmp = new SpotLight();
    tmp->setSpotLight(_amb, _diff, _spec, _cutoff, _pos, _dir, _atte);
    m_config.m_light = reinterpret_cast<Light*>(tmp);
    m_config.m_shader->setLight(m_config.m_light);
}

// rendering pipeline.
bool Pipeline::drawObjectMesh()
{
    if(m_config.m_indices->empty())
        return false;

    // For line cliping.
    bool line1 = false, line2 = false, line3 = false;
    for(unsigned int i = 0;i < m_config.m_indices->size();i += 3)
    {
        //! assemble to a triangle primitive.
        Vertex p1,p2,p3;
        {
            p1 = (*m_config.m_vertices)[(*m_config.m_indices)[i+0]];
            p2 = (*m_config.m_vertices)[(*m_config.m_indices)[i+1]];
            p3 = (*m_config.m_vertices)[(*m_config.m_indices)[i+2]];
        }

        //! vertex shader stage.
        VertexOut v1,v2,v3;
        {
            v1 = m_config.m_shader->vertexShader(p1);
            v2 = m_config.m_shader->vertexShader(p2);
            v3 = m_config.m_shader->vertexShader(p3);
        }

        //! back face culling.
        {
            if(m_config.m_backFaceCulling
                    && !backFaceCulling(v1.posTrans, v2.posTrans, v3.posTrans))
                continue;
        }

        //! geometry cliping.
        {
            if(m_config.m_geometryCliping)
            {
                if(m_config.m_polygonMode == PolygonMode::Wire)
                {
                    line1 = lineCliping(v1,v2);
                    line2 = lineCliping(v2,v3);
                    line3 = lineCliping(v3,v1);
                }
                if(m_config.m_polygonMode == PolygonMode::Fill && !triangleCliping(v1,v2,v3))
                    continue;
            }
        }

        //! perspective division.
        {
            perspectiveDivision(v1);
            perspectiveDivision(v2);
            perspectiveDivision(v3);
        }

        //! view port transformation.
        {
            v1.posH = m_config.m_viewPortMatrix * v1.posH;
            v2.posH = m_config.m_viewPortMatrix * v2.posH;
            v3.posH = m_config.m_viewPortMatrix * v3.posH;
        }

        //! rasterization and fragment shader stage.
        {
            if(m_config.m_polygonMode == PolygonMode::Wire)
            {
                if(!line1)
                    bresenhamLineRasterization(v1,v2);
                if(!line2)
                    bresenhamLineRasterization(v2,v3);
                if(!line3)
                    bresenhamLineRasterization(v3,v1);
            }
            else if(m_config.m_polygonMode == PolygonMode::Fill)
            {
                edgeWalkingFillRasterization(v1,v2,v3);
            }
        }
        if(!line1 && !line2 && !line3)
            ++ m_config.m_profile.num_triangles;
        if(!line1)
            ++ m_config.m_profile.num_vertices;
        if(!line2)
            ++ m_config.m_profile.num_vertices;
        if(!line3)
            ++ m_config.m_profile.num_vertices;
    }
    return true;
}

void Pipeline::setDefaultConfig()
{
    // default configuration.
    setDepthTesting(true);
    setBackFaceCulling(true);
    setGeometryCliping(true);
    setPolygonMode(PolygonMode::Fill);
    setShadingMode(ShadingMode::Simple);
}

void Pipeline::setShadingMode(ShadingMode mode)
{
    if(m_config.m_shader)delete m_config.m_shader;
    m_config.m_shader = nullptr;
    switch(mode)
    {
    case ShadingMode::Simple:
        m_config.m_shader = new SimpleShader();
        break;
    case ShadingMode::Gouraud:
        m_config.m_shader = new GouraudShader();
        break;
    case ShadingMode::Phong:
        m_config.m_shader = new PhongShader();
        break;
    default:
        m_config.m_shader = new SimpleShader();
        break;
    }
}

void Pipeline::swapFrameBuffer()
{
    // swap frame buffers to avoid flashing.
    FrameBuffer *tmp = m_config.m_frontBuffer;
    m_config.m_frontBuffer = m_config.m_backBuffer;
    m_config.m_backBuffer = tmp;
}

void Pipeline::perspectiveDivision(VertexOut &target)
{
    // perspective divison.
    target.posH.x /= target.posH.w;
    target.posH.y /= target.posH.w;
    target.posH.z /= target.posH.w;
    target.posH.w = 1.0f;
    // map from [-1,1] to [0,1]
    target.posH.z = (target.posH.z+1.0f) * 0.5f;
}

VertexOut Pipeline::lerp(const VertexOut &n1, const VertexOut &n2, double weight)
{
    // linear interpolation.
    VertexOut result;
    result.posH = n1.posH.lerp(n2.posH, weight);
    result.posTrans = n1.posTrans.lerp(n2.posTrans, weight);
    result.color = n1.color.lerp(n2.color, weight);
    result.normal = n1.normal.lerp(n2.normal, weight);
    result.texcoord = n1.texcoord.lerp(n2.texcoord, weight);
    result.oneDivZ = (1.0 - weight) * n1.oneDivZ + weight * n2.oneDivZ;
    return result;
}

bool Pipeline::lineCliping(const VertexOut &from, const VertexOut &to)
{
    // Cohen-Sutherland algorithm.
    // return whether the line is totally outside or not.
    float vMin = -from.posH.w, vMax = from.posH.w;
    float x1 = from.posH.x, y1 = from.posH.y;
    float x2 = to.posH.x, y2 = to.posH.y;

    int tmp = 0;
    int outcode1 = 0, outcode2 = 0;

    // outcode1 calculation.
    tmp = (y1>vMax)?1:0;
    tmp <<= 3;
    outcode1 |= tmp;
    tmp = (y1<vMin)?1:0;
    tmp <<= 2;
    outcode1 |= tmp;
    tmp = (x1>vMax)?1:0;
    tmp <<= 1;
    outcode1 |= tmp;
    tmp = (x1<vMin)?1:0;
    outcode1 |= tmp;

    // outcode2 calculation.
    tmp = (y2>vMax)?1:0;
    tmp <<= 3;
    outcode2 |= tmp;
    tmp = (y2<vMin)?1:0;
    tmp <<= 2;
    outcode2 |= tmp;
    tmp = (x2>vMax)?1:0;
    tmp <<= 1;
    outcode2 |= tmp;
    tmp = (x2<vMin)?1:0;
    outcode2 |= tmp;

    if((outcode1 & outcode2) != 0)
        return true;

    // bounding box judge.
    Vector2D minPoint,maxPoint;
    minPoint.x = min(from.posH.x, to.posH.x);
    minPoint.y = min(from.posH.y, to.posH.y);
    maxPoint.x = max(from.posH.x, to.posH.x);
    maxPoint.y = max(from.posH.y, to.posH.y);
    if(minPoint.x > vMax || maxPoint.x < vMin || minPoint.y > vMax || maxPoint.y < vMin)
        return true;

    return false;
}

bool Pipeline::triangleCliping(const VertexOut &v1, const VertexOut &v2, const VertexOut &v3)
{
    // true:not clip;
    // false: clip.
    float vMin = -v1.posH.w;
    float vMax = +v1.posH.w;

    // if the triangle is too far to see it, just return false.
    if(v1.posH.z > vMax && v2.posH.z > vMax && v3.posH.z > vMax)
        return false;

    // if the triangle is behind the camera, just return false.
    if(v1.posH.z < vMin && v2.posH.z < vMin && v3.posH.z < vMin)
        return false;

    // calculate the bounding box and check if clip or not.
    Vector2D minPoint,maxPoint;
    minPoint.x = min(v1.posH.x, min(v2.posH.x, v3.posH.x));
    minPoint.y = min(v1.posH.y, min(v2.posH.y, v3.posH.y));
    maxPoint.x = max(v1.posH.x, max(v2.posH.x, v3.posH.x));
    maxPoint.y = max(v1.posH.y, max(v2.posH.y, v3.posH.y));
    if(minPoint.x > vMax || maxPoint.x < vMin || minPoint.y > vMax || maxPoint.y < vMin)
        return false;

    return true;
}

bool Pipeline::backFaceCulling(const Vector4D &v1, const Vector4D &v2, const Vector4D &v3)
{
    // back faces culling.
    if(m_config.m_polygonMode == PolygonMode::Wire)
        return true;
    Vector4D tmp1 = v2 - v1;
    Vector4D tmp2 = v3 - v1;
    Vector3D edge1(tmp1.x, tmp1.y, tmp1.z);
    Vector3D edge2(tmp2.x, tmp2.y, tmp2.z);
    Vector3D viewRay(m_config.m_eyePos.x - v1.x,
                     m_config.m_eyePos.y - v1.y,
                     m_config.m_eyePos.z - v1.z);
    Vector3D normal = edge1.crossProduct(edge2);
    return normal.dotProduct(viewRay) > 0;
}

void Pipeline::bresenhamLineRasterization(const VertexOut &from, const VertexOut &to)
{
    int dx = to.posH.x - from.posH.x;
    int dy = to.posH.y - from.posH.y;
    int stepX = 1, stepY = 1;

    // judge the sign
    if(dx < 0)
    {
        stepX = -1;
        dx = -dx;
    }
    if(dy < 0)
    {
        stepY = -1;
        dy = -dy;
    }

    int d2x = 2*dx, d2y = 2*dy;
    int d2y_minus_d2x = d2y - d2x;
    int sx = from.posH.x;
    int sy = from.posH.y;

    VertexOut tmp;
    // slope < 1.
    if(dy <= dx)
    {
        int flag = d2y - dx;
        for(int i = 0;i <= dx;++ i)
        {
            // linear interpolation
            tmp = lerp(from, to, static_cast<double>(i)/dx);

            // depth testing.
            double depth = m_config.m_backBuffer->getDepth(sx, sy);
            if(tmp.posH.z > depth)
                continue;// fail to pass the depth testing.
            m_config.m_backBuffer->drawDepth(sx,sy,tmp.posH.z);

            // fragment shader
            m_config.m_backBuffer->drawPixel(sx,sy,m_config.m_shader->fragmentShader(tmp));
            sx += stepX;
            if(flag <= 0)
                flag += d2y;
            else
            {
                sy += stepY;
                flag += d2y_minus_d2x;
            }
        }
    }
    // slope > 1.
    else
    {
        int flag = d2x - dy;
        for(int i = 0;i <= dy;++ i)
        {
            // linear interpolation
            tmp = lerp(from, to, static_cast<double>(i)/dy);

            // fragment shader
            m_config.m_backBuffer->drawPixel(sx,sy,m_config.m_shader->fragmentShader(tmp));
            sy += stepY;

            if(flag <= 0)
                flag += d2x;
            else
            {
                sx += stepX;
                flag -= d2y_minus_d2x;
            }
        }
    }
}

void Pipeline::scanLinePerRow(const VertexOut &left, const VertexOut &right)
{
    // scan the line from left to right.
    VertexOut current;
    int length = right.posH.x - left.posH.x + 1;
    for(int i = 0;i <= length;++i)
    {
        // linear interpolation
        double weight = static_cast<double>(i)/length;
        current = lerp(left, right, weight);
        current.posH.x = left.posH.x + i;
        current.posH.y = left.posH.y;

        // depth testing.
        if(m_config.m_depthTesting)
        {
            double depth = m_config.m_backBuffer->getDepth(current.posH.x, current.posH.y);
            if(current.posH.z > depth)
                continue;// fail to pass the depth testing.
            m_config.m_backBuffer->drawDepth(current.posH.x,current.posH.y,current.posH.z);
        }

        //
        if(current.posH.x < 0 || current.posH.y < 0)
            continue;
        if(current.posH.x >= m_config.m_width || current.posH.y >= m_config.m_height)
            break;

        double w = 1.0/current.oneDivZ;
        current.posTrans *= w;
        current.color *= w;
        current.texcoord *= w;
        // fragment shader
        m_config.m_backBuffer->drawPixel(current.posH.x, current.posH.y,
                                         m_config.m_shader->fragmentShader(current));
    }
}

void Pipeline::rasterTopTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3)
{
    VertexOut left = v2;
    VertexOut right = v3;
    VertexOut dest = v1;
    VertexOut tmp, newleft, newright;
    if(left.posH.x > right.posH.x)
    {
        tmp = left;
        left = right;
        right = tmp;
    }
    int dy = left.posH.y - dest.posH.y + 1;

    for(int i = 0;i < dy;++i)
    {
        double weight = 0;
        if(dy != 0)
            weight = static_cast<double>(i)/dy;
        newleft = lerp(left, dest, weight);
        newright = lerp(right, dest, weight);
        newleft.posH.y = newright.posH.y = left.posH.y - i;
        //newleft.oneDivZ = (1.0 - weight) * left.oneDivZ + weight * dest.oneDivZ;
        //newright.oneDivZ = (1.0 - weight) * right.oneDivZ + weight * dest.oneDivZ;
        scanLinePerRow(newleft, newright);
    }
}

void Pipeline::rasterBottomTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3)
{
    VertexOut left = v1;
    VertexOut right = v2;
    VertexOut dest = v3;
    VertexOut tmp, newleft, newright;
    if(left.posH.x > right.posH.x)
    {
        tmp = left;
        left = right;
        right = tmp;
    }
    int dy = dest.posH.y - left.posH.y + 1;


    for(int i = 0;i < dy;++i)
    {
        double weight = 0;
        if(dy != 0)
            weight = static_cast<double>(i)/dy;
        newleft = lerp(left, dest, weight);
        newright = lerp(right, dest, weight);
        newleft.posH.y = newright.posH.y = left.posH.y + i;
        scanLinePerRow(newleft, newright);
    }
}

void Pipeline::edgeWalkingFillRasterization(const VertexOut &v1, const VertexOut &v2,
                                            const VertexOut &v3)
{
    // split the triangle into two part
    VertexOut tmp;
    VertexOut target[3] = {v1, v2,v3};
    if(target[0].posH.y > target[1].posH.y)
    {
        tmp = target[0];
        target[0] = target[1];
        target[1] = tmp;
    }
    if(target[0].posH.y > target[2].posH.y)
    {
        tmp = target[0];
        target[0] = target[2];
        target[2] = tmp;
    }
    if(target[1].posH.y > target[2].posH.y)
    {
        tmp = target[1];
        target[1] = target[2];
        target[2] = tmp;
    }

    // bottom triangle
    if(equal(target[0].posH.y,target[1].posH.y))
    {
        rasterBottomTriangle(target[0],target[1],target[2]);
    }
    // top triangle
    else if(equal(target[1].posH.y,target[2].posH.y))
    {
        rasterTopTriangle(target[0], target[1], target[2]);
    }
    // split it.
    else
    {
        double weight = static_cast<double>(target[1].posH.y-target[0].posH.y)
                /(target[2].posH.y-target[0].posH.y);
        VertexOut newPoint = lerp(target[0],target[2],weight);
        newPoint.posH.y = target[1].posH.y;
        rasterTopTriangle(target[0], newPoint, target[1]);
        rasterBottomTriangle(newPoint,target[1],target[2]);
    }
}

}
