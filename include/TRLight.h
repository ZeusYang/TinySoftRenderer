#ifndef TRLIGHT_SOURCE_H
#define TRLIGHT_SOURCE_H

#include <memory>

#include "glm/glm.hpp"

namespace TinyRenderer
{
	//Abstract class of light source
	class TRLight
	{
	public:
		typedef std::shared_ptr<TRLight> ptr;

		TRLight() : m_intensity(glm::vec3(1.0f)) {}
		TRLight(const glm::vec3 &intensity) : m_intensity(intensity) {}
		virtual ~TRLight() = default;

		const glm::vec3 &intensity() const { return m_intensity; }
		virtual float attenuation(const glm::vec3 &fragPos) const = 0;
		virtual float cutoff(const glm::vec3 &lightDir) const = 0;
		virtual glm::vec3 direction(const glm::vec3 &fragPos) const = 0;

	protected:
		glm::vec3 m_intensity;
	};

	//Point light source
	class TRPointLight : public TRLight
	{
	public:
		typedef std::shared_ptr<TRPointLight> ptr;

		TRPointLight(const glm::vec3 &intensity, const glm::vec3 &lightPos, const glm::vec3 &atten)
			: TRLight(intensity), m_lightPos(lightPos), m_attenuation(atten) { }

		virtual float attenuation(const glm::vec3 &fragPos) const override
		{
			//Refs: https://learnopengl.com/Lighting/Light-casters
			float distance = glm::length(m_lightPos - fragPos);
			return 1.0 / (m_attenuation.x + m_attenuation.y * distance + m_attenuation.z * (distance * distance));
		}

		virtual glm::vec3 direction(const glm::vec3 &fragPos) const override
		{
			return glm::normalize(m_lightPos - fragPos);
		}

		virtual float cutoff(const glm::vec3 &lightDir) const override { return 1.0f; }

		glm::vec3 &getLightPos() { return m_lightPos; }

	private:
		glm::vec3 m_lightPos;//Note: world space position of light source
		glm::vec3 m_attenuation;

	};

	//Spot light source
	class TRSpotLight final : public TRPointLight
	{
	public:
		typedef std::shared_ptr<TRSpotLight> ptr;

		TRSpotLight(const glm::vec3 &intensity, const glm::vec3 &lightPos, const glm::vec3 &atten, const glm::vec3 &dir,
			const float &innerCutoff, const float &outerCutoff) : TRPointLight(intensity, lightPos, atten),
			m_spotDir(glm::normalize(dir)), m_innerCutoff(innerCutoff), m_outerCutoff(outerCutoff) { }

		virtual float cutoff(const glm::vec3 &lightDir) const override
		{
			float theta = glm::dot(lightDir, -m_spotDir);
			static const float epsilon = m_innerCutoff - m_outerCutoff;
			return glm::clamp((theta - m_outerCutoff) / epsilon, 0.0f, 1.0f);
		}

		glm::vec3 &getSpotDirection() { return m_spotDir; }

	private:
		glm::vec3 m_spotDir;
		float m_innerCutoff;
		float m_outerCutoff;

	};

	//Directional light source
	class TRDirectionalLight final : public TRLight
	{
	public:
		typedef std::shared_ptr<TRDirectionalLight> ptr;

		TRDirectionalLight(const glm::vec3 &intensity, const glm::vec3 &dir)
			: TRLight(intensity), m_lightDir(glm::normalize(dir)) { }

		virtual float attenuation(const glm::vec3 &fragPos) const override { return 1.0f; }
		virtual glm::vec3 direction(const glm::vec3 &fragPos) const override { return m_lightDir; }
		virtual float cutoff(const glm::vec3 &lightDir) const override { return 1.0f; }

	private:
		glm::vec3 m_lightDir;
	};
}

#endif