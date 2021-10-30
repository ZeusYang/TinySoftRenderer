#include "TRShaderProgram.h"

namespace TinyRenderer
{
	//----------------------------------------------TR3DShadingPipeline----------------------------------------------

	void TR3DShadingPipeline::vertexShader(VertexData &vertex) const
	{
		//Local space -> World space -> Camera space -> Project space
		vertex.m_pos = glm::vec3(m_modelMatrix * glm::vec4(vertex.m_pos.x, vertex.m_pos.y, vertex.m_pos.z, 1.0f));
		vertex.m_nor = glm::normalize(m_invTransModelMatrix * vertex.m_nor);
		vertex.m_cpos = m_viewProjectMatrix * glm::vec4(vertex.m_pos, 1.0f);
	}

	void TR3DShadingPipeline::fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		//Just return the color.
		fragColor = glm::vec4(data.m_tex, 0.0, 1.0f);
	}

	//----------------------------------------------TRDoNothingShadingPipeline----------------------------------------------

	void TRDoNothingShadingPipeline::vertexShader(VertexData &vertex) const
	{
		//do nothing at all
		vertex.m_cpos = glm::vec4(vertex.m_pos, 1.0f);
	}

	void TRDoNothingShadingPipeline::fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		//Just return the color.
		fragColor = glm::vec4(data.m_tex, 0.0, 1.0f);
	}

	//----------------------------------------------TRTextureShadingPipeline----------------------------------------------

	void TRTextureShadingPipeline::fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		//Default color
		fragColor = glm::vec4(m_kE, 1.0f);

		if (m_diffuseTexId != -1)
		{
			fragColor = texture2D(m_diffuseTexId, data.m_tex, dUVdx, dUVdy);
		}
	}

	//----------------------------------------------TRLODVisualizePipeline----------------------------------------------

	void TRLODVisualizePipeline::fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
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
		auto& tex = TRShadingPipeline::getTexture2D(m_diffuseTexId);
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

	void TRPhongShadingPipeline::fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		fragColor = glm::vec4(0.0f);

		//Fetch the corresponding color 
		glm::vec3 ambColor, difColor, speColor, glowColor;
		glm::vec4 difftexcolor = (m_diffuseTexId != -1) ? texture2D(m_diffuseTexId, data.m_tex, dUVdx, dUVdy) : glm::vec4(1.0f);
		ambColor = difColor = (m_diffuseTexId != -1) ? glm::vec3(difftexcolor) : m_kD;
		speColor = (m_specularTexId != -1) ? glm::vec3(texture2D(m_specularTexId, data.m_tex, dUVdx, dUVdy)) : m_kS;
		glowColor = (m_glowTexId != -1) ? glm::vec3(texture2D(m_glowTexId, data.m_tex, dUVdx, dUVdy)) : m_kE;

		//No lighting
		if (!m_lightingEnable)
		{
			fragColor = glm::vec4(glowColor, 1.0f);
			return;
		}

		//Calculate the lighting
		glm::vec3 fragPos = glm::vec3(data.m_pos);
		glm::vec3 normal = glm::normalize(data.m_nor);
		glm::vec3 viewDir = glm::normalize(m_viewerPos - fragPos);
