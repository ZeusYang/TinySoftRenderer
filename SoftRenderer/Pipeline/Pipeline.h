#ifndef PIPELINE_H
#define PIPELINE_H

#include "FrameBuffer.h"
#include "../Shader/BaseShader.h"

/**
 * @projectName   SoftRenderer
 * @brief         Render pipeline class.
 * @author        YangWC
 * @date          2019-04-30
 */
namespace SoftRenderer
{

class Material;
class Texture2D;
struct Profile
{
public:
    unsigned int num_triangles;
    unsigned int num_vertices;
    Profile():num_triangles(0),num_vertices(0) {}
    void setZero(){num_triangles = num_vertices = 0;}
};

// illumination mode and polygon mode.
enum ShadingMode{ Simple, Gouraud, Phong};
enum PolygonMode{ Wire, Fill};

class Pipeline
{
private:
    class Setting
    {
    public:
        // State setting.
        bool m_depthTesting;                            // Depth testing switch.
        bool m_backFaceCulling;                         // Back faces culling switch.
        bool m_geometryCliping;                         // Geometry cliping switch.

        // Created by pipeline.
        Light *m_light;                                 // light.(Just only one)
        Profile m_profile;                              // Record.
        BaseShader *m_shader;                           // Shader.
        int m_width, m_height;                          // Width and height of viewport.
        ShadingMode m_shading;                          // Shading mode.
        PolygonMode m_polygonMode;                       // Polygon mode.
        FrameBuffer *m_backBuffer;                      // The frame buffer that's goint to be written.
        FrameBuffer *m_frontBuffer;                     // The frame buffer that's goint to be display.
        Matrix4x4 m_viewPortMatrix;                     // Viewport transformation matrix.
        std::vector<Texture2D*> m_textureUnits;         // Texture units.

        // Created by user.(would not deleted by pipeline)
        Vector3D m_eyePos;                              // Observer's position.
        const Material *m_material;                     // Object's material.
        const std::vector<Vertex> *m_vertices;          // Vertex buffer.
        const std::vector<unsigned int> *m_indices;     // Index buffer.
    };

    // Pipeline configuration.
    // (state machine like OpenGL)
    Setting m_config;

public:
    // ctor/dtor.
    Pipeline(int width, int height);
    ~Pipeline();

    // Preparation.
    void initialize();
    void beginFrame();
    void endFrame();

    // State setting.
    void setDepthTesting(bool open){m_config.m_depthTesting = open;}
    void setBackFaceCulling(bool open){m_config.m_backFaceCulling = open;}
    void setGeometryCliping(bool open){m_config.m_geometryCliping = open;}

    // Texture setting.
    bool bindTexture(const unsigned int &unit);
    bool unBindTexture(const unsigned int &unit);
    unsigned int loadTexture(const std::string &path);

    // Buffer setting.
    void swapFrameBuffer();
    void clearFrameBuffer(const Vector4D &color)
    {m_config.m_backBuffer->clearColorAndDepthBuffer(color);}
    void setVertexBuffer(const std::vector<Vertex> *vertices){m_config.m_vertices = vertices;}
    void setIndexBuffer(const std::vector<unsigned int> *indices){m_config.m_indices = indices;}

    // Matrix setting.
    void setModelMatrix(Matrix4x4 modelMatrix);
    void setViewMatrix(Vector3D eye, const Matrix4x4 &viewMatrix);
    void setViewMatrix(Vector3D eye, Vector3D target, Vector3D up);
    void setProjectMatrix(float fovy, float aspect, float near, float far);
    void setViewPort(int left, int top, int width, int height)
    {m_config.m_viewPortMatrix.setViewPort(left, top, width, height);}

    // Illumination setting.
    void setMaterial(const Material *material);
    void setShadingMode(ShadingMode shadingMode);
    void setPolygonMode(PolygonMode polygonMode){m_config.m_polygonMode = polygonMode;}
    void setSpotLight(Vector3D _amb, Vector3D _diff, Vector3D _spec, double _cutoff,
                      Vector3D _pos, Vector3D _dir, Vector3D _atte);
    void setDirectionLight(Vector3D _amb, Vector3D _diff, Vector3D _spec, Vector3D _dir);
    void setPointLight(Vector3D _amb, Vector3D _diff, Vector3D _spec, Vector3D _pos, Vector3D _atte);

    // Start the rendering pipeline.
    bool drawObjectMesh();

    // Getter.
    Profile getProfile(){return m_config.m_profile;}
    Matrix4x4 getViewPortMatrix(){return m_config.m_viewPortMatrix;}
    unsigned char *getFrameResult(){return m_config.m_frontBuffer->getColorBuffer();}

private:
    // Default configuration.
    void setDefaultConfig();

    // Perspective division.
    void perspectiveDivision(VertexOut &target);

    // Linear interpolation.
    VertexOut lerp(const VertexOut &n1, const VertexOut &n2, double weight);

    // Back faces culling.
    bool backFaceCulling(const Vector4D &v1, const Vector4D &v2, const Vector4D &v3);

    // Geometry cliping.
    bool lineCliping(const VertexOut &from, const VertexOut &to);
    bool triangleCliping(const VertexOut &v1, const VertexOut &v2, const VertexOut &v3);

    // Rasterization.
    void scanLinePerRow(const VertexOut &left, const VertexOut &right);
    void rasterTopTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3);
    void rasterBottomTriangle(VertexOut &v1, VertexOut &v2, VertexOut &v3);
    void bresenhamLineRasterization(const VertexOut &from, const VertexOut &to);
    void edgeWalkingFillRasterization(const VertexOut &v1, const VertexOut &v2, const VertexOut &v3);

};

}

#endif // PIPELINE_H
