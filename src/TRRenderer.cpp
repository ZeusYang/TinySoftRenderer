#include "TRRenderer.h"

#include "glm/gtc/matrix_transform.hpp"

#include "TRShadingPipeline.h"
#include "TRShaderProgram.h"
#include "TRMathUtils.h"
#include "TRParallelWrapper.h"

#include <cmath>

namespace TinyRenderer
{

	TRRenderer::TRRenderer(int width, int height)
		: m_backBuffer(nullptr), m_frontBuffer(nullptr)
	{
		//Double buffer to avoid flickering
		m_backBuffer = std::make_shared<TRFrameBuffer>(width, height);
		m_frontBuffer = std::make_shared<TRFrameBuffer>(width, height);

		//Setup viewport matrix (ndc space -> screen space)
		m_viewportMatrix = TRMathUtils::calcViewPortMatrix(width, height);
	}

	void TRRenderer::addDrawableMesh(TRDrawableMesh::ptr mesh)
	{
		m_drawableMeshes.push_back(mesh);
	}

	void TRRenderer::addDrawableMesh(const std::vector<TRDrawableMesh::ptr> &meshes)
	{
		m_drawableMeshes.insert(m_drawableMeshes.end(), meshes.begin(), meshes.end());
	}

	void TRRenderer::unloadDrawableMesh()
	{
		for (size_t i = 0; i < m_drawableMeshes.size(); ++i)
		{
			m_drawableMeshes[i]->clear();
		}
		std::vector<TRDrawableMesh::ptr>().swap(m_drawableMeshes);
	}

	void TRRenderer::setViewMatrix(const glm::mat4 &view)
	{
		m_mvp_dirty = true;
		m_viewMatrix = view;
	}

	void TRRenderer::setModelMatrix(const glm::mat4 &model)
	{
		m_mvp_dirty = true;
		m_modelMatrix = model;
	}

	void TRRenderer::setProjectMatrix(const glm::mat4 &project, float near, float far)
	{
		m_mvp_dirty = true;
		m_projectMatrix = project;
		m_frustum_near_far = glm::vec2(near, far);
	}

	void TRRenderer::setShaderPipeline(TRShadingPipeline::ptr shader)
	{
		m_shader_handler = shader;
	}

	void TRRenderer::setViewerPos(const glm::vec3 &viewer)
	{
		if (m_shader_handler == nullptr)
			return;
		m_shader_handler->setViewerPos(viewer);
	}

	glm::mat4 TRRenderer::getMVPMatrix()
	{
		if (m_mvp_dirty)
		{
			m_mvp_matrix = m_projectMatrix * m_viewMatrix * m_modelMatrix;
			m_mvp_dirty = false;
		}
		return m_mvp_matrix;
	}

	void TRRenderer::clearColor(glm::vec4 color)
	{
		m_backBuffer->clear(color);
	}

	int TRRenderer::addPointLight(glm::vec3 pos, glm::vec3 atten, glm::vec3 color)
	{
		return TRShadingPipeline::addPointLight(pos, atten, color);
	}

	TRPointLight &TRRenderer::getPointLight(const int &index)
	{
		return TRShadingPipeline::getPointLight(index);
	}

