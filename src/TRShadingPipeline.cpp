#include "TRShadingPipeline.h"

#include <algorithm>
#include <iostream>

#include "TRParallelWrapper.h"

namespace TinyRenderer
{
	//----------------------------------------------VertexData----------------------------------------------

	TRShadingPipeline::VertexData TRShadingPipeline::VertexData::lerp(
		const TRShadingPipeline::VertexData &v0,
		const TRShadingPipeline::VertexData &v1,
		float frac)
	{
		//Linear interpolation
		VertexData result;
		result.pos = (1.0f - frac) * v0.pos + frac * v1.pos;
		result.col = (1.0f - frac) * v0.col + frac * v1.col;
		result.nor = (1.0f - frac) * v0.nor + frac * v1.nor;
		result.tex = (1.0f - frac) * v0.tex + frac * v1.tex;
		result.cpos = (1.0f - frac) * v0.cpos + frac * v1.cpos;
		result.spos.x = (1.0f - frac) * v0.spos.x + frac * v1.spos.x;
		result.spos.y = (1.0f - frac) * v0.spos.y + frac * v1.spos.y;
		result.rhw = (1.0f - frac) * v0.rhw + frac * v1.rhw;

		result.TBN = v0.TBN;

		return result;
	}

	TRShadingPipeline::FragmentData TRShadingPipeline::VertexData::barycentricLerp(
		const VertexData &v0, 
		const VertexData &v1, 
		const VertexData &v2,
		const glm::vec3 &w)
	{
		FragmentData result;
		result.pos = w.x * v0.pos + w.y * v1.pos + w.z * v2.pos;
		result.col = w.x * v0.col + w.y * v1.col + w.z * v2.col;
		result.nor = w.x * v0.nor + w.y * v1.nor + w.z * v2.nor;
		result.tex = w.x * v0.tex + w.y * v1.tex + w.z * v2.tex;
		result.rhw = w.x * v0.rhw + w.y * v1.rhw + w.z * v2.rhw;
		result.spos.x = w.x * v0.spos.x + w.y * v1.spos.x + w.z * v2.spos.x;
		result.spos.y = w.x * v0.spos.y + w.y * v1.spos.y + w.z * v2.spos.y;

		result.TBN = v0.TBN;

		return result;
	}

	float TRShadingPipeline::VertexData::barycentricLerp(const float &d0, const float &d1, const float &d2, const glm::vec3 &w)
	{
		return w.x * d0 + w.y * d1 + w.z * d2;
	}

	void TRShadingPipeline::VertexData::prePerspCorrection(VertexData &v)
	{
		//Perspective correction: the world space properties should be multipy by 1/w before rasterization
		//https://zhuanlan.zhihu.com/p/144331875
		v.rhw = 1.0f / v.cpos.w;
		v.pos *= v.rhw;
		v.tex *= v.rhw;
		v.nor *= v.rhw;
		v.col *= v.rhw;

	}

	void TRShadingPipeline::FragmentData::aftPrespCorrection(FragmentData &v)
	{
		//Perspective correction: the world space properties should be multipy by w after rasterization
		//https://zhuanlan.zhihu.com/p/144331875
		float w = 1.0f / v.rhw;
		v.pos *= w;
		v.tex *= w;
		v.nor *= w;
		v.col *= w;

	}

	//----------------------------------------------TRShadingPipeline----------------------------------------------

	std::vector<TRTexture2D::ptr> TRShadingPipeline::m_global_texture_units = {};
	std::vector<TRPointLight> TRShadingPipeline::m_point_lights = {};
	glm::vec3 TRShadingPipeline::m_viewer_pos = glm::vec3(0.0f);

