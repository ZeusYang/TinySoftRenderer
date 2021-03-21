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

	void TRShadingPipeline::rasterize_fill_edge_walking(
		const VertexData &v0,
		const VertexData &v1,
		const VertexData &v2,
		const unsigned int &screen_width,
		const unsigned int &screene_height,
		std::vector<VertexData> &rasterized_points)
	{
		//Reorder the vertices of the triangle to make sure that p0.y < p1.y < p2.y
		VertexData points[3] = { v0, v1, v2 };
		{
			std::sort(points, points + 3, [](const VertexData &v1, const VertexData &v2) -> bool
			{
				return v1.spos.y < v2.spos.y;
			});
		}
		
		//Calculate the spliting-point to split the triangle into a top triangle and a bottom triangle
		auto splitPoint = points[1];
		auto pp1 = points[1], pp2 = points[1];
		if (points[2].spos.y - points[0].spos.y != 0)
		{
			splitPoint = TRShadingPipeline::VertexData::lerp(points[0], points[2],
				(float)(points[1].spos.y - points[0].spos.y) / (float)(points[2].spos.y - points[0].spos.y));
			splitPoint.spos.y = points[1].spos.y;
			pp2 = splitPoint;
			if (pp1.spos.x > pp2.spos.x)
			{
				std::swap(pp1, pp2);
			}
		}
		else
		{
			//Three points are in the same line
			std::vector<VertexData> tmp = { points[0], points[1], points[2] };
			std::sort(tmp.begin(), tmp.end(), [](const VertexData &v1, const VertexData &v2) -> bool
			{
				return v1.spos.x < v2.spos.x;
			});
			pp1 = tmp[0];
			pp2 = tmp[2];
		}

		//Scaning the top triangle
		{
			rasterize_fill_edge_walking_aux(points[0], pp1, pp2, screen_width, screene_height, rasterized_points);
		}

		//Scaning the bottom triangle
		{
			rasterize_fill_edge_walking_aux(pp1, pp2, points[2], screen_width, screene_height, rasterized_points);
		}

		//Scanning the split line
		{
			if (pp1.spos.y < 0 || pp1.spos.y >= screene_height)
				return;

			if (pp1.spos.x == pp2.spos.x)
			{
				rasterized_points.push_back(pp1);
				return;
			}
			
			//Scan line per row
			float denom_lerp_x = 1.0f / (pp2.spos.x - pp1.spos.x);
			for (int x = pp1.spos.x; x <= pp2.spos.x; ++x)
			{
				float frac_in_x = (x - pp1.spos.x) * denom_lerp_x;
				auto rasterized_point = TRShadingPipeline::VertexData::lerp(pp1, pp2, frac_in_x);
				rasterized_point.spos.x = x;
				rasterized_point.spos.y = pp1.spos.y;

				//Handle the outside situation
				if (rasterized_point.spos.x < 0)
					continue;
				if (rasterized_point.spos.x >= screen_width)
					break;

				rasterized_points.push_back(rasterized_point);
			}
		}
	}

	void TRShadingPipeline::rasterize_fill_edge_equations(
		const VertexData &v0,
		const VertexData &v1,
		const VertexData &v2,
		const unsigned int &screen_width,
		const unsigned int &screene_height,
		std::vector<VertexData> &rasterized_points)
	{
		VertexData v[] = { v0, v1, v2 };
		//Edge-equations rasterization algorithm
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
		int E1_t = (((B.y > A.y) || (A.y == B.y && A.x > B.x)) ? 0 : 0);
		int E2_t = (((C.y > B.y) || (B.y == C.y && B.x > C.x)) ? 0 : 0);
		int E3_t = (((A.y > C.y) || (C.y == A.y && C.x > A.x)) ? 0 : 0);

		int Cy1 = F01, Cy2 = F02, Cy3 = F03;
		for (int y = bounding_min.y; y <= bounding_max.y; ++y)
		{
			int Cx1 = Cy1, Cx2 = Cy2, Cx3 = Cy3;
			for (int x = bounding_min.x; x <= bounding_max.x; ++x)
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

	void TRShadingPipeline::rasterize_fill_edge_walking_aux(
		const VertexData &v0,
		const VertexData &v1,
		const VertexData &v2,
		const unsigned int &screen_width,
		const unsigned int &screene_height,
		std::vector<VertexData> &rasterized_points)
	{
		// v0.spos.y <= v1.spos.y <= v2.spos.y

		if (v0.spos.y == v2.spos.y)
			return;

		/**************************
		 * Top flat triangle
		 *			v0
		 *         /  \
		 *        /    \
		 *       /      \
		 *      v1-------v2
		 * Note: not rasterize line v1-v2 
		 *          or
		 * Bottom flat triangle
		 *	    v0-------v1
		 *        \     /
		 *         \   /
		 *          \ /  
		 *           v2
		 * Note: not rasterize line v0-v1
		 **************************/

		bool is_top_flat_tri = (v0.spos.y == v1.spos.y) ? false : true;
		float denom_lerp_y = 1.0f / (v2.spos.y - v0.spos.y);
		auto lEndPoint = v0, rEndPoint = is_top_flat_tri ? v0 : v1;

		int y_beg = is_top_flat_tri ? v0.spos.y : v0.spos.y + 1;
		int y_end = is_top_flat_tri ? v2.spos.y - 1 : v2.spos.y;
		
		float invslope1 = 0.0f, invslope2 = 0.0f;

		if (!is_top_flat_tri)
		{
			float frac_in_y = denom_lerp_y * (y_beg - v0.spos.y);
			lEndPoint = TRShadingPipeline::VertexData::lerp(v0, v2, frac_in_y);
			rEndPoint = TRShadingPipeline::VertexData::lerp(v1, v2, frac_in_y);
			lEndPoint.spos.y = rEndPoint.spos.y = y_beg;

			invslope1 = (float)(v2.spos.x - v0.spos.x) / (float)(v2.spos.y - v0.spos.y);
			invslope2 = (float)(v2.spos.x - v1.spos.x) / (float)(v2.spos.y - v1.spos.y);
		}
		else
		{
			invslope1 = (float)(v1.spos.x - v0.spos.x) / (float)(v1.spos.y - v0.spos.y);
			invslope2 = (float)(v2.spos.x - v0.spos.x) / (float)(v2.spos.y - v0.spos.y);
		}

		float curx1 = lEndPoint.spos.x, curx2 = rEndPoint.spos.x;
		for (int y = y_beg; y <= y_end; )
		{
			if (lEndPoint.spos.y < 0 || lEndPoint.spos.y >= screene_height)
			{
				++y;
				continue;
			}

			if (lEndPoint.spos.x == rEndPoint.spos.x)
			{
				rasterized_points.push_back(lEndPoint);
			}
			else
			{
				//lEndPoint.spos.x = (int)(curx1+0.5f);
				rEndPoint.spos.x = (int)(curx2+0.5f);
				float denom_lerp_x = 1.0f / (rEndPoint.spos.x - lEndPoint.spos.x);
				for (int x = lEndPoint.spos.x; x <= rEndPoint.spos.x; ++x)
				{
					float frac_in_x = (x - lEndPoint.spos.x) * denom_lerp_x;
					auto rasterized_point = TRShadingPipeline::VertexData::lerp(lEndPoint, rEndPoint, frac_in_x);
					rasterized_point.spos.x = x;
					rasterized_point.spos.y = lEndPoint.spos.y;
					//Handle the outside situation
					if (rasterized_point.spos.x < 0)
						continue;
					if (rasterized_point.spos.x >= screen_width)
						break;
					
					rasterized_points.push_back(rasterized_point);
				}
			}

			++y;
			float frac_in_y = denom_lerp_y * (y - v0.spos.y);
			if (is_top_flat_tri)
			{
				lEndPoint = TRShadingPipeline::VertexData::lerp(v0, v1, frac_in_y);
				rEndPoint = TRShadingPipeline::VertexData::lerp(v0, v2, frac_in_y);
			}
			else
			{
				lEndPoint = TRShadingPipeline::VertexData::lerp(v0, v2, frac_in_y);
				rEndPoint = TRShadingPipeline::VertexData::lerp(v1, v2, frac_in_y);
			}
			lEndPoint.spos.y = rEndPoint.spos.y = y;

			curx1 += invslope1;
			curx2 += invslope2;
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


	//----------------------------------------------TRDefaultShadingPipeline----------------------------------------------

	void TRDefaultShadingPipeline::vertexShader(VertexData &vertex)
	{
		//Local space -> World space -> Camera space -> Project space
		vertex.pos = m_model_matrix * glm::vec4(vertex.pos.x, vertex.pos.y, vertex.pos.z, 1.0f);
		vertex.nor = glm::normalize(m_inv_trans_model_matrix * vertex.nor);
		vertex.cpos = m_view_project_matrix * vertex.pos;

		glm::vec3 T = glm::normalize(m_inv_trans_model_matrix * m_tangent);
		glm::vec3 B = glm::normalize(m_inv_trans_model_matrix * m_bitangent);
		vertex.TBN = glm::mat3(T, B, vertex.nor);
	}

	void TRDefaultShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor)
	{
		//Just return the color.
		fragColor = glm::vec4(data.tex, 0.0, 1.0f);
	}

	//----------------------------------------------TRNoTransformNoShadingPipeline----------------------------------------------

	void TRDoNothingShadingPipeline::vertexShader(VertexData &vertex)
	{
		//do nothing at all
		vertex.cpos = vertex.pos;
	}

	void TRDoNothingShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor)
	{
		//Just return the color.
		fragColor = glm::vec4(data.tex, 0.0, 1.0f);
	}

	//----------------------------------------------TRPhongShadingPipeline----------------------------------------------

	void TRPhongShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor)
	{
		glm::vec3 amb_color, dif_color, spe_color, glow_color;
		glm::vec3 fragPos = glm::vec3(data.pos);
		glm::vec3 normal = glm::normalize(data.nor);
		//glm::vec3 normal = fetchFragmentNormal(data, data.tex);

		fragColor = glm::vec4(0.0f);

		//Fetch the corresponding color 
		fetchFragmentColor(amb_color, dif_color, spe_color, data.tex);

		glow_color = (m_glow_tex_id != -1) ? glm::vec3(TRShadingPipeline::getTexture2D(m_glow_tex_id)->sample(data.tex)) : m_ke;

		if (!m_lighting_enable)
		{
			fragColor = glm::vec4(glow_color, 1.0f);
			return;
		}

		//Calculate the lighting
		glm::vec3 viewDir = glm::normalize(m_viewer_pos - fragPos);
		for (size_t i = 0; i < m_point_lights.size(); ++i)
		{
			const auto &light = m_point_lights[i];
			glm::vec3 lightDir = glm::normalize(fragPos - light.lightPos);
			
			//Ambient
			glm::vec3 ambient = light.lightColor * amb_color;

			//Diffuse
			float diffCof = std::max(glm::dot(normal, lightDir), 0.0f);
			glm::vec3 diffuse = light.lightColor * dif_color * diffCof * m_kd;

			//Specular
			glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
			float spec = std::pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), m_shininess);
			glm::vec3 specular = light.lightColor * spec * spe_color;

			float distance = glm::length(light.lightPos - fragPos);
			float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance +
				light.attenuation.z * (distance * distance));

			fragColor.x += (ambient.x + diffuse.x + specular.x) * attenuation;
			fragColor.y += (ambient.y + diffuse.y + specular.y) * attenuation;
			fragColor.z += (ambient.z + diffuse.z + specular.z) * attenuation;
		}
		fragColor = glm::vec4(fragColor.x + glow_color.x, fragColor.y + glow_color.y, fragColor.z + glow_color.z, 1.0f);
	}

	glm::vec3 TRPhongShadingPipeline::fetchFragmentNormal(const VertexData &data, const glm::vec2 &uv) const
	{
		//If there is no normal map
		if (m_normal_tex_id == -1)
		{
			return glm::normalize(data.nor);
		}

		glm::vec3 normal = glm::normalize(glm::vec3(TRShadingPipeline::getTexture2D(m_normal_tex_id)->sample(uv)) * 2.0f - glm::vec3(1.0f));
		normal = glm::normalize(data.TBN * normal);
		return normal;
	}

	void TRPhongShadingPipeline::fetchFragmentColor(glm::vec3 &amb, glm::vec3 &diff, glm::vec3 &spe, const glm::vec2 &uv) const
	{
		if (m_diffuse_tex_id != -1)
		{
			diff = glm::vec3(TRShadingPipeline::getTexture2D(m_diffuse_tex_id)->sample(uv));
			amb = diff;
		}
		else
		{
			diff = m_ka;
			diff = m_kd;
		}
		
		spe = (m_specular_tex_id != -1) ?glm::vec3(TRShadingPipeline::getTexture2D(m_specular_tex_id)->sample(uv)) : m_ks;
	}

}