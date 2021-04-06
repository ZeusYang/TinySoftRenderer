#ifndef TRSHADERPIPELINE_H
#define TRSHADERPIPELINE_H

#include <vector>
#include <memory>

#include "glm/glm.hpp"

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
			glm::vec4 pos;  //World space position
			glm::vec3 col;  //World space color
			glm::vec3 nor;  //World space normal
			glm::vec2 tex;	//World space texture coordinate
			glm::vec4 cpos; //Clip space position
			glm::ivec2 spos;//Screen space position
			glm::mat3 TBN;  //Tangent, bitangent, normal matrix
			float rhw;

			VertexData() = default;
			VertexData(const glm::ivec2 &screen_pos) : spos(screen_pos) {}

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
			glm::vec4 pos;  //World space position
			glm::vec3 col;  //World space color
			glm::vec3 nor;  //World space normal
			glm::vec2 tex;	//World space texture coordinate
			glm::ivec2 spos;//Screen space position
			glm::mat3 TBN;  //Tangent, bitangent, normal matrix
			float rhw;
			
			//MSAA Mask
			//Note: each sampling point should have its own depth
			TRMaskPixelSampler coverage = 0;
			TRDepthPixelSampler coverage_depth = 0.0f;

			FragmentData() = default;
			FragmentData(const glm::ivec2 &screen_pos) : spos(screen_pos) {}

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
			FragmentData fragments[4];

			//Forward differencing
			//Note: Need to handle the boundary condition.
			inline float dUdx() const { return fragments[1].tex.x - fragments[0].tex.x; }
			inline float dUdy() const { return fragments[2].tex.x - fragments[0].tex.x; }
			inline float dVdx() const { return fragments[1].tex.y - fragments[0].tex.y;	}
			inline float dVdy() const { return fragments[2].tex.y - fragments[0].tex.y; }
		
			//Perspective correction restore
			inline void aftPrespCorrectionForBlocks()
			{
				TRShadingPipeline::FragmentData::aftPrespCorrection(fragments[0]);
				TRShadingPipeline::FragmentData::aftPrespCorrection(fragments[1]);
				TRShadingPipeline::FragmentData::aftPrespCorrection(fragments[2]);
				TRShadingPipeline::FragmentData::aftPrespCorrection(fragments[3]);
			}
		};

		virtual ~TRShadingPipeline() = default;

		//Vertex shader settting
		void setModelMatrix(const glm::mat4 &model) 
		{ 
			m_model_matrix = model;
			//Refs: https://learnopengl-cn.github.io/02%20Lighting/02%20Basic%20Lighting/
			m_inv_trans_model_matrix = glm::mat3(glm::transpose(glm::inverse(m_model_matrix)));
		}
		void setViewProjectMatrix(const glm::mat4 &vp) { m_view_project_matrix = vp; }
		void setLightingEnable(bool enable) { m_lighting_enable = enable; }

		//Fragment shader setting
		void setAmbientCoef(const glm::vec3 &ka) { m_ka = ka; }
		void setDiffuseCoef(const glm::vec3 &kd) { m_kd = kd; }
		void setSpecularCoef(const glm::vec3 &ks) { m_ks = ks; }
		void setEmissionColor(const glm::vec3 &ke) { m_ke = ke; }
		void setDiffuseTexId(const int &id) { m_diffuse_tex_id = id; }
		void setSpecularTexId(const int &id) { m_specular_tex_id = id; }
		void setNormalTexId(const int &id) { m_normal_tex_id = id; }
		void setGlowTexId(const int &id) { m_glow_tex_id = id; }
		void setShininess(const float &shininess) { m_shininess = shininess; }

		//Shaders
		virtual void vertexShader(VertexData &vertex) const = 0;
		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const = 0;

		//Rasterization
		static void rasterize_fill_edge_function(
			const VertexData &v0,
			const VertexData &v1,
			const VertexData &v2,
			const unsigned int &screen_width,
			const unsigned int &screene_height,
			std::vector<QuadFragments> &rasterized_points);

		//Textures and lights setting
		static int upload_texture_2D(TRTexture2D::ptr tex);
		static TRTexture2D::ptr getTexture2D(int index);
		static int addPointLight(glm::vec3 pos, glm::vec3 atten, glm::vec3 color);
		static TRPointLight &getPointLight(int index);
		static void setViewerPos(const glm::vec3 &viewer) { m_viewer_pos = viewer; }

		//Texture sampling
		static glm::vec4 texture2D(const unsigned int &id, const glm::vec2 &uv, 
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy);

	protected:

		glm::mat4 m_model_matrix = glm::mat4(1.0f);
		glm::mat3 m_inv_trans_model_matrix = glm::mat3(1.0f);
		glm::mat4 m_view_project_matrix = glm::mat4(1.0f);

		//Global shading setttings
		static std::vector<TRTexture2D::ptr> m_global_texture_units;
		static std::vector<TRPointLight> m_point_lights;
		static glm::vec3 m_viewer_pos;

		//Material setting
		glm::vec3 m_ka = glm::vec3(0.0f);
		glm::vec3 m_kd = glm::vec3(1.0f);
		glm::vec3 m_ks = glm::vec3(0.0f);
		glm::vec3 m_ke = glm::vec3(0.0f);
		float m_shininess = 0.0f;
		int m_diffuse_tex_id = -1;
		int m_specular_tex_id = -1;
		int m_normal_tex_id = -1;
		int m_glow_tex_id = -1;

		bool m_lighting_enable = true;
	};
}

#endif