	void TRShadingPipeline::rasterize_fill_edge_function(
		const VertexData &v0,
		const VertexData &v1,
		const VertexData &v2,
		const unsigned int &screen_width,
		const unsigned int &screene_height,
		std::vector<QuadFragments> &rasterized_fragments)
	{
		//Edge function rasterization algorithm
		//Accelerated Half-Space Triangle Rasterization
		//Refs:Mileff P, Nehéz K, Dudra J. Accelerated half-space triangle rasterization[J].
		//     Acta Polytechnica Hungarica, 2015, 12(7): 217-236.
		//	   http://acta.uni-obuda.hu/Mileff_Nehez_Dudra_63.pdf

		VertexData v[] = { v0, v1, v2 };
		glm::ivec2 bounding_min;
		glm::ivec2 bounding_max;
		bounding_min.x = std::max(std::min(v0.spos.x, std::min(v1.spos.x, v2.spos.x)), 0);
		bounding_min.y = std::max(std::min(v0.spos.y, std::min(v1.spos.y, v2.spos.y)), 0);
		bounding_max.x = std::min(std::max(v0.spos.x, std::max(v1.spos.x, v2.spos.x)), (int)screen_width - 1);
		bounding_max.y = std::min(std::max(v0.spos.y, std::max(v1.spos.y, v2.spos.y)), (int)screene_height - 1);

		//Adjust the order
		{
			int orient = 0;
			auto e1 = v1.spos - v0.spos;
			auto e2 = v2.spos - v0.spos;
			orient = e1.x * e2.y - e1.y * e2.x;
			if (orient > 0)
			{
				std::swap(v[1], v[2]);
			}
		}

		const glm::ivec2 &A = v[0].spos;
		const glm::ivec2 &B = v[1].spos;
		const glm::ivec2 &C = v[2].spos;

		const int I01 = A.y - B.y, I02 = B.y - C.y, I03 = C.y - A.y;
		const int J01 = B.x - A.x, J02 = C.x - B.x, J03 = A.x - C.x;
		const int K01 = A.x * B.y - A.y * B.x;
		const int K02 = B.x * C.y - B.y * C.x;
		const int K03 = C.x * A.y - C.y * A.x;

		int F01 = I01 * bounding_min.x + J01 * bounding_min.y + K01;
		int F02 = I02 * bounding_min.x + J02 * bounding_min.y + K02;
		int F03 = I03 * bounding_min.x + J03 * bounding_min.y + K03;

		//Degenerated to a line or a point
		if (F01 + F02 + F03 == 0)
			return;

		rasterized_fragments.reserve((bounding_max.y - bounding_min.y) * (bounding_max.x - bounding_min.x));

		//Top left fill rule
		const float offset = TRMaskPixelSampler::getSamplingNum() >= 4 ? 0.0 : +1.0;
		const int E1_t = (((B.y > A.y) || (A.y == B.y && A.x < B.x)) ? 0 : offset);
		const int E2_t = (((C.y > B.y) || (B.y == C.y && B.x < C.x)) ? 0 : offset);
		const int E3_t = (((A.y > C.y) || (C.y == A.y && C.x < A.x)) ? 0 : offset);

		int Cy1 = F01, Cy2 = F02, Cy3 = F03;
		const float one_div_delta = 1.0f / (F01 + F02 + F03);

		//Strict barycenteric weights calculation
		auto barycentericWeight = [&](const float &x, const float &y) -> glm::vec3
		{
			glm::vec3 s[2];
			s[0] = glm::vec3(v[2].spos.x - v[0].spos.x, v[1].spos.x - v[0].spos.x, v[0].spos.x - x);
			s[1] = glm::vec3(v[2].spos.y - v[0].spos.y, v[1].spos.y - v[0].spos.y, v[0].spos.y - y);
			auto uf = glm::cross(s[0], s[1]);
			return glm::vec3(1.f - (uf.x + uf.y) / uf.z, uf.y / uf.z, uf.x / uf.z);
		};

		auto sampling_is_inside = [&](const int &x, const int &y, const int &Cx1, const int &Cx2, 
			const int &Cx3, FragmentData &p) -> bool
		{
			//Invalid fragment
			if (x > bounding_max.x || y > bounding_max.y)
			{
				p.spos = glm::ivec2(-1);
				return false;
			}
			bool at_least_one_inside = false;
			const int samplingNum = TRMaskPixelSampler::getSamplingNum();
			auto samplingOffsetArray = TRMaskPixelSampler::getSamplingOffsets();
#pragma unroll
			for (int s = 0; s < samplingNum; ++s)
			{
				const auto &offset = samplingOffsetArray[s];
				//Edge function
				const float E1 = Cx1 + offset.x * I01 + offset.y * J01;
				const float E2 = Cx2 + offset.x * I02 + offset.y * J02;
				const float E3 = Cx3 + offset.x * I03 + offset.y * J03;
				//Note: Counter-clockwise winding order
				if ((E1 + E1_t) <= 0 && (E2 + E2_t) <= 0 && (E3 + E3_t) <= 0)
				{
					at_least_one_inside = true;
					p.coverage[s] = 1;//Covered
					//Note: each sampling point should have its own depth
					glm::vec3 uvw = glm::vec3(E2, E3, E1) * one_div_delta;
					p.coverage_depth[s] = VertexData::barycentricLerp(v[0].rhw, v[1].rhw, v[2].rhw, uvw);
				}
			}

			if (!at_least_one_inside)
			{
				p.spos = glm::ivec2(-1);
			}
			return at_least_one_inside;
		};

		for(int y = bounding_min.y;y <= bounding_max.y;y += 2)
		{
			int Cx1 = Cy1, Cx2 = Cy2, Cx3 = Cy3;
#pragma unroll 4
			for (int x = bounding_min.x; x <= bounding_max.x; x += 2)
			{
				//2x2 fragments block
				QuadFragments group;
				bool inside0 = sampling_is_inside(x, y, Cx1, Cx2, Cx3, group.fragments[0]);
				bool inside1 = sampling_is_inside(x + 1, y, Cx1 + I01, Cx2 + I02, Cx3 + I03, group.fragments[1]);
				bool inside2 = sampling_is_inside(x, y + 1, Cx1 + J01, Cx2 + J02, Cx3 + J03, group.fragments[2]);
				bool inside3 = sampling_is_inside(x + 1, y + 1, Cx1 + J01 + I01, Cx2 + J02 + I02, Cx3 + J03 + I03, group.fragments[3]);
				//Note: at least one of them is inside the triangle.
				if (inside0 || inside1 || inside2 || inside3)
				{
					if (!inside0)//Invalid fragment
					{
						group.fragments[0] = VertexData::barycentricLerp(v[0], v[1], v[2], barycentericWeight(x, y));
						group.fragments[0].spos = glm::ivec2(-1);
					}
					else
					{
						glm::vec3 uvw(Cx2, Cx3, Cx1);
						auto coverage = group.fragments[0].coverage;
						auto coverage_depth = group.fragments[0].coverage_depth;
						group.fragments[0] = VertexData::barycentricLerp(v[0], v[1], v[2], uvw * one_div_delta);
						group.fragments[0].spos = glm::ivec2(x, y);
						group.fragments[0].coverage = coverage;
						group.fragments[0].coverage_depth = coverage_depth;
					}

					if (!inside1)//Invalid fragment
					{
						group.fragments[1] = VertexData::barycentricLerp(v[0], v[1], v[2], barycentericWeight(x + 1, y));
						group.fragments[1].spos = glm::ivec2(-1);
					}
					else
					{
						glm::vec3 uvw(Cx2 + I02, Cx3 + I03, Cx1 + I01);
						auto coverage = group.fragments[1].coverage;
						auto coverage_depth = group.fragments[1].coverage_depth;
						group.fragments[1] = VertexData::barycentricLerp(v[0], v[1], v[2], uvw * one_div_delta);
						group.fragments[1].spos = glm::ivec2(x + 1, y);
						group.fragments[1].coverage = coverage;
						group.fragments[1].coverage_depth = coverage_depth;
					}

					if (!inside2)//Invalid fragment
					{
						group.fragments[2] = VertexData::barycentricLerp(v[0], v[1], v[2], barycentericWeight(x, y + 1));
						group.fragments[2].spos = glm::ivec2(-1);
					}
					else
					{
						glm::vec3 uvw(Cx2 + J02, Cx3 + J03, Cx1 + J01);
						auto coverage = group.fragments[2].coverage;
						auto coverage_depth = group.fragments[2].coverage_depth;
						group.fragments[2] = VertexData::barycentricLerp(v[0], v[1], v[2], uvw * one_div_delta);
						group.fragments[2].spos = glm::ivec2(x, y + 1);
						group.fragments[2].coverage = coverage;
						group.fragments[2].coverage_depth = coverage_depth;
					}

					if (!inside3)//Invalid fragment
					{
						group.fragments[3] = VertexData::barycentricLerp(v[0], v[1], v[2], barycentericWeight(x + 1, y + 1));
						group.fragments[3].spos = glm::ivec2(-1);
					}
					else
					{
						glm::vec3 uvw(Cx2 + J02 + I02, Cx3 + J03 + I03, Cx1 + J01 + I01);
						auto coverage = group.fragments[3].coverage;
						auto coverage_depth = group.fragments[3].coverage_depth;
						group.fragments[3] = VertexData::barycentricLerp(v[0], v[1], v[2], uvw * one_div_delta);
						group.fragments[3].spos = glm::ivec2(x + 1, y + 1);
						group.fragments[3].coverage = coverage;
						group.fragments[3].coverage_depth = coverage_depth;
					}

					rasterized_fragments.push_back(group);
				}
				Cx1 += 2 * I01; Cx2 += 2 * I02; Cx3 += 2 * I03;
			}
			Cy1 += 2 * J01;	Cy2 += 2 * J02; Cy3 += 2 * J03;
		}
	}