#pragma unroll
		for (size_t i = 0; i < m_lights.size(); ++i)
		{
			const auto &light = m_lights[i];
			glm::vec3 lightDir = light->direction(fragPos);

			glm::vec3 ambient, diffuse, specular;
			float attenuation = 1.0f;

			{
				//Ambient
				ambient = light->intensity() * ambColor;

				//Diffuse
				float diffCof = glm::max(glm::dot(normal, lightDir), 0.0f);
				diffuse = light->intensity() * difColor * diffCof * m_kD;

				//Phong Specular
				glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
				float spec = std::pow(glm::max(glm::dot(viewDir, reflectDir), 0.0f), m_shininess);
				specular = light->intensity() * spec * speColor;

				attenuation = light->attenuation(fragPos);
			}

			float cutoff = light->cutoff(lightDir);
			fragColor += glm::vec4((ambient + diffuse + specular) * attenuation * cutoff, 0.0f);
		}

		fragColor = glm::vec4(fragColor.x + glowColor.x, fragColor.y + glowColor.y,
			fragColor.z + glowColor.z, difftexcolor.a * m_transparency);

		//Tone mapping: HDR -> LDR
		//Refs: https://learnopengl.com/Advanced-Lighting/HDR
		{
			glm::vec3 hdrColor(fragColor);
			fragColor = glm::vec4(glm::vec3(1.0f - glm::exp(-hdrColor * m_exposure)), fragColor.a);
		}
	}

	//----------------------------------------------TRBlinPhongShadingPipeline----------------------------------------------

	void TRBlinnPhongShadingPipeline::fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		fragColor = glm::vec4(0.0f);

		//Fetch the corresponding color 
		glm::vec3 ambColor, difColor, speColor, glowColor;
		glm::vec4 difftexcolor = (m_diffuseTexId != -1) ? texture2D(m_diffuseTexId, data.m_tex, dUVdx, dUVdy) : glm::vec4(1.0f);
		ambColor = difColor = (m_diffuseTexId != -1) ? glm::vec3(difftexcolor) : m_kD;
		speColor = (m_specularTexId != -1) ? glm::vec3(texture2D(m_specularTexId, data.m_tex, dUVdx, dUVdy)) : m_kS;
		glowColor = (m_glowTexId != -1) ? glm::vec3(texture2D(m_glowTexId, data.m_tex, dUVdx, dUVdy)) : m_kE;

		//No lighting
		if (!m_lightingEnable)
		{
			fragColor = glm::vec4(glowColor, difftexcolor.a);
			return;
		}

		//Calculate the lighting
		glm::vec3 fragPos = glm::vec3(data.m_pos);
		glm::vec3 normal = glm::normalize(data.m_nor);
		glm::vec3 viewDir = glm::normalize(m_viewerPos - fragPos);
#pragma unroll
		for (size_t i = 0; i < m_lights.size(); ++i)
		{
			const auto &light = m_lights[i];
			glm::vec3 lightDir = light->direction(fragPos);

			glm::vec3 ambient, diffuse, specular;
			float attenuation = 1.0f;
			{
				//Ambient
				ambient = light->intensity() * ambColor * m_kA;

				//Diffuse
				float diffCof = glm::max(glm::dot(normal, lightDir), 0.0f);
				diffuse = light->intensity() * difColor * diffCof * m_kD;
				//diffuse = glm::vec3(diffCof);

				//Blin-Phong Specular
				glm::vec3 halfwayDir = glm::normalize(viewDir + lightDir);
				float spec = glm::pow(glm::max(glm::dot(halfwayDir, normal), 0.0f), m_shininess);
				specular = light->intensity() * spec * speColor;

				attenuation = light->attenuation(fragPos);
			}

			float cutoff = light->cutoff(lightDir);
			fragColor += glm::vec4((ambient + diffuse + specular) * attenuation * cutoff, 0.0f);
		}

		fragColor = glm::vec4(fragColor.x + glowColor.x, fragColor.y + glowColor.y,
			fragColor.z + glowColor.z, difftexcolor.a * m_transparency);

		//Tone mapping: HDR -> LDR
		//Refs: https://learnopengl.com/Advanced-Lighting/HDR
		{
			glm::vec3 hdrColor(fragColor);
			fragColor = glm::vec4(glm::vec3(1.0f - glm::exp(-hdrColor * m_exposure)), fragColor.a);
		}
	}

	//----------------------------------------------TRBlinnPhongNormalMapShadingPipeline----------------------------------------------

	void TRBlinnPhongNormalMapShadingPipeline::vertexShader(VertexData &vertex) const
	{
		//Local space -> World space -> Camera space -> Project space
		vertex.m_pos = glm::vec3(m_modelMatrix * glm::vec4(vertex.m_pos.x, vertex.m_pos.y, vertex.m_pos.z, 1.0f));
		vertex.m_nor = glm::normalize(m_invTransModelMatrix * vertex.m_nor);
		vertex.m_cpos = m_viewProjectMatrix * glm::vec4(vertex.m_pos, 1.0f);

		glm::vec3 T = glm::normalize(m_invTransModelMatrix * vertex.m_tbn[0]);
		glm::vec3 B = glm::normalize(m_invTransModelMatrix * vertex.m_tbn[1]);
		vertex.m_tbn = glm::mat3(T, B, vertex.m_nor);
		vertex.m_needInterpolatedTBN = true;
	}

	void TRBlinnPhongNormalMapShadingPipeline::fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		fragColor = glm::vec4(0.0f);

		//Fetch the corresponding color 
		glm::vec3 ambColor, difColor, speColor, glowColor;
		glm::vec4 difftexcolor = (m_diffuseTexId != -1) ? texture2D(m_diffuseTexId, data.m_tex, dUVdx, dUVdy) : glm::vec4(1.0f);
		ambColor = difColor = (m_diffuseTexId != -1) ? glm::vec3(difftexcolor) : m_kD;
		speColor = (m_specularTexId != -1) ? glm::vec3(texture2D(m_specularTexId, data.m_tex, dUVdx, dUVdy)) : m_kS;
		glowColor = (m_glowTexId != -1) ? glm::vec3(texture2D(m_glowTexId, data.m_tex, dUVdx, dUVdy)) : m_kE;

		//No lighting
		if (!m_lightingEnable)
		{
			fragColor = glm::vec4(glowColor, 1.0f);
			return;
		}

		//Normal
		glm::vec3 normal = data.m_nor;
		if (m_normalTexId != -1)
		{
			normal = glm::vec3(texture2D(m_normalTexId, data.m_tex, dUVdx, dUVdy)) * 2.0f - glm::vec3(1.0f);
			normal = data.m_tbn * normal;
		}
		normal = glm::normalize(normal);

		//Calculate the lighting
		glm::vec3 fragPos = glm::vec3(data.m_pos);
		glm::vec3 viewDir = glm::normalize(m_viewerPos - fragPos);
