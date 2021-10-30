#ifndef TRRENDERER_H
#define TRRENDERER_H

#include "glm/glm.hpp"
#include "SDL2/SDL.h"

#include "TRFrameBuffer.h"
#include "TRDrawableMesh.h"
#include "TRShadingState.h"
#include "TRShadingPipeline.h"

namespace TinyRenderer
{
	class TRRenderer final
	{
	public:
		typedef std::shared_ptr<TRRenderer> ptr;

		TRRenderer(int width, int height);
		~TRRenderer() = default;

		//Drawable objects load/unload
		void addDrawableMesh(TRDrawableMesh::ptr mesh);
		void addDrawableMesh(const std::vector<TRDrawableMesh::ptr> &meshes);
		void unloadDrawableMesh();

		void clearColor(const glm::vec4 &color) { m_backBuffer->clearColor(color); }
		void clearDepth(const float &depth) { m_backBuffer->clearDepth(depth); }
		void clearColorAndDepth(const glm::vec4 &color, const float &depth) { m_backBuffer->clearColorAndDepth(color, depth); }

		//Setting
		void setViewMatrix(const glm::mat4 &view) { m_viewMatrix = view; }
		void setModelMatrix(const glm::mat4 &model) { m_modelMatrix = model; }
		void setProjectMatrix(const glm::mat4 &project, float near, float far) { m_projectMatrix = project;m_frustumNearFar = glm::vec2(near, far); }
		void setShaderPipeline(TRShadingPipeline::ptr shader) { m_shaderHandler = shader; }
		void setViewerPos(const glm::vec3 &viewer);

		int addLightSource(TRLight::ptr lightSource);
		TRLight::ptr getLightSource(const int &index);
		void setExposure(const float &exposure);

		//Draw call
		unsigned int renderAllDrawableMeshes();

		unsigned int renderDrawableMesh(const size_t &index);

		//Commit rendered result
		unsigned char* commitRenderedColorBuffer();

		//Homogeneous space clipping - Sutherland Hodgeman algorithm
		static std::vector<TRShadingPipeline::VertexData> clipingSutherlandHodgeman(
			const TRShadingPipeline::VertexData &v0,
			const TRShadingPipeline::VertexData &v1,
			const TRShadingPipeline::VertexData &v2,
			const float &near, 
			const float &far);

	private:

		//Cliping auxiliary functions
		static std::vector<TRShadingPipeline::VertexData> clipingSutherlandHodgemanAux(
			const std::vector<TRShadingPipeline::VertexData> &polygon,
			const int &axis, 
			const int &side);

	private:

		//Drawable mesh array
		std::vector<TRDrawableMesh::ptr> m_drawableMeshes;

		//MVP transformation matrices
		glm::mat4 m_modelMatrix = glm::mat4(1.0f);				//From local space  -> world space
		glm::mat4 m_viewMatrix = glm::mat4(1.0f);				//From world space  -> camera space
		glm::mat4 m_projectMatrix = glm::mat4(1.0f);			//From camera space -> clip space
		glm::mat4 m_viewportMatrix = glm::mat4(1.0f);			//From ndc space    -> screen space

		TRShadingState m_shadingState;

		//Near plane & far plane
		glm::vec2 m_frustumNearFar;

		//Shader pipeline handler
		TRShadingPipeline::ptr m_shaderHandler = nullptr;

		//Double buffers
		TRFrameBuffer::ptr m_backBuffer;                      // The frame buffer that's goint to be written.
		TRFrameBuffer::ptr m_frontBuffer;                     // The frame buffer that's goint to be displayed.
		std::vector<unsigned char> m_renderedImg;			// The rendered image.
	};
}

#endif