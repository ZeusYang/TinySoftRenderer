/*The MIT License (MIT)

Copyright (c) 2021-Present, Wencong Yang (yangwc3@mail2.sysu.edu.cn).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.*/

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "TRWindowsApp.h"
#include "TRRenderer.h"
#include "TRMathUtils.h"
#include "TRShaderProgram.h"
#include "TRSceneParser.h"

#include <iostream>

using namespace TinyRenderer;

int main(int argc, char* args[])
{
	constexpr int width =  666;
	constexpr int height = 500;

	TRWindowsApp::ptr winApp = TRWindowsApp::getInstance(width, height, "TinySoftRenderer-By yangwc");

	if (winApp == nullptr)
	{
		return -1;
	}

	bool generatedMipmap = true;
	TRRenderer::ptr renderer = std::make_shared<TRRenderer>(width, height);

	//Load scene
	TRSceneParser parser;
	parser.parse("../../scenes/normalmapping.scene", renderer, generatedMipmap);

	renderer->setViewMatrix(TRMathUtils::calcViewMatrix(parser.m_scene.m_cameraPos,
		parser.m_scene.m_cameraFocus, parser.m_scene.m_cameraUp));
	renderer->setProjectMatrix(TRMathUtils::calcPerspProjectMatrix(parser.m_scene.m_frustumFovy,
		static_cast<float>(width) / height, parser.m_scene.m_frustumNear, parser.m_scene.m_frustumFar),
		parser.m_scene.m_frustumNear, parser.m_scene.m_frustumFar);

	winApp->readyToStart();

	//Blinn-Phong lighting
	renderer->setShaderPipeline(std::make_shared<TRBlinnPhongNormalMapShadingPipeline>());

	TRPointLight::ptr redLight = std::dynamic_pointer_cast<TRPointLight>(renderer->getLightSource(parser.getLight("readLight")));
	TRPointLight::ptr greenLight = std::dynamic_pointer_cast<TRPointLight>(renderer->getLightSource(parser.getLight("greenLight")));
	TRPointLight::ptr blueLight = std::dynamic_pointer_cast<TRPointLight>(renderer->getLightSource(parser.getLight("blueLight")));

	glm::mat4 redLightModelMat(1.0f);
	glm::mat4 greenLightModelMat(1.0f);
	glm::mat4 blueLightModelMat(1.0f);
	glm::vec3 &redLightPos = redLight->getLightPos();
	glm::vec3 &greenLightPos = greenLight->getLightPos();
	glm::vec3 &blueLightPos = blueLight->getLightPos();

	glm::vec3 cameraPos = parser.m_scene.m_cameraPos;
	glm::vec3 lookAtTarget = parser.m_scene.m_cameraFocus;

	TRDrawableMesh::ptr redLightMesh = parser.getEntity("RedLight");
	TRDrawableMesh::ptr greenLightMesh = parser.getEntity("GreenLight");
	TRDrawableMesh::ptr blueLightMesh = parser.getEntity("BlueLight");

	const auto originMat = redLightMesh->getModelMatrix();

	//Rendering loop
	while (!winApp->shouldWindowClose())
	{
		//Process event
		winApp->processEvent();

		//Clear frame buffer (both color buffer and depth buffer)
		renderer->clearColorAndDepth(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 0.0f);

		//Draw call
		renderer->setViewerPos(cameraPos);
		auto numTriangles = renderer->renderAllDrawableMeshes();

		//Display to screen
		double deltaTime = winApp->updateScreenSurface(
			renderer->commitRenderedColorBuffer(),
			width, 
			height,
			3,//RGB
			numTriangles);

		//Model transformation
		{
			redLightModelMat = glm::rotate(glm::mat4(1.0f), (float)deltaTime * 0.0008f, glm::vec3(1, 1, 0));
			redLightPos = glm::vec3(redLightModelMat * glm::vec4(redLightPos, 1.0f));
			redLightMesh->setModelMatrix(glm::translate(glm::mat4(1.0f), redLightPos));

			greenLightModelMat = glm::rotate(glm::mat4(1.0f), (float)deltaTime * 0.0008f, glm::vec3(1, 1, 1));
			greenLightPos = glm::vec3(greenLightModelMat * glm::vec4(greenLightPos, 1.0f));
			greenLightMesh->setModelMatrix(glm::translate(glm::mat4(1.0f), greenLightPos));

			blueLightModelMat = glm::rotate(glm::mat4(1.0f), (float)deltaTime * 0.0008f, glm::vec3(-1, 1, 1));
			blueLightPos = glm::vec3(blueLightModelMat * glm::vec4(blueLightPos, 1.0f));
			blueLightMesh->setModelMatrix(glm::translate(glm::mat4(1.0f), blueLightPos));
		}

		//Camera operation
		{
			//Camera rotation
			if (winApp->getIsMouseLeftButtonPressed())
			{
				int deltaX = winApp->getMouseMotionDeltaX();
				int deltaY = winApp->getMouseMotionDeltaY();
				glm::mat4 cameraRotMat(1.0f);
				if(std::abs(deltaX) > std::abs(deltaY))
					cameraRotMat = glm::rotate(glm::mat4(1.0f), -deltaX * 0.001f, glm::vec3(0, 1, 0));
				else 
					cameraRotMat = glm::rotate(glm::mat4(1.0f), -deltaY * 0.001f, glm::vec3(1, 0, 0));

				cameraPos = glm::vec3(cameraRotMat * glm::vec4(cameraPos, 1.0f));
				renderer->setViewMatrix(TRMathUtils::calcViewMatrix(cameraPos, lookAtTarget, glm::vec3(0.0, 1.0, 0.0f)));
			}

			//Camera zoom in and zoom out
			if (winApp->getMouseWheelDelta() != 0)
			{
				glm::vec3 dir = glm::normalize(cameraPos - lookAtTarget);
				float dist = glm::length(cameraPos - lookAtTarget);
				glm::vec3 newPos = cameraPos + (winApp->getMouseWheelDelta() * 0.1f) * dir;
				if (glm::length(newPos - lookAtTarget) > 1.0f)
				{
					cameraPos = newPos;
					renderer->setViewMatrix(TRMathUtils::calcViewMatrix(cameraPos, lookAtTarget, glm::vec3(0.0, 1.0, 0.0f)));
				}
			}
		}
	}

	renderer->unloadDrawableMesh();

	return 0;
}
