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
		result.m_pos = (1.0f - frac) * v0.m_pos + frac * v1.m_pos;
		result.m_nor = (1.0f - frac) * v0.m_nor + frac * v1.m_nor;
		result.m_tex = (1.0f - frac) * v0.m_tex + frac * v1.m_tex;
		result.m_cpos = (1.0f - frac) * v0.m_cpos + frac * v1.m_cpos;
		result.m_spos.x = (1.0f - frac) * v0.m_spos.x + frac * v1.m_spos.x;
		result.m_spos.y = (1.0f - frac) * v0.m_spos.y + frac * v1.m_spos.y;
		result.m_rhw = (1.0f - frac) * v0.m_rhw + frac * v1.m_rhw;
		if (v0.m_needInterpolatedTBN)
		{
			result.m_tbn = (1.0f - frac) * v0.m_tbn + frac * v1.m_tbn;
			result.m_needInterpolatedTBN = true;
		}

		return result;
	}

	TRShadingPipeline::FragmentData TRShadingPipeline::VertexData::barycentricLerp(
		const VertexData &v0, 
		const VertexData &v1, 
		const VertexData &v2,
		const glm::vec3 &w)
	{
		FragmentData result;
		result.m_pos = w.x * v0.m_pos + w.y * v1.m_pos + w.z * v2.m_pos;
		result.m_nor = w.x * v0.m_nor + w.y * v1.m_nor + w.z * v2.m_nor;
		result.m_tex = w.x * v0.m_tex + w.y * v1.m_tex + w.z * v2.m_tex;
		result.m_rhw = w.x * v0.m_rhw + w.y * v1.m_rhw + w.z * v2.m_rhw;
		result.m_spos.x = w.x * v0.m_spos.x + w.y * v1.m_spos.x + w.z * v2.m_spos.x;
		result.m_spos.y = w.x * v0.m_spos.y + w.y * v1.m_spos.y + w.z * v2.m_spos.y;

		if (v0.m_needInterpolatedTBN)
		{
			result.m_tbn = w.x * v0.m_tbn + w.y * v1.m_tbn + w.z * v2.m_tbn;
		}

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
		v.m_rhw = 1.0f / v.m_cpos.w;
		v.m_pos *= v.m_rhw;
		v.m_tex *= v.m_rhw;
		v.m_nor *= v.m_rhw;
	}

	void TRShadingPipeline::FragmentData::aftPrespCorrection(FragmentData &v)
	{
		//Perspective correction: the world space properties should be multipy by w after rasterization
		//https://zhuanlan.zhihu.com/p/144331875
		float w = 1.0f / v.m_rhw;
		v.m_pos *= w;
		v.m_tex *= w;
		v.m_nor *= w;
	}

	//----------------------------------------------TRShadingPipeline----------------------------------------------

	std::vector<TRTexture2D::ptr> TRShadingPipeline::m_globalTextureUnits = {};
	std::vector<TRLight::ptr> TRShadingPipeline::m_lights = {};
	glm::vec3 TRShadingPipeline::m_viewerPos = glm::vec3(0.0f);
	float TRShadingPipeline::m_exposure = 1.0f;

	void TRShadingPipeline::rasterizeFillEdgeFunction(
		const VertexData &v0,
		const VertexData &v1,
		const VertexData &v2,
		const unsigned int &screenWidth,
		const unsigned int &screenHeight,
		std::vector<QuadFragments> &rasterized_fragments)
	{
		//Edge function rasterization algorithm
		//Accelerated Half-Space Triangle Rasterization
		//Refs:Mileff P, Nehéz K, Dudra J. Accelerated half-space triangle rasterization[J].
		//     Acta Polytechnica Hungarica, 2015, 12(7): 217-236.
		//	   http://acta.uni-obuda.hu/Mileff_Nehez_Dudra_63.pdf

		VertexData v[] = { v0, v1, v2 };
		glm::ivec2 boundingMin;
		glm::ivec2 boundingMax;
		boundingMin.x = std::max(std::min(v0.m_spos.x, std::min(v1.m_spos.x, v2.m_spos.x)), 0);
		boundingMin.y = std::max(std::min(v0.m_spos.y, std::min(v1.m_spos.y, v2.m_spos.y)), 0);
		boundingMax.x = std::min(std::max(v0.m_spos.x, std::max(v1.m_spos.x, v2.m_spos.x)), (int)screenWidth - 1);
		boundingMax.y = std::min(std::max(v0.m_spos.y, std::max(v1.m_spos.y, v2.m_spos.y)), (int)screenHeight - 1);

		//Adjust the order
		{
			int orient = 0;
			auto e1 = v1.m_spos - v0.m_spos;
			auto e2 = v2.m_spos - v0.m_spos;
			orient = e1.x * e2.y - e1.y * e2.x;
			if (orient > 0)
			{
				std::swap(v[1], v[2]);
			}
		}

		const glm::ivec2 &A = v[0].m_spos;
		const glm::ivec2 &B = v[1].m_spos;
		const glm::ivec2 &C = v[2].m_spos;

		const int I01 = A.y - B.y, I02 = B.y - C.y, I03 = C.y - A.y;
		const int J01 = B.x - A.x, J02 = C.x - B.x, J03 = A.x - C.x;
		const int K01 = A.x * B.y - A.y * B.x;
		const int K02 = B.x * C.y - B.y * C.x;
		const int K03 = C.x * A.y - C.y * A.x;

		int F01 = I01 * boundingMin.x + J01 * boundingMin.y + K01;
		int F02 = I02 * boundingMin.x + J02 * boundingMin.y + K02;
		int F03 = I03 * boundingMin.x + J03 * boundingMin.y + K03;

		//Degenerated to a line or a point
		if (F01 + F02 + F03 == 0)
			return;

		rasterized_fragments.reserve((boundingMax.y - boundingMin.y) * (boundingMax.x - boundingMin.x));

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
			s[0] = glm::vec3(v[2].m_spos.x - v[0].m_spos.x, v[1].m_spos.x - v[0].m_spos.x, v[0].m_spos.x - x);
			s[1] = glm::vec3(v[2].m_spos.y - v[0].m_spos.y, v[1].m_spos.y - v[0].m_spos.y, v[0].m_spos.y - y);
			auto uf = glm::cross(s[0], s[1]);
			return glm::vec3(1.f - (uf.x + uf.y) / uf.z, uf.y / uf.z, uf.x / uf.z);
		};

		auto sampling_is_inside = [&](const int &x, const int &y, const int &Cx1, const int &Cx2, 
			const int &Cx3, FragmentData &p) -> bool
		{
			//Invalid fragment
			if (x > boundingMax.x || y > boundingMax.y)
			{
				p.m_spos = glm::ivec2(-1);
				return false;
			}
			bool atLeastOneInside = false;
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
					atLeastOneInside = true;
					p.m_coverage[s] = 1;//Covered
					//Note: each sampling point should have its own depth
					glm::vec3 uvw = glm::vec3(E2, E3, E1) * one_div_delta;
					p.m_coverageDepth[s] = VertexData::barycentricLerp(v[0].m_rhw, v[1].m_rhw, v[2].m_rhw, uvw);
				}
			}

			if (!atLeastOneInside)
			{
				p.m_spos = glm::ivec2(-1);
			}
			return atLeastOneInside;
		};

		for(int y = boundingMin.y;y <= boundingMax.y;y += 2)
		{
			int Cx1 = Cy1, Cx2 = Cy2, Cx3 = Cy3;
#pragma unroll 4
			for (int x = boundingMin.x; x <= boundingMax.x; x += 2)
			{
				//2x2 fragments block
				QuadFragments group;
				bool inside0 = sampling_is_inside(x, y, Cx1, Cx2, Cx3, group.m_fragments[0]);
				bool inside1 = sampling_is_inside(x + 1, y, Cx1 + I01, Cx2 + I02, Cx3 + I03, group.m_fragments[1]);
				bool inside2 = sampling_is_inside(x, y + 1, Cx1 + J01, Cx2 + J02, Cx3 + J03, group.m_fragments[2]);
				bool inside3 = sampling_is_inside(x + 1, y + 1, Cx1 + J01 + I01, Cx2 + J02 + I02, Cx3 + J03 + I03, group.m_fragments[3]);
				//Note: at least one of them is inside the triangle.
				if (inside0 || inside1 || inside2 || inside3)
				{
					if (!inside0)//Invalid fragment
					{
						group.m_fragments[0] = VertexData::barycentricLerp(v[0], v[1], v[2], barycentericWeight(x, y));
						group.m_fragments[0].m_spos = glm::ivec2(-1);
					}
					else
					{
						glm::vec3 uvw(Cx2, Cx3, Cx1);
						auto coverage = group.m_fragments[0].m_coverage;
						auto coverage_depth = group.m_fragments[0].m_coverageDepth;
						group.m_fragments[0] = VertexData::barycentricLerp(v[0], v[1], v[2], uvw * one_div_delta);
						group.m_fragments[0].m_spos = glm::ivec2(x, y);
						group.m_fragments[0].m_coverage = coverage;
						group.m_fragments[0].m_coverageDepth = coverage_depth;
					}

					if (!inside1)//Invalid fragment
					{
						group.m_fragments[1] = VertexData::barycentricLerp(v[0], v[1], v[2], barycentericWeight(x + 1, y));
						group.m_fragments[1].m_spos = glm::ivec2(-1);
					}
					else
					{
						glm::vec3 uvw(Cx2 + I02, Cx3 + I03, Cx1 + I01);
						auto coverage = group.m_fragments[1].m_coverage;
						auto coverage_depth = group.m_fragments[1].m_coverageDepth;
						group.m_fragments[1] = VertexData::barycentricLerp(v[0], v[1], v[2], uvw * one_div_delta);
						group.m_fragments[1].m_spos = glm::ivec2(x + 1, y);
						group.m_fragments[1].m_coverage = coverage;
						group.m_fragments[1].m_coverageDepth = coverage_depth;
					}

					if (!inside2)//Invalid fragment
					{
						group.m_fragments[2] = VertexData::barycentricLerp(v[0], v[1], v[2], barycentericWeight(x, y + 1));
						group.m_fragments[2].m_spos = glm::ivec2(-1);
					}
					else
					{
						glm::vec3 uvw(Cx2 + J02, Cx3 + J03, Cx1 + J01);
						auto coverage = group.m_fragments[2].m_coverage;
						auto coverage_depth = group.m_fragments[2].m_coverageDepth;
						group.m_fragments[2] = VertexData::barycentricLerp(v[0], v[1], v[2], uvw * one_div_delta);
						group.m_fragments[2].m_spos = glm::ivec2(x, y + 1);
						group.m_fragments[2].m_coverage = coverage;
						group.m_fragments[2].m_coverageDepth = coverage_depth;
					}

					if (!inside3)//Invalid fragment
					{
						group.m_fragments[3] = VertexData::barycentricLerp(v[0], v[1], v[2], barycentericWeight(x + 1, y + 1));
						group.m_fragments[3].m_spos = glm::ivec2(-1);
					}
					else
					{
						glm::vec3 uvw(Cx2 + J02 + I02, Cx3 + J03 + I03, Cx1 + J01 + I01);
						auto coverage = group.m_fragments[3].m_coverage;
						auto coverage_depth = group.m_fragments[3].m_coverageDepth;
						group.m_fragments[3] = VertexData::barycentricLerp(v[0], v[1], v[2], uvw * one_div_delta);
						group.m_fragments[3].m_spos = glm::ivec2(x + 1, y + 1);
						group.m_fragments[3].m_coverage = coverage;
						group.m_fragments[3].m_coverageDepth = coverage_depth;
					}

					rasterized_fragments.push_back(group);
				}
				Cx1 += 2 * I01; Cx2 += 2 * I02; Cx3 += 2 * I03;
			}
			Cy1 += 2 * J01;	Cy2 += 2 * J02; Cy3 += 2 * J03;
		}
	}

	int TRShadingPipeline::uploadTexture2D(TRTexture2D::ptr tex)
	{
		if (tex != nullptr)
		{
			m_globalTextureUnits.push_back(tex);
			return m_globalTextureUnits.size() - 1;
		}
		return -1;
	}

	TRTexture2D::ptr TRShadingPipeline::getTexture2D(int index)
	{
		if (index < 0 || index >= m_globalTextureUnits.size())
			return nullptr;
		return m_globalTextureUnits[index];
	}

	int TRShadingPipeline::addLight(TRLight::ptr lightSource)
	{
		m_lights.push_back(lightSource);
		return m_lights.size() - 1;
	}

	TRLight::ptr TRShadingPipeline::getLight(int index)
	{
		return m_lights[index];
	}

	glm::vec4 TRShadingPipeline::texture2D(const unsigned int &id, const glm::vec2 &uv,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy)
	{
		if (id < 0 || id >= m_globalTextureUnits.size())
			return glm::vec4(0.0f);
		const auto &texture = m_globalTextureUnits[id];
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