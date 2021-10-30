#include "TRSceneParser.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "glm/gtc/matrix_transform.hpp"

#include "TRLight.h"

namespace TinyRenderer
{
	TRDrawableMesh::ptr TRSceneParser::getEntity(const std::string &name)
	{
		if (m_scene.m_entities.find(name) == m_scene.m_entities.end())
			return nullptr;
		return m_scene.m_entities[name];
	}

	int TRSceneParser::getLight(const std::string &name)
	{
		if (m_scene.m_lights.find(name) == m_scene.m_lights.end())
			return -1;
		return m_scene.m_lights[name];
	}

	void TRSceneParser::parse(const std::string &path, TRRenderer::ptr renderer, bool generatedMipmap)
	{
		std::ifstream sceneFile;
		sceneFile.open(path, std::ios::in);

		if (!sceneFile.is_open())
		{
			std::cerr << "File does not exist: " << path << std::endl;
			exit(1);
		}

		std::string line;
		while (std::getline(sceneFile, line))
		{
			if (line.empty())
				continue;

			std::stringstream ss;
			ss << line;
			std::string header;
			ss >> header;
			if (header == "Config:")
			{
				std::cout << "Config:=========================================\n";
				float exposure = 1.0f;
				{
					std::getline(sceneFile, line);
					exposure = parseFloat(line);
				}
				renderer->setExposure(exposure);
			}
			else if (header == "Camera:")
			{
				std::cout << "Camera:=========================================\n";
				{
					std::getline(sceneFile, line);
					m_scene.m_cameraPos = parseVec3(line);
				}

				{
					std::getline(sceneFile, line);
					m_scene.m_cameraFocus = parseVec3(line);
				}

				{
					std::getline(sceneFile, line);
					m_scene.m_cameraUp = parseVec3(line);
				}


			}
			else if (header == "Frustum:")
			{
				std::cout << "Frustum:========================================\n";
				{
					std::getline(sceneFile, line);
					m_scene.m_frustumFovy = parseFloat(line);
				}

				{
					std::getline(sceneFile, line);
					m_scene.m_frustumNear = parseFloat(line);
				}

				{
					std::getline(sceneFile, line);
					m_scene.m_frustumFar = parseFloat(line);
				}
			}
			else if (header == "PointLight:")
			{
				std::cout << "PointLight:=====================================\n";
				std::string name;
				{
					std::getline(sceneFile, line);
					name = parseStr(line);
				}

				glm::vec3 pos;
				{
					std::getline(sceneFile, line);
					pos = parseVec3(line);
				}

				glm::vec3 atten;
				{
					std::getline(sceneFile, line);
					atten = parseVec3(line);
				}

				glm::vec3 color;
				{
					std::getline(sceneFile, line);
					color = parseVec3(line);
				}

				TRLight::ptr lightSource = std::make_shared<TRPointLight>(color, pos, atten);
				m_scene.m_lights[name] = renderer->addLightSource(lightSource);
			}
			else if (header == "SpotLight:")
			{
				std::cout << "SpotLight:=====================================\n";
				std::string name;
				{
					std::getline(sceneFile, line);
					name = parseStr(line);
				}

				glm::vec3 pos;
				{
					std::getline(sceneFile, line);
					pos = parseVec3(line);
				}

				glm::vec3 atten;
				{
					std::getline(sceneFile, line);
					atten = parseVec3(line);
				}

				glm::vec3 color;
				{
					std::getline(sceneFile, line);
					color = parseVec3(line);
				}

				float innerCutoff;
				{
					std::getline(sceneFile, line);
					innerCutoff = parseFloat(line);
				}

				float outerCutoff;
				{
					std::getline(sceneFile, line);
					outerCutoff = parseFloat(line);
				}

				glm::vec3 spotDir;
				{
					std::getline(sceneFile, line);
					spotDir = parseVec3(line);
				}

				TRLight::ptr lightSource = std::make_shared<TRSpotLight>(color, pos, atten, spotDir,
					glm::cos(glm::radians(innerCutoff)), glm::cos(glm::radians(outerCutoff)));
				m_scene.m_lights[name] = renderer->addLightSource(lightSource);
			}
			else if (header == "DirectionalLight:")
			{
				std::cout << "DirectionalLight:=====================================\n";
				std::string name;
				{
					std::getline(sceneFile, line);
					name = parseStr(line);
				}

				glm::vec3 dir;
				{
					std::getline(sceneFile, line);
					dir = parseVec3(line);
				}

				glm::vec3 color;
				{
					std::getline(sceneFile, line);
					color = parseVec3(line);
				}

				TRLight::ptr lightSource = std::make_shared<TRDirectionalLight>(color, dir);
				m_scene.m_lights[name] = renderer->addLightSource(lightSource);
			}
			else if (header == "Entity:")
			{
				std::cout << "Entity:=========================================\n";
				std::string name;
				{
					std::getline(sceneFile, line);
					name = parseStr(line);
				}

				std::getline(sceneFile, line);
				std::string path = parseStr(line);
				TRDrawableMesh::ptr drawable = std::make_shared<TRDrawableMesh>(path, generatedMipmap);
				renderer->addDrawableMesh(drawable);
				m_scene.m_entities[name] = drawable;

				{
					glm::vec3 translate;
					{
						std::getline(sceneFile, line);
						translate = parseVec3(line);
					}

					glm::vec3 rotation;
					{
						std::getline(sceneFile, line);
						rotation = parseVec3(line);
					}

					glm::vec3 scale;
					{
						std::getline(sceneFile, line);
						scale = parseVec3(line);
					}
					glm::mat4 modelMatrix(1.0f);
					modelMatrix = glm::translate(modelMatrix, translate);
					modelMatrix = glm::scale(modelMatrix, scale);
					//modelMatrix = glm::rotate(modelMatrix, rotation);
					drawable->setModelMatrix(modelMatrix);
				}

				{
					std::getline(sceneFile, line);
					bool lighting = parseBool(line);
					drawable->setLightingMode(lighting ? TRLightingMode::TR_LIGHTING_ENABLE : TRLightingMode::TR_LIGHTING_DISABLE);
				}

				{
					std::getline(sceneFile, line);
					std::string cullface = parseStr(line);
					if (cullface == "back")
						drawable->setCullfaceMode(TRCullFaceMode::TR_CULL_BACK);
					else if (cullface == "front")
						drawable->setCullfaceMode(TRCullFaceMode::TR_CULL_FRONT);
					else
						drawable->setCullfaceMode(TRCullFaceMode::TR_CULL_DISABLE);
				}

				{
					std::getline(sceneFile, line);
					bool depthtest = parseBool(line);
					drawable->setDepthtestMode(depthtest ? TRDepthTestMode::TR_DEPTH_TEST_ENABLE : TRDepthTestMode::TR_DEPTH_TEST_DISABLE);
				}

				{
					std::getline(sceneFile, line);
					bool depthwrite = parseBool(line);
					drawable->setDepthwriteMode(depthwrite ? TRDepthWriteMode::TR_DEPTH_WRITE_ENABLE : TRDepthWriteMode::TR_DEPTH_WRITE_DISABLE);
				}

				{
					std::getline(sceneFile, line);
					std::string alphablend = parseStr(line);
					if (alphablend == "alphablend")
						drawable->setAlphablendMode(TRAlphaBlendingMode::TR_ALPHA_BLENDING);
					else if (alphablend == "alpha2coverage")
						drawable->setAlphablendMode(TRAlphaBlendingMode::TR_ALPHA_TO_COVERAGE);
					else
						drawable->setAlphablendMode(TRAlphaBlendingMode::TR_ALPHA_DISABLE);
				}

				//Material
				std::getline(sceneFile, line);
				{
					{
						float alpha;
						std::getline(sceneFile, line);
						alpha = parseFloat(line);
						drawable->setTransparency(alpha);
					}

					{
						float ns;
						std::getline(sceneFile, line);
						ns = parseFloat(line);
						drawable->setSpecularExponent(ns);
					}

					{
						glm::vec3 ka;
						std::getline(sceneFile, line);
						ka = parseVec3(line);
						drawable->setAmbientCoff(ka);
					}

					{
						glm::vec3 kd;
						std::getline(sceneFile, line);
						kd = parseVec3(line);
						drawable->setDiffuseCoff(kd);
					}

					{
						glm::vec3 ks;
						std::getline(sceneFile, line);
						ks = parseVec3(line);
						drawable->setSpecularCoff(ks);
					}

					{
						glm::vec3 ke;
						std::getline(sceneFile, line);
						ke = parseVec3(line);
						drawable->setEmissionCoff(ke);
					}

				}
			}

		}

		sceneFile.close();
	}