	unsigned int TRRenderer::renderAllDrawableMeshes()
	{
		if (m_shader_handler == nullptr)
		{
			m_shader_handler = std::make_shared<TR3DShadingPipeline>();
		}

		//Load the matrices
		m_shader_handler->setModelMatrix(m_modelMatrix);
		m_shader_handler->setViewProjectMatrix(m_projectMatrix * m_viewMatrix);

		//Draw a mesh step by step
		unsigned int num_triangles = 0;

		//Pre allocation
		static size_t maxFaces = 0;
		if (maxFaces == 0)
		{
			for (size_t m = 0; m < m_drawableMeshes.size(); ++m)
			{
				maxFaces = std::max(maxFaces, m_drawableMeshes[m]->getMeshFaces().size());
			}
		}
		static std::vector<std::vector<TRShadingPipeline::QuadFragments>> fragments(maxFaces);

		for (size_t m = 0; m < m_drawableMeshes.size(); ++m)
		{
			const auto &drawable = m_drawableMeshes[m];
			//Configuration
			TRPolygonMode polygonMode = drawable->getPolygonMode();
			TRCullFaceMode cullfaceMode = drawable->getCullfaceMode();
			TRDepthTestMode depthtestMode = drawable->getDepthtestMode();
			TRDepthWriteMode depthwriteMode = drawable->getDepthwriteMode();

			//Setup the shading options
			m_shader_handler->setModelMatrix(drawable->getModelMatrix());
			m_shader_handler->setLightingEnable(drawable->getLightingMode() == TRLightingMode::TR_LIGHTING_ENABLE);
			m_shader_handler->setAmbientCoef(drawable->getAmbientCoff());
			m_shader_handler->setDiffuseCoef(drawable->getDiffuseCoff());
			m_shader_handler->setSpecularCoef(drawable->getSpecularCoff());
			m_shader_handler->setEmissionColor(drawable->getEmissionCoff());
			m_shader_handler->setDiffuseTexId(drawable->getDiffuseMapTexId());
			m_shader_handler->setSpecularTexId(drawable->getSpecularMapTexId());
			m_shader_handler->setNormalTexId(drawable->getNormalMapTexId());
			m_shader_handler->setGlowTexId(drawable->getGlowMapTexId());
			m_shader_handler->setShininess(drawable->getSpecularExponent());

			const auto& vertices = drawable->getVerticesAttrib();
			const auto& faces = drawable->getMeshFaces();

			parallelFor((size_t)0, (size_t)faces.size(), [&](const size_t &f)
			{
				//A triangle as primitive
				TRShadingPipeline::VertexData v[3];
				{
					glm::mat3 TBN_mat;
					TBN_mat[0] = faces[f].tangent;
					TBN_mat[1] = faces[f].bitangent;

					v[0].pos = vertices.vpositions[faces[f].vposIndex[0]];
					v[0].col = glm::vec3(vertices.vcolors[faces[f].vposIndex[0]]);
					v[0].nor = vertices.vnormals[faces[f].vnorIndex[0]];
					v[0].tex = vertices.vtexcoords[faces[f].vtexIndex[0]];

					v[1].pos = vertices.vpositions[faces[f].vposIndex[1]];
					v[1].col = glm::vec3(vertices.vcolors[faces[f].vposIndex[1]]);
					v[1].nor = vertices.vnormals[faces[f].vnorIndex[1]];
					v[1].tex = vertices.vtexcoords[faces[f].vtexIndex[1]];

					v[2].pos = vertices.vpositions[faces[f].vposIndex[2]];
					v[2].col = glm::vec3(vertices.vcolors[faces[f].vposIndex[2]]);
					v[2].nor = vertices.vnormals[faces[f].vnorIndex[2]];
					v[2].tex = vertices.vtexcoords[faces[f].vtexIndex[2]];

					v[0].TBN = v[1].TBN = v[2].TBN = TBN_mat;
				}

				//Vertex shader stage
				std::vector<TRShadingPipeline::VertexData> clipped_vertices;
				{
					//Vertex shader
					{
						m_shader_handler->vertexShader(v[0]);
						m_shader_handler->vertexShader(v[1]);
						m_shader_handler->vertexShader(v[2]);
					}

					//Homogeneous space cliping
					{
						clipped_vertices = clipingSutherlandHodgeman(v[0], v[1], v[2]);
						if (clipped_vertices.empty())
						{
							return;
						}
					}

					//Perspective division
					for (auto &vert : clipped_vertices)
					{
						//From clip space -> ndc space
						TRShadingPipeline::VertexData::prePerspCorrection(vert);
						vert.cpos /= vert.cpos.w;
					}
				}

				int num_verts = clipped_vertices.size();
				for (int i = 0; i < num_verts - 2; ++i)
				{
					//Triangle assembly
					TRShadingPipeline::VertexData vert[3] = {
							clipped_vertices[0],
							clipped_vertices[i + 1],
							clipped_vertices[i + 2] };

					//Rasterization stage
					{
						//Transform to screen space & Rasterization
						{
							vert[0].spos = glm::ivec2(m_viewportMatrix * vert[0].cpos + glm::vec4(0.5f));
							vert[1].spos = glm::ivec2(m_viewportMatrix * vert[1].cpos + glm::vec4(0.5f));
							vert[2].spos = glm::ivec2(m_viewportMatrix * vert[2].cpos + glm::vec4(0.5f));

							//Backface culling
							{
								if (isBackFacing(vert[0].spos, vert[1].spos, vert[2].spos, cullfaceMode))
								{
									continue;
								}
							}

							m_shader_handler->rasterize_fill_edge_function(vert[0], vert[1], vert[2],
								m_backBuffer->getWidth(), m_backBuffer->getHeight(), fragments[f]);
						}
					}
				}
			}, TRExecutionPolicy::TR_PARALLEL);

			//Fragment shader & Depth testing

			auto fragment_func = [&](TRShadingPipeline::VertexData &fragment,
				const glm::vec2 &dUVdx, const glm::vec2 &dUVdy)
			{
				//Note: spos.x equals -1 -> invalid fragment
				if (fragment.spos.x == -1)
					return;
				if (depthtestMode == TRDepthTestMode::TR_DEPTH_TEST_ENABLE &&
					m_backBuffer->readDepth(fragment.spos.x, fragment.spos.y) > fragment.cpos.z)
				{
					glm::vec4 fragColor;
					m_shader_handler->fragmentShader(fragment, fragColor, dUVdx, dUVdy);
					m_backBuffer->writeColor(fragment.spos.x, fragment.spos.y, fragColor);
					if (depthwriteMode == TRDepthWriteMode::TR_DEPTH_WRITE_ENABLE)
					{
						m_backBuffer->writeDepth(fragment.spos.x, fragment.spos.y, fragment.cpos.z);
					}
				}
			};

			parallelFor((size_t)0, (size_t)fragments.size(), [&](const size_t &f)
			{
				if (fragments[f].empty())
					return;

				++ num_triangles;

				//Note: 2x2 fragment block as an execution unit for calculating dFdx, dFdy.
				parallelFor((size_t)0, (size_t)fragments[f].size(), [&](const size_t &index)
				{
					auto &block = fragments[f][index];

					//Perspective correction restore
					block.aftPrespCorrectionForBlocks();

					//Calculate dUVdx, dUVdy for mipmap
					glm::vec2 dUVdx(block.dUdx(), block.dVdx());
					glm::vec2 dUVdy(block.dUdy(), block.dVdy());

					fragment_func(block.fragments[0], dUVdx, dUVdy);
					fragment_func(block.fragments[1], dUVdx, dUVdy);
					fragment_func(block.fragments[2], dUVdx, dUVdy);
					fragment_func(block.fragments[3], dUVdx, dUVdy);

				}, TRExecutionPolicy::TR_PARALLEL);
				fragments[f].clear();

			}, TRExecutionPolicy::TR_SERIAL); //Note: parallelization herein is not good at all.
		}

		//Swap double buffers
		{
			std::swap(m_backBuffer, m_frontBuffer);
		}

		return num_triangles;
	}

