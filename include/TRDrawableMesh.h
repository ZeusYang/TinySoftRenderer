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

		glm::vec3 m_vpositions = glm::vec3(0, 0, 0);
		glm::vec2 m_vtexcoords = glm::vec2(0, 0);
		glm::vec3 m_vnormals   = glm::vec3(0, 1, 0);

		// tangent
		glm::vec3 m_vtangent;
		// bitangent
		glm::vec3 m_vbitangent;
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

		void setDiffuseMapTexId(const int &id) { m_drawingMaterial.m_diffuseMapTexId = id; }
		void setSpecularMapTexId(const int &id) { m_drawingMaterial.m_specularMapTexId = id; }
		void setNormalMapTexId(const int &id) { m_drawingMaterial.m_normalMapTexId = id; }
		void setGlowMapTexId(const int &id) { m_drawingMaterial.m_glowMapTexId = id; }

		const int& getDiffuseMapTexId() const { return m_drawingMaterial.m_diffuseMapTexId; }
		const int& getSpecularMapTexId() const { return m_drawingMaterial.m_specularMapTexId; }
		const int& getNormalMapTexId() const { return m_drawingMaterial.m_normalMapTexId; }
		const int& getGlowMapTexId() const { return m_drawingMaterial.m_glowMapTexId; }

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
			int m_diffuseMapTexId = -1;
			int m_specularMapTexId = -1;
			int m_normalMapTexId = -1;
			int m_glowMapTexId = -1;
		};
		DrawableMaterialTex m_drawingMaterial;
	};

	using TRDrawableBuffer = std::vector<TRDrawableSubMesh>;

	class TRDrawableMesh
	{
	public:
		typedef std::shared_ptr<TRDrawableMesh> ptr;

		TRDrawableMesh(const std::string &path, bool generatedMipmap);

		void clear();

		void setCullfaceMode(TRCullFaceMode mode) { m_drawing_config.m_cullfaceMode = mode; }
		void setDepthtestMode(TRDepthTestMode mode) { m_drawing_config.m_depthtestMode = mode; }
		void setDepthwriteMode(TRDepthWriteMode mode) { m_drawing_config.m_depthwriteMode = mode; }
		void setAlphablendMode(TRAlphaBlendingMode mode) { m_drawing_config.m_alphaBlendMode = mode; }
		void setModelMatrix(const glm::mat4& mat) { m_drawing_config.m_modelMatrix = mat; }
		void setLightingMode(TRLightingMode mode) { m_drawing_config.m_lightingMode = mode; }

		//Setting

		void setAmbientCoff(const glm::vec3 &cof) { m_drawingMaterial.m_kA = cof; }
		void setDiffuseCoff(const glm::vec3 &cof) { m_drawingMaterial.m_kD = cof; }
		void setSpecularCoff(const glm::vec3 &cof) { m_drawingMaterial.m_kS = cof; }
		void setEmissionCoff(const glm::vec3 &cof) { m_drawingMaterial.m_kE = cof; }
		void setSpecularExponent(const float &cof) { m_drawingMaterial.m_shininess = cof; }
		void setTransparency(const float &alpha) { m_drawingMaterial.m_transparency = alpha; }

		//Getter
		const glm::vec3& getAmbientCoff() const { return m_drawingMaterial.m_kA; }
		const glm::vec3& getDiffuseCoff() const { return m_drawingMaterial.m_kD; }
		const glm::vec3& getSpecularCoff() const { return m_drawingMaterial.m_kS; }
		const glm::vec3& getEmissionCoff() const { return m_drawingMaterial.m_kE; }
		const float& getSpecularExponent() const { return m_drawingMaterial.m_shininess; }
		const float& getTransparency() const { return m_drawingMaterial.m_transparency; }

		TRCullFaceMode getCullfaceMode() const { return m_drawing_config.m_cullfaceMode; }
		TRDepthTestMode getDepthtestMode() const { return m_drawing_config.m_depthtestMode; }
		TRDepthWriteMode getDepthwriteMode() const { return m_drawing_config.m_depthwriteMode; }
		TRAlphaBlendingMode getAlphablendMode() const { return m_drawing_config.m_alphaBlendMode; }
		const glm::mat4& getModelMatrix() const { return m_drawing_config.m_modelMatrix; }
		TRLightingMode getLightingMode() const { return m_drawing_config.m_lightingMode; }

		unsigned int getDrawableMaxFaceNums() const;
		TRDrawableBuffer& getDrawableSubMeshes() { return m_drawables; }

	protected:
		void importMeshFromFile(const std::string &path, bool generatedMipmap = true);

	protected:
		TRDrawableBuffer m_drawables;

		//Configuration
		struct DrawableConfig
		{
			TRCullFaceMode m_cullfaceMode = TRCullFaceMode::TR_CULL_BACK;
			TRDepthTestMode m_depthtestMode = TRDepthTestMode::TR_DEPTH_TEST_ENABLE;
			TRDepthWriteMode m_depthwriteMode = TRDepthWriteMode::TR_DEPTH_WRITE_ENABLE;
			TRAlphaBlendingMode m_alphaBlendMode = TRAlphaBlendingMode::TR_ALPHA_DISABLE;
			TRLightingMode m_lightingMode = TRLightingMode::TR_LIGHTING_ENABLE;
			glm::mat4 m_modelMatrix = glm::mat4(1.0f);
		};
		DrawableConfig m_drawing_config;

		//Material
		struct DrawableMaterialCof
		{
			glm::vec3 m_kA = glm::vec3(0.0f);//Ambient coefficient
			glm::vec3 m_kD = glm::vec3(1.0f);//Diffuse coefficient
			glm::vec3 m_kS = glm::vec3(0.0f);//Specular coefficient
			glm::vec3 m_kE = glm::vec3(0.0f);//Emission
			float m_shininess = 1.0f;		   //Specular highlight exponment
			float m_transparency = 1.0f;	   //Transparency
		};
		DrawableMaterialCof m_drawingMaterial;

	};
}

#endif