#pragma unroll
		for (size_t i = 0; i < m_lights.size(); ++i)
		{
			const auto &light = m_lights[i];
			glm::vec3 lightDir = light->direction(fragPos);

			glm::vec3 ambient, diffuse, specular;
			float attenuation = 1.0f;
			{
				//Ambient
				ambient = light->intensity() * ambColor;

				//Diffuse
				float diffCof = glm::max(glm::dot(normal, lightDir), 0.0f);
				diffuse = light->intensity() * difColor * diffCof * m_kD;

				//Blin-Phong Specular
				glm::vec3 halfwayDir = glm::normalize(viewDir + lightDir);
				float spec = glm::pow(glm::max(glm::dot(halfwayDir, normal), 0.0f), m_shininess);
				specular = light->intensity() * spec * speColor;

				attenuation = light->attenuation(fragPos);
			}

			float cutoff = light->cutoff(lightDir);
			fragColor += glm::vec4((ambient + diffuse + specular) * attenuation * cutoff, 0.0f);
		}

		fragColor = glm::vec4(fragColor.x + glowColor.x, fragColor.y + glowColor.y,
			fragColor.z + glowColor.z, difftexcolor.a * m_transparency);

		//Tone mapping: HDR -> LDR
		//Refs: https://learnopengl.com/Advanced-Lighting/HDR
		{
			glm::vec3 hdrColor(fragColor);
			fragColor = glm::vec4(glm::vec3(1.0f - glm::exp(-hdrColor * m_exposure)), fragColor.a);
		}
	}

	//----------------------------------------------TRAlphaBlendingShadingPipeline----------------------------------------------

	void TRAlphaBlendingShadingPipeline::fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
		const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const
	{
		//Default color
		fragColor = glm::vec4(m_kE, 1.0f);

		if (m_diffuseTexId != -1)
		{
			fragColor = texture2D(m_diffuseTexId, data.m_tex, dUVdx, dUVdy);
		}

		fragColor.a *= m_transparency;
	}
}