	unsigned char* TRRenderer::commitRenderedColorBuffer()
	{
		return m_frontBuffer->getColorBuffer();
	}

	std::vector<TRShadingPipeline::VertexData> TRRenderer::clipingSutherlandHodgeman(
		const TRShadingPipeline::VertexData &v0,
		const TRShadingPipeline::VertexData &v1,
		const TRShadingPipeline::VertexData &v2) const
	{
		//Clipping in the homogeneous clipping space
		//Refs:
		//https://fabiensanglard.net/polygon_codec/clippingdocument/Clipping.pdf
		//https://fabiensanglard.net/polygon_codec/

		//Optimization: complete outside or complete inside
		//Note: in the following situation, we could return the answer without complicate cliping,
		//      and this optimization should be very important.
		{
			auto isPointInsideInClipingFrustum = [](const glm::vec4 &p, const float &near, const float &far) -> bool
			{
				return (p.x <= p.w && p.x >= -p.w) && (p.y <= p.w && p.y >= -p.w)
					&& (p.z <= p.w && p.z >= -p.w) && (p.w <= far && p.w >= near);
			};

			//Totally inside
			if (isPointInsideInClipingFrustum(v0.cpos, m_frustum_near_far.x, m_frustum_near_far.y) &&
				isPointInsideInClipingFrustum(v1.cpos, m_frustum_near_far.x, m_frustum_near_far.y) &&
				isPointInsideInClipingFrustum(v2.cpos, m_frustum_near_far.x, m_frustum_near_far.y))
			{
				return { v0,v1,v2 };
			}

			//Totally outside
			if (v0.cpos.w < m_frustum_near_far.x && v1.cpos.w < m_frustum_near_far.x && v2.cpos.w < m_frustum_near_far.x)
				return{};
			if (v0.cpos.w > m_frustum_near_far.y && v1.cpos.w > m_frustum_near_far.y && v2.cpos.w > m_frustum_near_far.y)
				return{};
			if (v0.cpos.x > v0.cpos.w && v1.cpos.x > v1.cpos.w && v2.cpos.x > v2.cpos.w)
				return{};
			if (v0.cpos.x < -v0.cpos.w && v1.cpos.x < -v1.cpos.w && v2.cpos.x < -v2.cpos.w)
				return{};
			if (v0.cpos.y > v0.cpos.w && v1.cpos.y > v1.cpos.w && v2.cpos.y > v2.cpos.w)
				return{};
			if (v0.cpos.y < -v0.cpos.w && v1.cpos.y < -v1.cpos.w && v2.cpos.y < -v2.cpos.w)
				return{};
			if (v0.cpos.z > v0.cpos.w && v1.cpos.z > v1.cpos.w && v2.cpos.z > v2.cpos.w)
				return{};
			if (v0.cpos.z < -v0.cpos.w && v1.cpos.z < -v1.cpos.w && v2.cpos.z < -v2.cpos.w)
				return{};
		}

		std::vector<TRShadingPipeline::VertexData> inside_vertices;
		std::vector<TRShadingPipeline::VertexData> tmp = { v0, v1, v2 };
		enum Axis { X = 0, Y = 1, Z = 2 };

		//w=x plane & w=-x plane
		{
			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::X, +1);
			tmp = inside_vertices;

			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::X, -1);
			tmp = inside_vertices;
		}

		//w=y plane & w=-y plane
		{
			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::Y, +1);
			tmp = inside_vertices;

			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::Y, -1);
			tmp = inside_vertices;
		}

		//w=z plane & w=-z plane
		{
			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::Z, +1);
			tmp = inside_vertices;

			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::Z, -1);
			tmp = inside_vertices;
		}

		//w=1e-5 plane
		{
			inside_vertices = {};
			int num_verts = tmp.size();
			constexpr float w_clipping_plane = 1e-5;
			for (int i = 0; i < num_verts; ++i)
			{
				const auto &beg_vert = tmp[(i - 1 + num_verts) % num_verts];
				const auto &end_vert = tmp[i];
				float beg_is_inside = (beg_vert.cpos.w < w_clipping_plane) ? -1 : 1;
				float end_is_inside = (end_vert.cpos.w < w_clipping_plane) ? -1 : 1;
				//One of them is outside
				if (beg_is_inside * end_is_inside < 0)
				{
					// t = (w_clipping_plane-w1)/((w1-w2)
					float t = (w_clipping_plane - beg_vert.cpos.w) / (beg_vert.cpos.w - end_vert.cpos.w);
					auto intersected_vert = TRShadingPipeline::VertexData::lerp(beg_vert, end_vert, t);
					inside_vertices.push_back(intersected_vert);
				}
				//If current vertices is inside
				if (end_is_inside > 0)
				{
					inside_vertices.push_back(end_vert);
				}
			}
		}

		return inside_vertices;
	}

	std::vector<TRShadingPipeline::VertexData> TRRenderer::clipingSutherlandHodgeman_aux(
		const std::vector<TRShadingPipeline::VertexData> &polygon,
		const int &axis,
		const int &side) const
	{
		std::vector<TRShadingPipeline::VertexData> inside_polygon;

		int num_verts = polygon.size();
		for (int i = 0; i < num_verts; ++i)
		{
			const auto &beg_vert = polygon[(i - 1 + num_verts) % num_verts];
			const auto &end_vert = polygon[i];
			char beg_is_inside = ((side * (beg_vert.cpos[axis]) <= beg_vert.cpos.w) ? 1 : -1);
			char end_is_inside = ((side * (end_vert.cpos[axis]) <= end_vert.cpos.w) ? 1 : -1);
			//One of them is outside
			if (beg_is_inside * end_is_inside < 0)
			{
				// t = (w1 - y1)/((w1-y1)-(w2-y2))
				float t = (beg_vert.cpos.w - side * beg_vert.cpos[axis])
					/ ((beg_vert.cpos.w - side * beg_vert.cpos[axis]) - (end_vert.cpos.w - side * end_vert.cpos[axis]));
				auto intersected_vert = TRShadingPipeline::VertexData::lerp(beg_vert, end_vert, t);
				inside_polygon.push_back(intersected_vert);
			}
			//If current vertices is inside
			if (end_is_inside > 0)
			{
				inside_polygon.push_back(end_vert);
			}
		}
		return inside_polygon;
	}

	bool TRRenderer::isBackFacing(const glm::ivec2 &v0, const glm::ivec2 &v1, const glm::ivec2 &v2, TRCullFaceMode mode) const
	{
		//Back face culling in screen space
		if (mode == TRCullFaceMode::TR_CULL_DISABLE)
			return false;

		auto e1 = v1 - v0;
		auto e2 = v2 - v0;

		int orient = e1.x * e2.y - e1.y * e2.x;

		return (mode == TRCullFaceMode::TR_CULL_BACK) ? (orient > 0) : (orient < 0);
	}

}
