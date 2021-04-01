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

#include <iostream>

using namespace TinyRenderer;

int main(int argc, char* args[])
{
	constexpr int width =  666;
	constexpr int height = 500;

	TRWindowsApp::ptr winApp = TRWindowsApp::getInstance(width, height, "TinySoftRenderer-By Wencong Yang");

	if (winApp == nullptr)
	{
		return -1;
	}

	TRRenderer::ptr renderer = std::make_shared<TRRenderer>(width, height);

	//camera
	glm::vec3 cameraPos = glm::vec3(0.8f, 0.0f, 3.7f);
	glm::vec3 lookAtTarget = glm::vec3(0.0f);

	renderer->setViewMatrix(TRMathUtils::calcViewMatrix(cameraPos, lookAtTarget, glm::vec3(0.0, 1.0, 0.0f)));
	renderer->setProjectMatrix(TRMathUtils::calcPerspProjectMatrix(45.0f, static_cast<float>(width) / height, 0.001f, 10.0f), 0.001f, 10.0f);

	//Load the rendering data
	TRDrawableMesh::ptr diabloMesh = std::make_shared<TRDrawableMesh>("model/diablo3_pose/diablo3_pose.obj");
	TRDrawableMesh::ptr houseMesh = std::make_shared<TRDrawableMesh>("model/floor.obj");
	TRDrawableMesh::ptr redLightMesh = std::make_shared<TRDrawableMesh>("model/light_red.obj");
	TRDrawableMesh::ptr greenLightMesh = std::make_shared<TRDrawableMesh>("model/light_green.obj");
	TRDrawableMesh::ptr blueLightMesh = std::make_shared<TRDrawableMesh>("model/light_blue.obj");
	renderer->addDrawableMesh({ diabloMesh, redLightMesh, greenLightMesh, blueLightMesh, houseMesh });
	redLightMesh->setLightingMode(TRLightingMode::TR_LIGHTING_DISABLE);
	greenLightMesh->setLightingMode(TRLightingMode::TR_LIGHTING_DISABLE);
	blueLightMesh->setLightingMode(TRLightingMode::TR_LIGHTING_DISABLE);

	//diabloMesh->setPolygonMode(TRPolygonMode::TR_TRIANGLE_WIRE);
	//houseMesh->setPolygonMode(TRPolygonMode::TR_TRIANGLE_WIRE);

	winApp->readyToStart();

	//Simple shading
	//renderer->setShaderPipeline(std::make_shared<TR3DShadingPipeline>());

	//Simple texture
	//renderer->setShaderPipeline(std::make_shared<TRTextureShadingPipeline>());

	//LOD visualization
	//renderer->setShaderPipeline(std::make_shared<TRLODVisualizePipeline>());

	//Phong lighting
	//renderer->setShaderPipeline(std::make_shared<TRPhongShadingPipeline>());
	
	//Blinn-Phong lighting
	renderer->setShaderPipeline(std::make_shared<TRBlinnPhongShadingPipeline>());

	//Blinn-Phong lighting using normal mapping
	//renderer->setShaderPipeline(std::make_shared<TRBlinnPhongNormalMapShadingPipeline>());

	glm::vec3 redLightPos = glm::vec3(0.0f, -0.05f, 1.2f);
	glm::vec3 greenLightPos = glm::vec3(0.87f, -0.05f, -0.87f);
	glm::vec3 blueLightPos = glm::vec3(-0.83f, -0.05f, -0.83f);
	int redLightIndex = renderer->addPointLight(redLightPos, glm::vec3(1.0, 0.7, 1.8), glm::vec3(1.9f, 0.0f, 0.0f));
	int greenLightIndex = renderer->addPointLight(greenLightPos, glm::vec3(1.0, 0.7, 1.8), glm::vec3(0.0f, 1.9f, 0.0f));
	int blueLightIndex = renderer->addPointLight(blueLightPos, glm::vec3(1.0, 0.7, 1.8), glm::vec3(0.0f, 0.0f, 1.9f));
	auto &redLight = renderer->getPointLight(redLightIndex);
	auto &greenLight = renderer->getPointLight(greenLightIndex);
	auto &blueLight = renderer->getPointLight(blueLightIndex);

	glm::mat4 redLightModelMat = glm::translate(glm::mat4(1.0f), redLightPos);
	redLightMesh->setModelMatrix(redLightModelMat);

	glm::mat4 greenLightModelMat = glm::translate(glm::mat4(1.0f), greenLightPos);
	greenLightMesh->setModelMatrix(greenLightModelMat);

	glm::mat4 blueLightModelMat = glm::translate(glm::mat4(1.0f), blueLightPos);
	blueLightMesh->setModelMatrix(blueLightModelMat);

	static glm::mat4 model_mat = glm::mat4(1.0f);
	model_mat = glm::rotate(model_mat,  30.0f, glm::vec3(0, 1, 0));
	renderer->setModelMatrix(model_mat);

	//Rendering loop
	while (!winApp->shouldWindowClose())
	{
		//Process event
		winApp->processEvent();

		//Clear frame buffer (both color buffer and depth buffer)
		renderer->clearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		//Draw call
		renderer->setViewerPos(cameraPos);
		renderer->renderAllDrawableMeshes();

		//Display to screen
		double deltaTime = winApp->updateScreenSurface(
			renderer->commitRenderedColorBuffer(),
			width, 
			height,
			4,
			renderer->getNumberOfClipFaces(),
			renderer->getNumberOfCullFaces());

		//Model transformation
		{
			redLightModelMat = glm::rotate(glm::mat4(1.0f), (float)deltaTime * 0.0008f, glm::vec3(0, 1, 0));
			redLightPos = glm::vec3(redLightModelMat * glm::vec4(redLightPos, 1.0f));
			redLightMesh->setModelMatrix(glm::translate(glm::mat4(1.0f), redLightPos));
			redLight.lightPos = redLightPos;

			greenLightModelMat = glm::rotate(glm::mat4(1.0f), (float)deltaTime * 0.0008f, glm::vec3(1, 1, 1));
			greenLightPos = glm::vec3(greenLightModelMat * glm::vec4(greenLightPos, 1.0f));
			greenLightMesh->setModelMatrix(glm::translate(glm::mat4(1.0f), greenLightPos));
			greenLight.lightPos = greenLightPos;

			blueLightModelMat = glm::rotate(glm::mat4(1.0f), (float)deltaTime * 0.0008f, glm::vec3(-1, 1, 1));
			blueLightPos = glm::vec3(blueLightModelMat * glm::vec4(blueLightPos, 1.0f));
			blueLightMesh->setModelMatrix(glm::translate(glm::mat4(1.0f), blueLightPos));
			blueLight.lightPos = blueLightPos;
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
