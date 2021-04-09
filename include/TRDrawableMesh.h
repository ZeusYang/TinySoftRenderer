#ifndef TRDRAWABLEMESH_H
#define TRDRAWABLEMESH_H

#include <vector>
#include <memory>
#include <string>

#include "glm/glm.hpp"

#include "TRShadingState.h"

namespace TinyRenderer
{
	class TRVertex final
	{
	public:

		glm::vec4 vpositions = glm::vec4(0, 0, 0, 1);
		glm::vec4 vcolors    = glm::vec4(1, 1, 1, 1);
		glm::vec2 vtexcoords = glm::vec2(0, 0);
		glm::vec3 vnormals   = glm::vec3(0, 1, 0);

		// tangent
		glm::vec3 vtangent;
		// bitangent
		glm::vec3 vbitangent;
	};

	using TRVertexBuffer = std::vector<TRVertex>;
	using TRIndexBuffer  = std::vector<unsigned int>;

	class TRDrawableSubMesh
	{
	public:
		typedef std::shared_ptr<TRDrawableSubMesh> ptr;

		TRDrawableSubMesh() = default;
		~TRDrawableSubMesh() = default;
		
		TRDrawableSubMesh(const TRDrawableSubMesh& mesh);
		TRDrawableSubMesh& operator=(const TRDrawableSubMesh& mesh);

		void setVertices(const std::vector<TRVertex> &vertices) { m_vertices = vertices; }
		void setIndices(const std::vector<unsigned int> &indices) { m_indices = indices; }

		void setDiffuseMapTexId(const int &id) { m_drawing_material.diffuseMapTexId = id; }
		void setSpecularMapTexId(const int &id) { m_drawing_material.specularMapTexId = id; }
		void setNormalMapTexId(const int &id) { m_drawing_material.normalMapTexId = id; }
		void setGlowMapTexId(const int &id) { m_drawing_material.glowMapTexId = id; }

		const int& getDiffuseMapTexId() const { return m_drawing_material.diffuseMapTexId; }
		const int& getSpecularMapTexId() const { return m_drawing_material.specularMapTexId; }
		const int& getNormalMapTexId() const { return m_drawing_material.normalMapTexId; }
		const int& getGlowMapTexId() const { return m_drawing_material.glowMapTexId; }

		TRVertexBuffer& getVertices() { return m_vertices; }
		TRIndexBuffer& getIndices() { return m_indices; }
		const std::vector<TRVertex>& getVertices() const { return m_vertices; }
		const std::vector<unsigned int>& getIndices() const { return m_indices; }

		void clear();

	protected:
		TRVertexBuffer m_vertices;
		TRIndexBuffer  m_indices;

		struct DrawableMaterialTex
		{
			int diffuseMapTexId = -1;
			int specularMapTexId = -1;
			int normalMapTexId = -1;
			int glowMapTexId = -1;
		};
		DrawableMaterialTex m_drawing_material;
	};

	using TRDrawableBuffer = std::vector<TRDrawableSubMesh>;

	class TRDrawableMesh
	{
	public:
		typedef std::shared_ptr<TRDrawableMesh> ptr;

		TRDrawableMesh(const std::string &path, bool generatedMipmap);

		void clear();

		void setPolygonMode(TRPolygonMode mode) { m_drawing_config.polygonMode = mode; }
		void setCullfaceMode(TRCullFaceMode mode) { m_drawing_config.cullfaceMode = mode; }
		void setDepthtestMode(TRDepthTestMode mode) { m_drawing_config.depthtestMode = mode; }
		void setDepthwriteMode(TRDepthWriteMode mode) { m_drawing_config.depthwriteMode = mode; }
		void setAlphablendMode(TRAlphaBlendingMode mode) { m_drawing_config.alphaBlendMode = mode; }
		void setModelMatrix(const glm::mat4& mat) { m_drawing_config.modelMatrix = mat; }
		void setLightingMode(TRLightingMode mode) { m_drawing_config.lightingMode = mode; }

		//Setting

		void setAmbientCoff(const glm::vec3 &cof) { m_drawing_material.kA = cof; }
		void setDiffuseCoff(const glm::vec3 &cof) { m_drawing_material.kD = cof; }
		void setSpecularCoff(const glm::vec3 &cof) { m_drawing_material.kS = cof; }
		void setEmissionCoff(const glm::vec3 &cof) { m_drawing_material.kE = cof; }
		void setSpecularExponent(const float &cof) { m_drawing_material.shininess = cof; }
		void setTransparency(const float &alpha) { m_drawing_material.transparency = alpha; }

		//Getter
		const glm::vec3& getAmbientCoff() const { return m_drawing_material.kA; }
		const glm::vec3& getDiffuseCoff() const { return m_drawing_material.kD; }
		const glm::vec3& getSpecularCoff() const { return m_drawing_material.kS; }
		const glm::vec3& getEmissionCoff() const { return m_drawing_material.kE; }
		const float& getSpecularExponent() const { return m_drawing_material.shininess; }
		const float& getTransparency() const { return m_drawing_material.transparency; }

		TRPolygonMode getPolygonMode() const { return m_drawing_config.polygonMode; }
		TRCullFaceMode getCullfaceMode() const { return m_drawing_config.cullfaceMode; }
		TRDepthTestMode getDepthtestMode() const { return m_drawing_config.depthtestMode; }
		TRDepthWriteMode getDepthwriteMode() const { return m_drawing_config.depthwriteMode; }
		TRAlphaBlendingMode getAlphablendMode() const { return m_drawing_config.alphaBlendMode; }
		const glm::mat4& getModelMatrix() const { return m_drawing_config.modelMatrix; }
		TRLightingMode getLightingMode() const { return m_drawing_config.lightingMode; }

		unsigned int getDrawableMaxFaceNums() const;
		TRDrawableBuffer& getDrawableSubMeshes() { return m_drawables; }

	protected:
		void importMeshFromFile(const std::string &path, bool generatedMipmap = true);

	protected:
		TRDrawableBuffer m_drawables;

		//Configuration
		struct DrawableConfig
		{
			TRPolygonMode polygonMode = TRPolygonMode::TR_TRIANGLE_FILL;
			TRCullFaceMode cullfaceMode = TRCullFaceMode::TR_CULL_BACK;
			TRDepthTestMode depthtestMode = TRDepthTestMode::TR_DEPTH_TEST_ENABLE;
			TRDepthWriteMode depthwriteMode = TRDepthWriteMode::TR_DEPTH_WRITE_ENABLE;
			TRAlphaBlendingMode alphaBlendMode = TRAlphaBlendingMode::TR_ALPHA_DISABLE;
			TRLightingMode lightingMode = TRLightingMode::TR_LIGHTING_ENABLE;
			glm::mat4 modelMatrix = glm::mat4(1.0f);
		};
		DrawableConfig m_drawing_config;

		//Material
		struct DrawableMaterialCof
		{
			glm::vec3 kA = glm::vec3(0.0f);//Ambient coefficient
			glm::vec3 kD = glm::vec3(1.0f);//Diffuse coefficient
			glm::vec3 kS = glm::vec3(0.0f);//Specular coefficient
			glm::vec3 kE = glm::vec3(0.0f);//Emission
			float shininess = 1.0f;		   //Specular highlight exponment
			float transparency = 1.0f;	   //Transparency
		};
		DrawableMaterialCof m_drawing_material;

	};
}

#endif