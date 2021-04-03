#ifndef TRRENDERER_H
#define TRRENDERER_H

#include "glm/glm.hpp"
#include "SDL2/SDL.h"

#include "TRFrameBuffer.h"
#include "TRDrawableMesh.h"
#include "TRShadingState.h"
#include "TRShadingPipeline.h"

#include <mutex>

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

		void clearColor(glm::vec4 color);

		//Setting
		void setViewMatrix(const glm::mat4 &view);
		void setModelMatrix(const glm::mat4 &model);
		void setProjectMatrix(const glm::mat4 &project, float near, float far);
		void setShaderPipeline(TRShadingPipeline::ptr shader);
		void setViewerPos(const glm::vec3 &viewer);

		int addPointLight(glm::vec3 pos, glm::vec3 atten, glm::vec3 color);
		TRPointLight &getPointLight(const int &index);

		glm::mat4 getMVPMatrix();

		//Draw call
		void renderAllDrawableMeshes();

		//Commit rendered result
		unsigned char* commitRenderedColorBuffer();
		unsigned int getNumberOfClipFaces() const;
		unsigned int getNumberOfCullFaces() const;

	private:


	private:

		//Homogeneous space clipping - Sutherland Hodgeman algorithm
		std::vector<TRShadingPipeline::VertexData> clipingSutherlandHodgeman(
			const TRShadingPipeline::VertexData &v0,
			const TRShadingPipeline::VertexData &v1,
			const TRShadingPipeline::VertexData &v2) const;

		//Cliping auxiliary functions
		std::vector<TRShadingPipeline::VertexData> clipingSutherlandHodgeman_aux(
			const std::vector<TRShadingPipeline::VertexData> &polygon,
			const int &axis, 
			const int &side) const;

		//Back face culling
		bool isBackFacing(const glm::ivec2 &v0, const glm::ivec2 &v1, const glm::ivec2 &v2, TRCullFaceMode mode) const;

	private:

		//Drawable mesh array
		std::vector<TRDrawableMesh::ptr> m_drawableMeshes;

		//MVP transformation matrices
		glm::mat4 m_viewMatrix = glm::mat4(1.0f);
		glm::mat4 m_modelMatrix = glm::mat4(1.0f);
		glm::mat4 m_projectMatrix = glm::mat4(1.0f);
		glm::mat4 m_mvp_matrix = glm::mat4(1.0f);
		bool m_mvp_dirty = false;

		//Near plane & far plane
		glm::vec2 m_frustum_near_far;

		//Viewport transformation (ndc space -> screen space)
		glm::mat4 m_viewportMatrix = glm::mat4(1.0f);

		//Shader pipeline handler
		TRShadingPipeline::ptr m_shader_handler = nullptr;

		//Double buffers
		TRFrameBuffer::ptr m_backBuffer;                      // The frame buffer that's goint to be written.
		TRFrameBuffer::ptr m_frontBuffer;                     // The frame buffer that's goint to be display.

		struct Profile
		{
			unsigned int m_num_cliped_triangles = 0;
			unsigned int m_num_culled_triangles = 0;
		};
		Profile m_clip_cull_profile;
	};
}

#endif