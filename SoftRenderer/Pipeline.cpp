#include "Pipeline.h"
#include "SimpleShader.h"

#include <QDebug>
#include "Texture2D.h"

namespace SoftRenderer
{

Pipeline::Pipeline(int width, int height)
    :m_width(width),m_height(height)
    ,m_shader(nullptr),m_frontBuffer(nullptr)
    ,m_backBuffer(nullptr)
{
    m_eyePos = Vector3D(0.0f,0.0f,0.0f);
}

Pipeline::~Pipeline()
{
    if(m_shader)delete m_shader;
    if(m_frontBuffer)delete m_frontBuffer;
    if(m_backBuffer)delete m_backBuffer;
    m_shader = nullptr;
    m_frontBuffer = nullptr;
    m_backBuffer = nullptr;

    for(unsigned int x = 0;x < m_textureUnits.size();++x)
    {
        delete m_textureUnits[x];
        m_textureUnits[x] = nullptr;
    }
}

void Pipeline::initialize()
{
    if(m_frontBuffer)
        delete m_frontBuffer;
    if(m_backBuffer)
        delete m_backBuffer;
    if(m_shader)
        delete m_shader;
    viewPortMatrix.setViewPort(0,0,m_width,m_height);
    m_frontBuffer = new FrameBuffer(m_width, m_height);
    m_backBuffer = new FrameBuffer(m_width, m_height);
    m_shader = new SimpleShader();
}

void Pipeline::beginFrame()
{
    m_profile.setZero();
}

void Pipeline::endFrame()
{
}

unsigned int Pipeline::loadTexture(const std::string &path)
{
    Texture2D *tex = new Texture2D();
    if(!tex->loadImage(path))
        return 0;
    m_textureUnits.push_back(tex);
    return m_textureUnits.size() - 1;
}

bool Pipeline::bindTexture(const unsigned int &unit)
{
    if(unit >= m_textureUnits.size())
        return false;
    m_shader->bindShaderUnit(m_textureUnits[unit]);
    return true;
}

bool Pipeline::unBindTexture(const unsigned int &unit)
{
    if(unit >= m_textureUnits.size())
        return false;
    m_shader->bindShaderUnit(nullptr);
}

void Pipeline::setViewPort(int left, int top, int width, int height)
{
    viewPortMatrix.setViewPort(left, top, width, height);
}

void Pipeline::setModelMatrix(Matrix4x4 modelMatrix)
{
    m_shader->setModelMatrix(modelMatrix);
}

void Pipeline::setViewMatrix(Vector3D eye, Vector3D target, Vector3D up)
{
    this->m_eyePos = eye;
    Matrix4x4 viewMatrix;
    viewMatrix.setLookAt(eye, target, up);
    m_shader->setViewMatrix(viewMatrix);
}

void Pipeline::setProjectMatrix(float fovy, float aspect, float near, float far)
{
    Matrix4x4 projectMatrix;
    projectMatrix.setPerspective(fovy, aspect, near, far);
    m_shader->setProjectMatrix(projectMatrix);
}

void Pipeline::drawIndex(RenderMode mode)
{
    // renderer pipeline.
    bool line1 = false, line2 = false, line3 = false;
    m_mode = mode;
    if(m_indices->empty())return;

    for(unsigned int i = 0;i < m_indices->size();i += 3)
    {
        //! assembly to triangle primitive.
        Vertex p1,p2,p3;
        {

            p1 = (*m_vertices)[(*m_indices)[i+0]];
            p2 = (*m_vertices)[(*m_indices)[i+1]];
            p3 = (*m_vertices)[(*m_indices)[i+2]];
        }

        //! vertex shader stage.
        VertexOut v1,v2,v3;
        {
            v1 = m_shader->vertexShader(p1);
            v2 = m_shader->vertexShader(p2);
            v3 = m_shader->vertexShader(p3);
        }

        //! back face culling.
        {
            if(!backFaceCulling(v1.posTrans, v2.posTrans, v3.posTrans))
                continue;
        }

        //! geometry cliping.
        {
            if(m_mode == RenderMode::wire)
            {
                line1 = lineCliping(v1,v2);
                line2 = lineCliping(v2,v3);
                line3 = lineCliping(v3,v1);
            }
            else if(m_mode == RenderMode::fill && !triangleCliping(v1,v2,v3))
                continue;
        }

        //! perspective division.
        {
            perspectiveDivision(v1);
            perspectiveDivision(v2);
            perspectiveDivision(v3);
        }

        //! view port transformation.
        {
            v1.posH = viewPortMatrix * v1.posH;
            v2.posH = viewPortMatrix * v2.posH;
            v3.posH = viewPortMatrix * v3.posH;
        }


        //! rasterization and fragment shader stage.
        {
            if(mode == RenderMode::wire)
            {
                if(!line1)
                    bresenhamLineRasterization(v1,v2);
                if(!line2)
                    bresenhamLineRasterization(v2,v3);
                if(!line3)
                    bresenhamLineRasterization(v3,v1);
            }
            else if(mode == RenderMode::fill)
            {
                edgeWalkingFillRasterization(v1,v2,v3);
            }
        }
        if(!line1 && !line2 && !line3)
            ++ m_profile.num_triangles;
        if(!line1)
            ++ m_profile.num_vertices;
        if(!line2)
            ++ m_profile.num_vertices;
        if(!line3)
            ++ m_profile.num_vertices;
    }
}

void Pipeline::clearBuffer(const Vector4D &color)
{
    m_backBuffer->clearColorAndDepthBuffer(color);
}

void Pipeline::setShaderMode(ShadingMode mode)
{
    if(m_shader)delete m_shader;
    if(mode == ShadingMode::simple)
        m_shader = new SimpleShader();
    else if(mode == ShadingMode::phong)
        ;
}

void Pipeline::swapBuffer()
{
    FrameBuffer *tmp = m_frontBuffer;
    m_frontBuffer = m_backBuffer;
    m_backBuffer = tmp;
}

void Pipeline::perspectiveDivision(VertexOut &target)
{
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
    result.posTrans = n1.posTrans.lerp(n2.posTrans, weight);
    result.posH = n1.posH.lerp(n2.posH, weight);
    result.color = n1.color.lerp(n2.color, weight);
    result.normal = n1.normal.lerp(n2.normal, weight);
    result.texcoord = n1.texcoord.lerp(n2.texcoord, weight);
    result.oneDivZ = (1.0 - weight) * n1.oneDivZ + weight * n2.oneDivZ;
    return result;
}

bool Pipeline::lineCliping(const VertexOut &from, const VertexOut &to)
{
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
    // back face culling.
    if(m_mode == RenderMode::wire)
        return true;
    Vector4D tmp1 = v2 - v1;
    Vector4D tmp2 = v3 - v1;
    Vector3D edge1(tmp1.x, tmp1.y, tmp1.z);
    Vector3D edge2(tmp2.x, tmp2.y, tmp2.z);
    Vector3D viewRay(m_eyePos.x - v1.x,
                     m_eyePos.y - v1.y,
                     m_eyePos.z - v1.z);
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
            double depth = m_backBuffer->getDepth(sx, sy);
            if(tmp.posH.z > depth)
                continue;// fail to pass the depth testing.
            m_backBuffer->drawDepth(sx,sy,tmp.posH.z);

            // fragment shader
            m_backBuffer->drawPixel(sx,sy,m_shader->fragmentShader(tmp));
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
            m_backBuffer->drawPixel(sx,sy,m_shader->fragmentShader(tmp));
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
        double depth = m_backBuffer->getDepth(current.posH.x, current.posH.y);
        if(current.posH.z > depth)
            continue;// fail to pass the depth testing.
        m_backBuffer->drawDepth(current.posH.x,current.posH.y,current.posH.z);

        double w = 1.0/current.oneDivZ;
        current.posTrans *= w;
        current.color *= w;
        current.texcoord *= w;
        // fragment shader
        m_backBuffer->drawPixel(current.posH.x, current.posH.y,
                                m_shader->fragmentShader(current));
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

void Pipeline::edgeWalkingFillRasterization(const VertexOut &v1, const VertexOut &v2, const VertexOut &v3)
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
