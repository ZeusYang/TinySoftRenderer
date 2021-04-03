#include "TRShaderProgram.h"

namespace TinyRenderer
{
	//----------------------------------------------TR3DShadingPipeline----------------------------------------------

	void TR3DShadingPipeline::vertexShader(VertexData &vertex) const
	{
		//Local space -> World space -> Camera space -> Project space
		vertex.pos = m_model_matrix * glm::vec4(vertex.pos.x, vertex.pos.y, vertex.pos.z, 1.0f);
		vertex.nor = glm::normalize(m_inv_trans_model_matrix * vertex.nor);
		vertex.cpos = m_view_project_matrix * vertex.pos;
	}

	void TR3DShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		//Just return the color.
		fragColor = glm::vec4(data.tex, 0.0, 1.0f);
	}

	//----------------------------------------------TRDoNothingShadingPipeline----------------------------------------------

	void TRDoNothingShadingPipeline::vertexShader(VertexData &vertex) const
	{
		//do nothing at all
		vertex.cpos = vertex.pos;
	}

	void TRDoNothingShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		//Just return the color.
		fragColor = glm::vec4(data.tex, 0.0, 1.0f);
	}

	//----------------------------------------------TRTextureShadingPipeline----------------------------------------------

	void TRTextureShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		//Default color
		fragColor = glm::vec4(m_ke, 1.0f);

		if (m_diffuse_tex_id != -1)
		{
			fragColor = texture2D(m_diffuse_tex_id, data.tex, dUVdx, dUVdy);
		}
	}

	//----------------------------------------------TRLODVisualizePipeline----------------------------------------------

	void TRLODVisualizePipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		//Visualization of LOD
		static const glm::vec3 mipmapColors[] =
		{
			glm::vec3(1, 0, 0),
			glm::vec3(0, 0, 1),
			glm::vec3(1, 0.5f, 0),
			glm::vec3(1, 0, 0.5f),
			glm::vec3(0, 0.5f, 0.5f),
			glm::vec3(0, 0.25f, 0.5f),
			glm::vec3(0.25f, 0.5f, 0),
			glm::vec3(0.5f, 0, 1),
			glm::vec3(1, 0.25f, 0.5f),
			glm::vec3(0.5f, 0.5f, 0.5f),
			glm::vec3(0.25f, 0.25f, 0.25f),
			glm::vec3(0.125f, 0.125f, 0.125f)
		};
		auto tex = TRShadingPipeline::getTexture2D(m_diffuse_tex_id);
		int w = 1000, h = 100;
		if (tex != nullptr)
		{
			w = tex->getWidth();
			h = tex->getHeight();
		}
		glm::vec2 dfdx = dUVdx * glm::vec2(w, h);
		glm::vec2 dfdy = dUVdy * glm::vec2(w, h);
		float L = glm::max(glm::dot(dfdx, dfdx), glm::dot(dfdy, dfdy));
		float LOD = 0.5f * glm::log2(L);
		fragColor = glm::vec4(mipmapColors[glm::max(int(LOD + 0.5), 0)], 1.0f);
		return;
	}

	//----------------------------------------------TRPhongShadingPipeline----------------------------------------------

	void TRPhongShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		fragColor = glm::vec4(0.0f);

		//Fetch the corresponding color 
		glm::vec3 amb_color, dif_color, spe_color, glow_color;
		amb_color = dif_color = (m_diffuse_tex_id != -1) ? glm::vec3(texture2D(m_diffuse_tex_id, data.tex, dUVdx, dUVdy)) : m_kd;
		spe_color = (m_specular_tex_id != -1) ? glm::vec3(texture2D(m_specular_tex_id, data.tex, dUVdx, dUVdy)) : m_ks;
		glow_color = (m_glow_tex_id != -1) ? glm::vec3(texture2D(m_glow_tex_id, data.tex, dUVdx, dUVdy)) : m_ke;

		//No lighting
		if (!m_lighting_enable)
		{
			fragColor = glm::vec4(glow_color, 1.0f);
			return;
		}

		//Calculate the lighting
		glm::vec3 fragPos = glm::vec3(data.pos);
		glm::vec3 normal = glm::normalize(data.nor);
		glm::vec3 viewDir = glm::normalize(m_viewer_pos - fragPos);
		for (size_t i = 0; i < m_point_lights.size(); ++i)
		{
			const auto &light = m_point_lights[i];
			glm::vec3 lightDir = glm::normalize(light.lightPos - fragPos);

			glm::vec3 ambient, diffuse, specular;
			float attenuation = 1.0f;

			{
				//Ambient
				ambient = light.lightColor * amb_color;

				//Diffuse
				float diffCof = glm::max(glm::dot(normal, lightDir), 0.0f);
				diffuse = light.lightColor * dif_color * diffCof * m_kd;

				//Phong Specular
				glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
				float spec = std::pow(glm::max(glm::dot(viewDir, reflectDir), 0.0f), m_shininess);
				specular = light.lightColor * spec * spe_color;

				float distance = glm::length(light.lightPos - fragPos);
				attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance +
					light.attenuation.z * (distance * distance));
			}

			fragColor.x += (ambient.x + diffuse.x + specular.x) * attenuation;
			fragColor.y += (ambient.y + diffuse.y + specular.y) * attenuation;
			fragColor.z += (ambient.z + diffuse.z + specular.z) * attenuation;
		}

		fragColor = glm::vec4(fragColor.x + glow_color.x, fragColor.y + glow_color.y, fragColor.z + glow_color.z, 1.0f);

		//Tone mapping: HDR -> LDR
		//Refs: https://learnopengl.com/Advanced-Lighting/HDR
		{
			glm::vec3 hdrColor(fragColor);
			fragColor.x = 1.0f - glm::exp(-hdrColor.x * 2.0f);
			fragColor.y = 1.0f - glm::exp(-hdrColor.y * 2.0f);
			fragColor.z = 1.0f - glm::exp(-hdrColor.z * 2.0f);
		}
	}

	//----------------------------------------------TRBlinPhongShadingPipeline----------------------------------------------

	void TRBlinnPhongShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		fragColor = glm::vec4(0.0f);

		//Fetch the corresponding color 
		glm::vec3 amb_color, dif_color, spe_color, glow_color;
		amb_color = dif_color = (m_diffuse_tex_id != -1) ? glm::vec3(texture2D(m_diffuse_tex_id, data.tex, dUVdx, dUVdy)) : m_kd;
		spe_color = (m_specular_tex_id != -1) ? glm::vec3(texture2D(m_specular_tex_id, data.tex, dUVdx, dUVdy)) : m_ks;
		glow_color = (m_glow_tex_id != -1) ? glm::vec3(texture2D(m_glow_tex_id, data.tex, dUVdx, dUVdy)) : m_ke;

		//No lighting
		if (!m_lighting_enable)
		{
			fragColor = glm::vec4(glow_color, 1.0f);
			return;
		}

		//Calculate the lighting
		glm::vec3 fragPos = glm::vec3(data.pos);
		glm::vec3 normal = glm::normalize(data.nor);
		glm::vec3 viewDir = glm::normalize(m_viewer_pos - fragPos);
		for (size_t i = 0; i < m_point_lights.size(); ++i)
		{
			const auto &light = m_point_lights[i];
			glm::vec3 lightDir = glm::normalize(light.lightPos - fragPos);

			glm::vec3 ambient, diffuse, specular;
			float attenuation = 1.0f;
			{
				//Ambient
				ambient = light.lightColor * amb_color;

				//Diffuse
				float diffCof = glm::max(glm::dot(normal, lightDir), 0.0f);
				diffuse = light.lightColor * dif_color * diffCof * m_kd;

				//Blin-Phong Specular
				glm::vec3 halfwayDir = glm::normalize(viewDir + lightDir);
				float spec = glm::pow(glm::max(glm::dot(halfwayDir, normal), 0.0f), m_shininess);
				specular = light.lightColor * spec * spe_color;

				float distance = glm::length(light.lightPos - fragPos);
				attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance +
					light.attenuation.z * (distance * distance));
			}

			fragColor.x += (ambient.x + diffuse.x + specular.x) * attenuation;
			fragColor.y += (ambient.y + diffuse.y + specular.y) * attenuation;
			fragColor.z += (ambient.z + diffuse.z + specular.z) * attenuation;
		}

		fragColor = glm::vec4(fragColor.x + glow_color.x, fragColor.y + glow_color.y, fragColor.z + glow_color.z, 1.0f);

		//Tone mapping: HDR -> LDR
		//Refs: https://learnopengl.com/Advanced-Lighting/HDR
		{
			glm::vec3 hdrColor(fragColor);
			fragColor.x = 1.0f - glm::exp(-hdrColor.x * 2.0f);
			fragColor.y = 1.0f - glm::exp(-hdrColor.y * 2.0f);
			fragColor.z = 1.0f - glm::exp(-hdrColor.z * 2.0f);
		}
	}

	//----------------------------------------------TRBlinnPhongNormalMapShadingPipeline----------------------------------------------

	void TRBlinnPhongNormalMapShadingPipeline::vertexShader(VertexData &vertex) const
	{
		//Local space -> World space -> Camera space -> Project space
		vertex.pos = m_model_matrix * glm::vec4(vertex.pos.x, vertex.pos.y, vertex.pos.z, 1.0f);
		vertex.nor = glm::normalize(m_inv_trans_model_matrix * vertex.nor);
		vertex.cpos = m_view_project_matrix * vertex.pos;

		glm::vec3 T = glm::normalize(m_inv_trans_model_matrix * vertex.TBN[0]);
		glm::vec3 B = glm::normalize(m_inv_trans_model_matrix * vertex.TBN[1]);
		vertex.TBN = glm::mat3(T, B, vertex.nor);
	}

	void TRBlinnPhongNormalMapShadingPipeline::fragmentShader(const VertexData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		fragColor = glm::vec4(0.0f);

		//Fetch the corresponding color 
		glm::vec3 amb_color, dif_color, spe_color, glow_color;
		amb_color = dif_color = (m_diffuse_tex_id != -1) ? glm::vec3(texture2D(m_diffuse_tex_id, data.tex, dUVdx, dUVdy)) : m_kd;
		spe_color = (m_specular_tex_id != -1) ? glm::vec3(texture2D(m_specular_tex_id, data.tex, dUVdx, dUVdy)) : m_ks;
		glow_color = (m_glow_tex_id != -1) ? glm::vec3(texture2D(m_glow_tex_id, data.tex, dUVdx, dUVdy)) : m_ke;

		//No lighting
		if (!m_lighting_enable)
		{
			fragColor = glm::vec4(glow_color, 1.0f);
			return;
		}

		//Normal
		glm::vec3 normal = data.nor;
		if (m_normal_tex_id != -1)
		{
			normal = glm::vec3(texture2D(m_normal_tex_id, data.tex, dUVdx, dUVdy)) * 2.0f - glm::vec3(1.0f);
			normal = data.TBN * normal;
		}
		normal = glm::normalize(normal);

		//Calculate the lighting
		glm::vec3 fragPos = glm::vec3(data.pos);
		glm::vec3 viewDir = glm::normalize(m_viewer_pos - fragPos);
		for (size_t i = 0; i < m_point_lights.size(); ++i)
		{
			const auto &light = m_point_lights[i];
			glm::vec3 lightDir = glm::normalize(light.lightPos - fragPos);

			glm::vec3 ambient, diffuse, specular;
			float attenuation = 1.0f;
			{
				//Ambient
				ambient = light.lightColor * amb_color;

				//Diffuse
				float diffCof = glm::max(glm::dot(normal, lightDir), 0.0f);
				diffuse = light.lightColor * dif_color * diffCof * m_kd;

				//Blin-Phong Specular
				glm::vec3 halfwayDir = glm::normalize(viewDir + lightDir);
				float spec = glm::pow(glm::max(glm::dot(halfwayDir, normal), 0.0f), m_shininess);
				specular = light.lightColor * spec * spe_color;

				float distance = glm::length(light.lightPos - fragPos);
				attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance +
					light.attenuation.z * (distance * distance));
			}

			fragColor.x += (ambient.x + diffuse.x + specular.x) * attenuation;
			fragColor.y += (ambient.y + diffuse.y + specular.y) * attenuation;
			fragColor.z += (ambient.z + diffuse.z + specular.z) * attenuation;
		}

		fragColor = glm::vec4(fragColor.x + glow_color.x, fragColor.y + glow_color.y, fragColor.z + glow_color.z, 1.0f);

		//Tone mapping: HDR -> LDR
		//Refs: https://learnopengl.com/Advanced-Lighting/HDR
		{
			glm::vec3 hdrColor(fragColor);
			fragColor.x = 1.0f - glm::exp(-hdrColor.x * 2.0f);
			fragColor.y = 1.0f - glm::exp(-hdrColor.y * 2.0f);
			fragColor.z = 1.0f - glm::exp(-hdrColor.z * 2.0f);
		}
	}

}