	int TRShadingPipeline::upload_texture_2D(TRTexture2D::ptr tex)
	{
		if (tex != nullptr)
		{
			m_global_texture_units.push_back(tex);
			return m_global_texture_units.size() - 1;
		}
		return -1;
	}

	TRTexture2D::ptr TRShadingPipeline::getTexture2D(int index)
	{
		if (index < 0 || index >= m_global_texture_units.size())
			return nullptr;
		return m_global_texture_units[index];
	}

	int TRShadingPipeline::addPointLight(glm::vec3 pos, glm::vec3 atten, glm::vec3 color)
	{
		m_point_lights.push_back(TRPointLight(pos, atten, color));
		return m_point_lights.size() - 1;
	}

	TRPointLight &TRShadingPipeline::getPointLight(int index)
	{
		return m_point_lights[index];
	}

	glm::vec4 TRShadingPipeline::texture2D(const unsigned int &id, const glm::vec2 &uv,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy)
	{
		if (id < 0 || id >= m_global_texture_units.size())
			return glm::vec4(0.0f);
		const auto &texture = m_global_texture_units[id];
		if (texture->isGeneratedMipmap())
		{
			//Calculate lod level
			glm::vec2 dfdx = dUVdx * glm::vec2(texture->getWidth(), texture->getHeight());
			glm::vec2 dfdy = dUVdy * glm::vec2(texture->getWidth(), texture->getHeight());
			float L = glm::max(glm::dot(dfdx, dfdx), glm::dot(dfdy, dfdy));
			return texture->sample(uv, glm::max(0.5f * glm::log2(L), 0.0f));
		}
		else
		{
			return texture->sample(uv);
		}
	}

}