	float TRSceneParser::parseFloat(std::string str) const
	{
		std::stringstream ss;
		std::string token;
		ss << str;
		ss >> token;
		float ret;
		ss >> ret;
		std::cout << token << " " << ret << std::endl;
		return ret;
	}

	glm::vec3 TRSceneParser::parseVec3(std::string str) const
	{
		std::stringstream ss;
		std::string token;
		ss << str;
		ss >> token;
		glm::vec3 ret;
		ss >> ret.x;
		ss >> ret.y;
		ss >> ret.z;
		std::cout << token << " " << ret.x << "," << ret.y << "," << ret.z << std::endl;
		return ret;
	}

	bool TRSceneParser::parseBool(std::string str) const
	{
		std::stringstream ss;
		std::string token;
		ss << str;
		ss >> token;
		bool ret = true;
		std::string tag;
		ss >> tag;
		if (tag == "true")
			ret = true;
		if (tag == "false")
			ret = false;
		std::cout << token << " " << (ret ? "true" : "false") << std::endl;
		return true;
	}

	std::string TRSceneParser::parseStr(std::string str) const
	{
		std::stringstream ss;
		std::string token;
		ss << str;
		ss >> token;
		std::string ret;
		ss >> ret;
		std::cout << token << " " << ret << std::endl;
		return ret;
	}
}