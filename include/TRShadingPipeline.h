#ifndef TRSHADERPIPELINE_H
#define TRSHADERPIPELINE_H

#include <vector>
#include <memory>

#include "glm/glm.hpp"

#include "TRLight.h"
#include "TRTexture2D.h"
#include "TRParallelWrapper.h"
#include "TRPixelSampler.h"

namespace TinyRenderer
{
	class TRShadingPipeline
	{
	public:
		typedef std::shared_ptr<TRShadingPipeline> ptr;
		
		struct FragmentData;
		struct VertexData
		{
			glm::vec3 m_pos;  //World space position
			glm::vec3 m_nor;  //World space normal
			glm::vec2 m_tex;	//World space texture coordinate
			glm::vec4 m_cpos; //Clip space position
			glm::ivec2 m_spos;//Screen space position
			glm::mat3 m_tbn;  //Tangent, bitangent, normal matrix
			bool m_needInterpolatedTBN = false;
			float m_rhw;

			VertexData() = default;
			VertexData(const glm::ivec2 &screenPos) : m_spos(screenPos) {}

			//Linear interpolation
			static VertexData lerp(const VertexData &v0, const VertexData &v1, float frac);

			static FragmentData barycentricLerp(const VertexData &v0, const VertexData &v1, const VertexData &v2, const glm::vec3 &w);
			static float barycentricLerp(const float &d0, const float &d1, const float &d2, const glm::vec3 &w);

			//Perspective correction for interpolation
			static void prePerspCorrection(VertexData &v);
		};

		struct FragmentData
		{
		public:
			glm::vec3 m_pos;  //World space position
			glm::vec3 m_nor;  //World space normal
			glm::vec2 m_tex;	//World space texture coordinate
			glm::ivec2 m_spos;//Screen space position
			glm::mat3 m_tbn;  //Tangent, bitangent, normal matrix
			float m_rhw;
			
			//MSAA Mask
			//Note: each sampling point should have its own depth
			TRMaskPixelSampler m_coverage = 0;
			TRDepthPixelSampler m_coverageDepth = 0.0f;

			FragmentData() = default;
			FragmentData(const glm::ivec2 &screenPos) : m_spos(screenPos) {}

			static void aftPrespCorrection(FragmentData &v);

		};

		//2x2 fragments block for calculating dFdx and dFdy.
		class QuadFragments
		{
		public:
			/*************************************
			 *   f2--f3
			 *   |   |
			 *   f0--f1
			 *   f0 -> (x+0, y+0), f1 -> (x+1,y+0 )
			 *   f2 -> (x+0, y+1), f3 -> (x+1,y+1)
			 ************************************/
			FragmentData m_fragments[4];

			//Forward differencing
			//Note: Need to handle the boundary condition.
			inline float dUdx() const { return m_fragments[1].m_tex.x - m_fragments[0].m_tex.x; }
			inline float dUdy() const { return m_fragments[2].m_tex.x - m_fragments[0].m_tex.x; }
			inline float dVdx() const { return m_fragments[1].m_tex.y - m_fragments[0].m_tex.y;	}
			inline float dVdy() const { return m_fragments[2].m_tex.y - m_fragments[0].m_tex.y; }
		
			//Perspective correction restore
			inline void aftPrespCorrectionForBlocks()
			{
				TRShadingPipeline::FragmentData::aftPrespCorrection(m_fragments[0]);
				TRShadingPipeline::FragmentData::aftPrespCorrection(m_fragments[1]);
				TRShadingPipeline::FragmentData::aftPrespCorrection(m_fragments[2]);
				TRShadingPipeline::FragmentData::aftPrespCorrection(m_fragments[3]);
			}
		};

		virtual ~TRShadingPipeline() = default;

		//Vertex shader settting
		void setModelMatrix(const glm::mat4 &model) 
		{ 
			m_modelMatrix = model;
			//Refs: https://learnopengl-cn.github.io/02%20Lighting/02%20Basic%20Lighting/
			m_invTransModelMatrix = glm::mat3(glm::transpose(glm::inverse(m_modelMatrix)));
		}
		void setViewProjectMatrix(const glm::mat4 &vp) { m_viewProjectMatrix = vp; }
		void setLightingEnable(bool enable) { m_lightingEnable = enable; }

		//Fragment shader setting
		void setAmbientCoef(const glm::vec3 &ka) { m_kA = ka; }
		void setDiffuseCoef(const glm::vec3 &kd) { m_kD = kd; }
		void setSpecularCoef(const glm::vec3 &ks) { m_kS = ks; }
		void setEmissionColor(const glm::vec3 &ke) { m_kE = ke; }
		void setTransparency(const float &alpha) { m_transparency = alpha; }
		void setDiffuseTexId(const int &id) { m_diffuseTexId = id; }
		void setSpecularTexId(const int &id) { m_specularTexId = id; }
		void setNormalTexId(const int &id) { m_normalTexId = id; }
		void setGlowTexId(const int &id) { m_glowTexId = id; }
		void setShininess(const float &shininess) { m_shininess = shininess; }

		//Shaders
		virtual void vertexShader(VertexData &vertex) const = 0;
		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const = 0;

		//Rasterization
		static void rasterizeFillEdgeFunction(
			const VertexData &v0,
			const VertexData &v1,
			const VertexData &v2,
			const unsigned int &screenWidth,
			const unsigned int &screenHeight,
			std::vector<QuadFragments> &rasterized_points);

		//Textures and lights setting
		static int uploadTexture2D(TRTexture2D::ptr tex);
		static TRTexture2D::ptr getTexture2D(int index);
		static int addLight(TRLight::ptr lightSource);
		static TRLight::ptr getLight(int index);
		static void setExposure(const float &exposure) { m_exposure = exposure; }
		static void setViewerPos(const glm::vec3 &viewer) { m_viewerPos = viewer; }

		//Texture sampling
		static glm::vec4 texture2D(const unsigned int &id, const glm::vec2 &uv, 
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy);

	protected:

		glm::mat4 m_modelMatrix = glm::mat4(1.0f);
		glm::mat3 m_invTransModelMatrix = glm::mat3(1.0f);
		glm::mat4 m_viewProjectMatrix = glm::mat4(1.0f);

		//Global shading setttings
		static std::vector<TRTexture2D::ptr> m_globalTextureUnits;
		static std::vector<TRLight::ptr> m_lights;
		static glm::vec3 m_viewerPos;
		static float m_exposure;

		//Material setting
		glm::vec3 m_kA = glm::vec3(0.0f);
		glm::vec3 m_kD = glm::vec3(1.0f);
		glm::vec3 m_kS = glm::vec3(0.0f);
		glm::vec3 m_kE = glm::vec3(0.0f);
		float m_transparency = 1.0f;
		float m_shininess = 0.0f;
		int m_diffuseTexId = -1;
		int m_specularTexId = -1;
		int m_normalTexId = -1;
		int m_glowTexId = -1;

		bool m_lightingEnable = true;
	};
}

#endif