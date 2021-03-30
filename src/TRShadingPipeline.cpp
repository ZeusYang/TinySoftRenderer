#include "TRShadingPipeline.h"

#include <algorithm>
#include <iostream>

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

		result.TBN = v0.TBN;

		return result;
	}

	TRShadingPipeline::VertexData TRShadingPipeline::VertexData::barycentricLerp(
		const VertexData &v0, 
		const VertexData &v1, 
		const VertexData &v2,
		const glm::vec3 &w)
	{
		VertexData result;
		result.pos = w.x * v0.pos + w.y * v1.pos + w.z * v2.pos;
		result.col = w.x * v0.col + w.y * v1.col + w.z * v2.col;
		result.nor = w.x * v0.nor + w.y * v1.nor + w.z * v2.nor;
		result.tex = w.x * v0.tex + w.y * v1.tex + w.z * v2.tex;
		result.cpos = w.x * v0.cpos + w.y * v1.cpos + w.z * v2.cpos;
		result.spos.x = w.x * v0.spos.x + w.y * v1.spos.x + w.z * v2.spos.x;
		result.spos.y = w.x * v0.spos.y + w.y * v1.spos.y + w.z * v2.spos.y;

		result.TBN = v0.TBN;

		return result;
	}

	void TRShadingPipeline::VertexData::prePerspCorrection(VertexData &v)
	{
		//Perspective correction: the world space properties should be multipy by 1/w before rasterization
		//https://zhuanlan.zhihu.com/p/144331875
		//We use pos.w to store 1/w
		float one_div_w =  1.0f / v.cpos.w;
		v.pos = glm::vec4(v.pos.x * one_div_w, v.pos.y * one_div_w, v.pos.z * one_div_w, one_div_w);
		v.tex = v.tex * one_div_w;
		v.nor = v.nor * one_div_w;
		v.col = v.col * one_div_w;
	}

	void TRShadingPipeline::VertexData::aftPrespCorrection(VertexData &v)
	{
		//Perspective correction: the world space properties should be multipy by w after rasterization
		//https://zhuanlan.zhihu.com/p/144331875
		//We use pos.w to store 1/w
		float w = 1.0f / v.pos.w;
		//v.cpos.z *= w;
		v.pos = glm::vec4(v.pos.x * w, v.pos.y * w, v.pos.z * w, v.pos.w);
		v.tex = v.tex * w;
		v.nor = v.nor * w;
		v.col = v.col * w;
	}

	//----------------------------------------------TRShadingPipeline----------------------------------------------

	std::vector<TRTexture2D::ptr> TRShadingPipeline::m_global_texture_units = {};
	std::vector<TRPointLight> TRShadingPipeline::m_point_lights = {};
	glm::vec3 TRShadingPipeline::m_viewer_pos = glm::vec3(0.0f);

	void TRShadingPipeline::rasterize_wire(
		const VertexData &v0,
		const VertexData &v1,
		const VertexData &v2,
		const unsigned int &screen_width,
		const unsigned int &screene_height,
		std::vector<VertexData> &rasterized_points)
	{
		//Draw each line step by step
		rasterize_wire_aux(v0, v1, screen_width, screene_height, rasterized_points);
		rasterize_wire_aux(v1, v2, screen_width, screene_height, rasterized_points);
		rasterize_wire_aux(v0, v2, screen_width, screene_height, rasterized_points);
	}

	void TRShadingPipeline::rasterize_fill_edge_function(
		const VertexData &v0,
		const VertexData &v1,
		const VertexData &v2,
		const unsigned int &screen_width,
		const unsigned int &screene_height,
		std::vector<VertexData> &rasterized_points)
	{
		//Edge function rasterization algorithm
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

		//Accelerated Half-Space Triangle Rasterization
		//Refs:Mileff P, Neh¨¦z K, Dudra J. Accelerated half-space triangle rasterization[J].
		//     Acta Polytechnica Hungarica, 2015, 12(7): 217-236. http://acta.uni-obuda.hu/Mileff_Nehez_Dudra_63.pdf

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

		const float one_div_delta = 1.0f / (F01 + F02 + F03);

		//Top left fill rule
		int E1_t = (((B.y > A.y) || (A.y == B.y && A.x > B.x)) ? 0 : -1);
		int E2_t = (((C.y > B.y) || (B.y == C.y && B.x > C.x)) ? 0 : -1);
		int E3_t = (((A.y > C.y) || (C.y == A.y && C.x > A.x)) ? 0 : -1);

		//Rasterize a block
		auto rasterize_block_func = [&](const glm::ivec2 &bound_min, const glm::ivec2 &bound_max, int Cy1, int Cy2, int Cy3)
		{
			for (int y = bound_min.y; y <= bound_max.y; ++y)
			{
				int Cx1 = Cy1, Cx2 = Cy2, Cx3 = Cy3;
				for (int x = bound_min.x; x <= bound_max.x; ++x)
				{
					int E1 = Cx1 + E1_t, E2 = Cx2 + E2_t, E3 = Cx3 + E3_t;
					//Counter-clockwise winding order
					if (E1 <= 0 && E2 <= 0 && E3 <= 0)
					{
						glm::vec3 uvw(Cx2 * one_div_delta, Cx3 * one_div_delta, Cx1 * one_div_delta);
						auto rasterized_point = TRShadingPipeline::VertexData::barycentricLerp(v[0], v[1], v[2], uvw);
						rasterized_point.spos = glm::ivec2(x, y);
						rasterized_points.push_back(rasterized_point);
					}
					Cx1 += I01; Cx2 += I02; Cx3 += I03;
				}
				Cy1 += J01; Cy2 += J02; Cy3 += J03;
			}
		};

		//Adaptive block-based strategy
		float orient = (float)(bounding_max.x - bounding_min.x) / (float)(bounding_max.y - bounding_min.y);
		if (orient > 0.4 && orient < 1.6)
		{
			//Block level traversal
			constexpr int blockSize = 8;
			//Round to block grid.
			bounding_max.x = bounding_max.x & ~(blockSize - 1);
			bounding_min.x = bounding_min.x & ~(blockSize - 1);
			bounding_max.y = bounding_max.y & ~(blockSize - 1);
			bounding_min.y = bounding_min.y & ~(blockSize - 1);

			int stepX = (bounding_max.x - bounding_min.x) / blockSize + 1;
			int stepY = (bounding_max.y - bounding_min.y) / blockSize + 1;
			for (int i = 0; i < stepX * stepY; ++i)
			{
				int sx = i % stepX, sy = i / stepX;
				//Test four corners of the block
				/*********************
				 *   c2---c3
				 *   |    |
				 *   c0---c1
				 ********************/
				glm::ivec2 c0(bounding_min.x + sx * blockSize, bounding_min.y + sy * blockSize);
				glm::ivec2 c1(c0.x + blockSize, c0.y);
				glm::ivec2 c2(c0.x, c0.y + blockSize);
				glm::ivec2 c3(c1.x, c2.y);
				//Edge function
				auto edge_func = [&](const int &x, const int &y) -> int
				{
					int f1 = I01 * x + J01 * y + K01;
					int f2 = I02 * x + J02 * y + K02;
					int f3 = I03 * x + J03 * y + K03;
					return f1 + f2 + f3;
				};
				
				int c0_f = edge_func(c0.x, c0.y), c1_f = edge_func(c1.x, c1.y);
				int c2_f = edge_func(c2.x, c2.y), c3_f = edge_func(c3.x, c3.y);
				
				//Totally outside
				if (c0_f > 0 && c1_f > 0 && c2_f > 0 && c3_f > 0)
				{
					continue;
				}
				//Totally inside or partially inside
				else
				{
					int Cy1 = I01 * c0.x + J01 * c0.y + K01;
					int Cy2 = I02 * c0.x + J02 * c0.y + K02;
					int Cy3 = I03 * c0.x + J03 * c0.y + K03;
					rasterize_block_func(c0, c3, Cy1, Cy2, Cy3);
				}
			}
		}
		else
		{
			//Pixel level traversal
			int Cy1 = F01, Cy2 = F02, Cy3 = F03;
			rasterize_block_func(bounding_min, bounding_max, Cy1, Cy2, Cy3);
		}
	}

	void TRShadingPipeline::rasterize_wire_aux(
		const VertexData &from,
		const VertexData &to,
		const unsigned int &screen_width,
		const unsigned int &screen_height,
		std::vector<VertexData> &rasterized_points)
	{
		//Bresenham line rasterization

		int dx = to.spos.x - from.spos.x;
		int dy = to.spos.y - from.spos.y;
		int stepX = 1, stepY = 1;

		// judge the sign
		if (dx < 0)
		{
			stepX = -1;
			dx = -dx;
		}
		if (dy < 0)
		{
			stepY = -1;
			dy = -dy;
		}

		int d2x = 2 * dx, d2y = 2 * dy;
		int d2y_minus_d2x = d2y - d2x;
		int sx = from.spos.x;
		int sy = from.spos.y;

		// slope < 1.
		if (dy <= dx)
		{
			int flag = d2y - dx;
			for (int i = 0; i <= dx; ++i)
			{
				auto mid = VertexData::lerp(from, to, static_cast<float>(i) / dx);
				mid.spos = glm::ivec2(sx, sy);
				if (mid.spos.x >= 0 && mid.spos.x <= screen_width && mid.spos.y >= 0 && mid.spos.y <= screen_height)
				{
					rasterized_points.push_back(mid);
				}
				sx += stepX;
				if (flag <= 0)
				{
					flag += d2y;
				}
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
			for (int i = 0; i <= dy; ++i)
			{
				auto mid = VertexData::lerp(from, to, static_cast<float>(i) / dy);
				mid.spos = glm::ivec2(sx, sy);
				if (mid.spos.x >= 0 && mid.spos.x < screen_width && mid.spos.y >= 0 && mid.spos.y < screen_height)
				{
					rasterized_points.push_back(mid);
				}
				sy += stepY;
				if (flag <= 0)
				{
					flag += d2x;
				}
				else
				{
					sx += stepX;
					flag -= d2y_minus_d2x;
				}
			}
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

	glm::vec4 TRShadingPipeline::texture2D(const unsigned int &id, const glm::vec2 &uv)
	{
		if (id < 0 || id >= m_global_texture_units.size())
			return glm::vec4(0.0f);
		return m_global_texture_units[id]->sample(uv);
	}

}