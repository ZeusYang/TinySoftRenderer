#include "TRRenderer.h"

#include "glm/gtc/matrix_transform.hpp"

#include "TRShadingPipeline.h"
#include "TRShaderProgram.h"
#include "TRMathUtils.h"
#include "TRParallelWrapper.h"

#include "tbb/parallel_pipeline.h"
#include "tbb/task_arena.h"

namespace TinyRenderer
{
	using MutexType = tbb::spin_mutex;				//TBB thread mutex type
	static MutexType FACE_INDEX_MUTEX;				//Face index mutex lock
	static constexpr int PIPELINE_BATCH_SIZE = 512; //The number of faces processed for each batch

	//The cache for rasterized results. For example: the face i -> FragmentCache[i]
	using FragmentCache = std::array<std::vector<TRShadingPipeline::QuadFragments>, PIPELINE_BATCH_SIZE>;

	//----------------------------------------------DrawcallSetting----------------------------------------------
	//Draw call setting which would be utilized in shading parallel pipeline 
	class DrawcallSetting final
	{
	public:

		const TRVertexBuffer &vertexBuffer;			//Vertex data buffer
		const TRIndexBuffer  &indexBuffer;			//Index data buffer
		TRShadingPipeline *shaderHandler;			//Shader handler
		const TRShadingState &shadingState;			//Shading state
		const glm::mat4 &viewportMatrix;			//Viewport transformation matrix
		float near, far;							//Near plane and far plane of frustum
		TRFrameBuffer *frameBuffer;					//Framebuffer 

		explicit DrawcallSetting(const TRVertexBuffer &vbo, const TRIndexBuffer &ibo, TRShadingPipeline *handler,
			const TRShadingState &state, const glm::mat4 &viewportMat, float np, float fp, TRFrameBuffer *fb)
			: vertexBuffer(vbo), indexBuffer(ibo), shaderHandler(handler), shadingState(state),
			viewportMatrix(viewportMat), near(np), far(fp), frameBuffer(fb) {}
	};

	//----------------------------------------------FramebufferMutex----------------------------------------------
	//Each point (i,j) of framebuffer have its own mutex lock for avoiding accessing conflict among different threads
	class FramebufferMutex final
	{
	public:
		FramebufferMutex(int width, int height)
			: width(width), height(height) 
		{
			mutexBuffer.resize(width * height, nullptr);
			for (int i = 0; i < width * height; ++i)
			{
				mutexBuffer[i] = new MutexType();
			}
		}

		~FramebufferMutex()
		{
			for (auto &mutex : mutexBuffer)
			{
				delete mutex;
				mutex = nullptr;
			}
		}

		MutexType &getLocker(const int &x, const int &y)
		{
			return *mutexBuffer[y * width + x];
		}

	public:
		using MutexBuffer = std::vector<MutexType*>;
		int width, height;
		MutexBuffer mutexBuffer;
	};

	//----------------------------------------------TBBVertexRastFilter----------------------------------------------
	//Vertex transformation, cliping, culling and rasterization.
	class TBBVertexRastFilter final
	{
	public:
		explicit TBBVertexRastFilter(int bs, int &startIndex, int &overIndex, const DrawcallSetting &drawcall,
			FragmentCache &cache) : batchSize(bs), startIndex(startIndex), overIndex(overIndex), currIndex(startIndex),
			drawCall(drawcall), fragmentCache(cache) {}

		int operator()(tbb::flow_control &fc) const
		{
			//Note: process the faces in [startIndex, overIndex) parallely
			int faceIndex = 0;
			//Fetch the face index that needs to be processed.
			{
				//Note: a mutex lock must be herein for accessing to currIndex
				MutexType::scoped_lock lock(FACE_INDEX_MUTEX);
				if (currIndex >= overIndex)
				{
					fc.stop();//Exceed range, stop the processing flow
					return -1;
				}
				faceIndex = currIndex;
				++currIndex;
			}

			//The fragment cache index
			int order = faceIndex - startIndex;
			faceIndex *= 3;

			TRShadingPipeline::VertexData v[3];
			const auto &indexBuffer = drawCall.indexBuffer;
			const auto &vertexBuffer = drawCall.vertexBuffer;
#pragma unroll 3
			for (int i = 0; i < 3; ++i)
			{
				v[i].pos = vertexBuffer[indexBuffer[faceIndex + i]].vpositions;
				v[i].col = vertexBuffer[indexBuffer[faceIndex + i]].vcolors;
				v[i].nor = vertexBuffer[indexBuffer[faceIndex + i]].vnormals;
				v[i].tex = vertexBuffer[indexBuffer[faceIndex + i]].vtexcoords;
				v[i].TBN[0] = vertexBuffer[indexBuffer[faceIndex + i]].vtangent;
				v[i].TBN[1] = vertexBuffer[indexBuffer[faceIndex + i]].vbitangent;
			}

			//Vertex shader stage
			drawCall.shaderHandler->vertexShader(v[0]);
			drawCall.shaderHandler->vertexShader(v[1]);
			drawCall.shaderHandler->vertexShader(v[2]);

			//Homogeneous space cliping
			std::vector<TRShadingPipeline::VertexData> clipped_vertices;
			clipped_vertices = TRRenderer::clipingSutherlandHodgeman(v[0], v[1], v[2], drawCall.near, drawCall.far);
			if (clipped_vertices.empty())
			{
				return -1; //Totally outside
			}

			//Perspective division: from clip space -> ndc space
			for (auto &vert : clipped_vertices)
			{
				TRShadingPipeline::VertexData::prePerspCorrection(vert);
				vert.cpos *= vert.rhw;
			}

			int num_verts = clipped_vertices.size();
			for (int i = 0; i < num_verts - 2; ++i)
			{
				//Triangle assembly
				TRShadingPipeline::VertexData vert[3] = { clipped_vertices[0], clipped_vertices[i + 1], clipped_vertices[i + 2] };

				//Transform to screen space
				vert[0].spos = glm::ivec2(drawCall.viewportMatrix * vert[0].cpos + glm::vec4(0.5f));
				vert[1].spos = glm::ivec2(drawCall.viewportMatrix * vert[1].cpos + glm::vec4(0.5f));
				vert[2].spos = glm::ivec2(drawCall.viewportMatrix * vert[2].cpos + glm::vec4(0.5f));

				//Backface culling
				if (shouldCulled(vert[0].spos, vert[1].spos, vert[2].spos, drawCall.shadingState.trCullFaceMode))
				{
					continue;
				}

				//Rasterization
				TRShadingPipeline::rasterize_fill_edge_function(vert[0], vert[1], vert[2],
					drawCall.frameBuffer->getWidth(), drawCall.frameBuffer->getHeight(), fragmentCache[order]);
			}

			return order;
		}

	private:

		static inline bool shouldCulled(const glm::ivec2 &v0, const glm::ivec2 &v1, const glm::ivec2 &v2, TRCullFaceMode mode)
		{
			if (mode == TRCullFaceMode::TR_CULL_DISABLE)
				return false;
			//Back face culling in screen space
			auto e1 = v1 - v0;
			auto e2 = v2 - v0;
			int orient = e1.x * e2.y - e1.y * e2.x;
			return (mode == TRCullFaceMode::TR_CULL_BACK) ? orient > 0 : orient < 0;
		}

	private:
		int batchSize;
		const int startIndex;
		const int overIndex;
		int &currIndex;
		const DrawcallSetting &drawCall;

		FragmentCache &fragmentCache;
	};

	//----------------------------------------------TBBFragmentFilter----------------------------------------------
	//Fragment shader execution
	class TBBFragmentFilter final
	{
	public:
		explicit TBBFragmentFilter(int bs, const DrawcallSetting &drawcall, FragmentCache &cache, FramebufferMutex &fbMutex)
			: batchSize(bs), drawCall(drawcall), fragmentCache(cache), framebufferMutex(fbMutex) {}

		void operator()(int index) const
		{
			//No fragments
			if (index == -1 || fragmentCache[index].empty())
				return;

			//Fragment shader & Depth testing
			auto fragment_func = [&](TRShadingPipeline::FragmentData &fragment, const glm::vec2 &dUVdx, const glm::vec2 &dUVdy)
			{
				//Note: spos.x equals -1 -> invalid fragment
				if (fragment.spos.x == -1)
					return;

				auto &coverage = fragment.coverage;
				const auto &fragCoord = fragment.spos;
				auto &framebuffer = drawCall.frameBuffer;
				const auto &shadingState = drawCall.shadingState;

				//A mutex locker herein for (x,y) to prevent from simultanenously accessing depth buffer at the same place
				MutexType::scoped_lock lock(framebufferMutex.getLocker(fragCoord.x, fragCoord.y));

				const int samplingNum = TRMaskPixelSampler::getSamplingNum();

				int num_failed = 0;
				//Depth testing for each sampling point (Early Z strategy herein)
				if (shadingState.trDepthTestMode == TRDepthTestMode::TR_DEPTH_TEST_ENABLE)
				{
					const auto &coverageDepth = fragment.coverage_depth;
#pragma unroll
					for (int s = 0; s < samplingNum; ++s)
					{
						if (coverage[s] == 1 &&
							framebuffer->readDepth(fragCoord.x, fragCoord.y, s) >= coverageDepth[s])
						{
							coverage[s] = 0;//Occuluded
							++num_failed;
						}
						else if (coverage[s] == 0)
						{
							++num_failed;
						}
					}
				}

				//No valid mask, just discard.
				if (num_failed == samplingNum)
					return;

				//Execute fragment shader, and save the result to frame buffer
				glm::vec4 fragColor;
				drawCall.shaderHandler->fragmentShader(fragment, fragColor, dUVdx, dUVdy);

				//Alpha to coverage
				//Note: alpha to coverage only work with MSAA
				//Refs: http://www.zwqxin.com/archives/opengl/talk-about-alpha-to-coverage.html
				if (shadingState.trAlphaBlendMode == TRAlphaBlendingMode::TR_ALPHA_TO_COVERAGE && samplingNum >= 4)
				{
					int num_cancle = samplingNum  - int(samplingNum * fragColor.a);
					//None left, just discard in advance
					if (num_cancle == samplingNum)
					{
						return;
					}
					for (int c = 0; c < num_cancle; ++c)
					{
						coverage[c] = 0;
					}
				}

				//Save the rendered result to frame buffer
				framebuffer->writeCoverageMask(fragCoord.x, fragCoord.y, coverage);
				switch (shadingState.trAlphaBlendMode)
				{
				case TRAlphaBlendingMode::TR_ALPHA_DISABLE://No alpha blending
				case TRAlphaBlendingMode::TR_ALPHA_TO_COVERAGE://Or alpha to coverage
					framebuffer->writeColorWithMask(fragCoord.x, fragCoord.y, fragColor, coverage);
					break;
				case TRAlphaBlendingMode::TR_ALPHA_BLENDING://Alpha blending
					framebuffer->writeColorWithMaskAlphaBlending(fragCoord.x, fragCoord.y, fragColor, coverage);
					break;
				default:
					framebuffer->writeColorWithMask(fragCoord.x, fragCoord.y, fragColor, coverage);
				}

				//Depth writing
				if (shadingState.trDepthWriteMode == TRDepthWriteMode::TR_DEPTH_WRITE_ENABLE)
				{
					framebuffer->writeDepthWithMask(fragCoord.x, fragCoord.y, fragment.coverage_depth, coverage);
				}
				
			};

			//Note: 2x2 fragment block as an execution unit for calculating dFdx, dFdy.
			parallelFor((size_t)0, (size_t)fragmentCache[index].size(), [&](const size_t &f)
			{
				auto &block = fragmentCache[index][f];

				//Perspective correction restore
				block.aftPrespCorrectionForBlocks();

				//Calculate dUVdx, dUVdy for mipmap
				glm::vec2 dUVdx(block.dUdx(), block.dVdx());
				glm::vec2 dUVdy(block.dUdy(), block.dVdy());

				fragment_func(block.fragments[0], dUVdx, dUVdy);
				fragment_func(block.fragments[1], dUVdx, dUVdy);
				fragment_func(block.fragments[2], dUVdx, dUVdy);
				fragment_func(block.fragments[3], dUVdx, dUVdy);

			}, TRExecutionPolicy::TR_PARALLEL);

			fragmentCache[index].clear();
		}

	private:
		int batchSize;
		const DrawcallSetting &drawCall;
		FragmentCache &fragmentCache;
		FramebufferMutex &framebufferMutex;
	};

	//----------------------------------------------TRRenderer----------------------------------------------

	TRRenderer::TRRenderer(int width, int height)
		: m_backBuffer(nullptr), m_frontBuffer(nullptr)
	{
		//Double buffer to avoid flickering
		m_backBuffer = std::make_shared<TRFrameBuffer>(width, height);
		m_frontBuffer = std::make_shared<TRFrameBuffer>(width, height);
		m_renderedImg.resize(width * height * 3, 0);

		//Setup viewport matrix (ndc space -> screen space)
		m_viewportMatrix = TRMathUtils::calcViewPortMatrix(width, height);
	}

	void TRRenderer::addDrawableMesh(TRDrawableMesh::ptr mesh)
	{
		m_drawableMeshes.push_back(mesh);
	}

	void TRRenderer::addDrawableMesh(const std::vector<TRDrawableMesh::ptr> &meshes)
	{
		m_drawableMeshes.insert(m_drawableMeshes.end(), meshes.begin(), meshes.end());
	}

	void TRRenderer::unloadDrawableMesh()
	{
		for (size_t i = 0; i < m_drawableMeshes.size(); ++i)
		{
			m_drawableMeshes[i]->clear();
		}
		std::vector<TRDrawableMesh::ptr>().swap(m_drawableMeshes);
	}

	void TRRenderer::setViewerPos(const glm::vec3 &viewer)
	{
		if (m_shader_handler == nullptr)
			return;
		m_shader_handler->setViewerPos(viewer);
	}

	int TRRenderer::addPointLight(glm::vec3 pos, glm::vec3 atten, glm::vec3 color)
	{
		return TRShadingPipeline::addPointLight(pos, atten, color);
	}

	TRPointLight &TRRenderer::getPointLight(const int &index)
	{
		return TRShadingPipeline::getPointLight(index);
	}

	unsigned int TRRenderer::renderAllDrawableMeshes()
	{
		if (m_shader_handler == nullptr)
		{
			m_shader_handler = std::make_shared<TR3DShadingPipeline>();
		}

		//Load the matrices
		m_shader_handler->setModelMatrix(m_modelMatrix);
		m_shader_handler->setViewProjectMatrix(m_projectMatrix * m_viewMatrix);

		//Draw a mesh step by step
		unsigned int num_triangles = 0;

		for (size_t m = 0; m < m_drawableMeshes.size(); ++m)
		{
			renderDrawableMesh(m);
		}

		//MSAA resolve stage
		m_backBuffer->resolve();

		//Swap double buffers
		{
			std::swap(m_backBuffer, m_frontBuffer);
		}

		return num_triangles;
	}

	unsigned int TRRenderer::renderDrawableMesh(const size_t &index)
	{
		if (index >= m_drawableMeshes.size())
			return 0;

		//unsigned int num_triangles = 0;
		const auto &drawable = m_drawableMeshes[index];
		const auto &submeshes = drawable->getDrawableSubMeshes();

		//Configuration
		m_shading_state.trCullFaceMode = drawable->getCullfaceMode();
		m_shading_state.trDepthTestMode = drawable->getDepthtestMode();
		m_shading_state.trDepthWriteMode = drawable->getDepthwriteMode();
		m_shading_state.trAlphaBlendMode = drawable->getAlphablendMode();

		//Setup the shading options
		m_shader_handler->setModelMatrix(drawable->getModelMatrix());
		m_shader_handler->setLightingEnable(drawable->getLightingMode() == TRLightingMode::TR_LIGHTING_ENABLE);
		m_shader_handler->setAmbientCoef(drawable->getAmbientCoff());
		m_shader_handler->setDiffuseCoef(drawable->getDiffuseCoff());
		m_shader_handler->setSpecularCoef(drawable->getSpecularCoff());
		m_shader_handler->setEmissionColor(drawable->getEmissionCoff());
		m_shader_handler->setShininess(drawable->getSpecularExponent());
		m_shader_handler->setTransparency(drawable->getTransparency());

		//Note: For those drawables which need the alpha blending, we should make sure the faces rendered in a fixed order 
		tbb::filter_mode executeMopde = m_shading_state.trAlphaBlendMode == TRAlphaBlendingMode::TR_ALPHA_DISABLE ?
			tbb::filter_mode::parallel : tbb::filter_mode::serial_in_order;

		//Setting for drawcall
		static int ntokens = tbb::this_task_arena::max_concurrency() * 128;
		static FragmentCache fragmentCache;
		static FramebufferMutex framebufferMutex(m_backBuffer->getWidth(), m_backBuffer->getHeight());

		for (size_t s = 0; s < submeshes.size(); ++s)
		{
			const auto &submesh = submeshes[s];
			int faceNum = submesh.getIndices().size() / 3;

			//Texture setting
			m_shader_handler->setDiffuseTexId(submesh.getDiffuseMapTexId());
			m_shader_handler->setSpecularTexId(submesh.getSpecularMapTexId());
			m_shader_handler->setNormalTexId(submesh.getNormalMapTexId());
			m_shader_handler->setGlowTexId(submesh.getGlowMapTexId());

			//Draw call setting
			DrawcallSetting drawCall(submesh.getVertices(), submesh.getIndices(), m_shader_handler.get(),
				m_shading_state, m_viewportMatrix, m_frustum_near_far.x, m_frustum_near_far.y, m_backBuffer.get());

			for (int f = 0; f < faceNum; f += PIPELINE_BATCH_SIZE)
			{
				int startIndex = f;
				int overIndex = glm::min(f + PIPELINE_BATCH_SIZE, faceNum);
				tbb::parallel_pipeline(ntokens, //Number of tokens
					//Note: Vertex shader and rasterization could be parallelized
					tbb::make_filter<void, int>(executeMopde,
						TBBVertexRastFilter(PIPELINE_BATCH_SIZE, startIndex, overIndex, drawCall, fragmentCache)) &
					//Note: Fragment shaders between different faces could parallelized
					//      because a mutex lock for framebuffer could avoid conflicts
					tbb::make_filter<int, void>(executeMopde,
						TBBFragmentFilter(PIPELINE_BATCH_SIZE, drawCall, fragmentCache, framebufferMutex)));
			}

		}

		return 0;
	}

	unsigned char* TRRenderer::commitRenderedColorBuffer()
	{
		const auto &pixelBuffer = m_frontBuffer->getColorBuffer();
		parallelFor((size_t)0, (size_t)(m_frontBuffer->getWidth() * m_frontBuffer->getHeight()), [&](const size_t &index)
		{
			const auto &pixel = pixelBuffer[index];
			m_renderedImg[index * 3 + 0] = pixel[0][0];
			m_renderedImg[index * 3 + 1] = pixel[0][1];
			m_renderedImg[index * 3 + 2] = pixel[0][2];
		});
		return m_renderedImg.data();
	}

	std::vector<TRShadingPipeline::VertexData> TRRenderer::clipingSutherlandHodgeman(
		const TRShadingPipeline::VertexData &v0,
		const TRShadingPipeline::VertexData &v1,
		const TRShadingPipeline::VertexData &v2,
		const float &near,
		const float &far)
	{
		//Clipping in the homogeneous clipping space
		//Refs:
		//https://fabiensanglard.net/polygon_codec/clippingdocument/Clipping.pdf
		//https://fabiensanglard.net/polygon_codec/

		//Optimization: complete outside or complete inside
		//Note: in the following situation, we could return the answer without complicate cliping,
		//      and this optimization should be very important.
		{
			auto isPointInsideInClipingFrustum = [](const glm::vec4 &p, const float &near, const float &far) -> bool
			{
				return (p.x <= p.w && p.x >= -p.w) && (p.y <= p.w && p.y >= -p.w)
					&& (p.z <= p.w && p.z >= -p.w) && (p.w <= far && p.w >= near);
			};

			//Totally inside
			if (isPointInsideInClipingFrustum(v0.cpos, near, far) &&
				isPointInsideInClipingFrustum(v1.cpos, near, far) &&
				isPointInsideInClipingFrustum(v2.cpos, near, far))
			{
				return { v0,v1,v2 };
			}

			//Totally outside
			if (v0.cpos.w < near && v1.cpos.w < near && v2.cpos.w < near)
				return{};
			if (v0.cpos.w > far && v1.cpos.w > far && v2.cpos.w > far)
				return{};
			if (v0.cpos.x > v0.cpos.w && v1.cpos.x > v1.cpos.w && v2.cpos.x > v2.cpos.w)
				return{};
			if (v0.cpos.x < -v0.cpos.w && v1.cpos.x < -v1.cpos.w && v2.cpos.x < -v2.cpos.w)
				return{};
			if (v0.cpos.y > v0.cpos.w && v1.cpos.y > v1.cpos.w && v2.cpos.y > v2.cpos.w)
				return{};
			if (v0.cpos.y < -v0.cpos.w && v1.cpos.y < -v1.cpos.w && v2.cpos.y < -v2.cpos.w)
				return{};
			if (v0.cpos.z > v0.cpos.w && v1.cpos.z > v1.cpos.w && v2.cpos.z > v2.cpos.w)
				return{};
			if (v0.cpos.z < -v0.cpos.w && v1.cpos.z < -v1.cpos.w && v2.cpos.z < -v2.cpos.w)
				return{};
		}

		std::vector<TRShadingPipeline::VertexData> inside_vertices;
		std::vector<TRShadingPipeline::VertexData> tmp = { v0, v1, v2 };
		enum Axis { X = 0, Y = 1, Z = 2 };

		//w=x plane & w=-x plane
		{
			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::X, +1);
			tmp = inside_vertices;

			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::X, -1);
			tmp = inside_vertices;
		}

		//w=y plane & w=-y plane
		{
			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::Y, +1);
			tmp = inside_vertices;

			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::Y, -1);
			tmp = inside_vertices;
		}

		//w=z plane & w=-z plane
		{
			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::Z, +1);
			tmp = inside_vertices;

			inside_vertices = clipingSutherlandHodgeman_aux(tmp, Axis::Z, -1);
			tmp = inside_vertices;
		}

		//w=1e-5 plane
		{
			inside_vertices = {};
			int num_verts = tmp.size();
			constexpr float w_clipping_plane = 1e-5;
			for (int i = 0; i < num_verts; ++i)
			{
				const auto &beg_vert = tmp[(i - 1 + num_verts) % num_verts];
				const auto &end_vert = tmp[i];
				float beg_is_inside = (beg_vert.cpos.w < w_clipping_plane) ? -1 : 1;
				float end_is_inside = (end_vert.cpos.w < w_clipping_plane) ? -1 : 1;
				//One of them is outside
				if (beg_is_inside * end_is_inside < 0)
				{
					// t = (w_clipping_plane-w1)/((w1-w2)
					float t = (w_clipping_plane - beg_vert.cpos.w) / (beg_vert.cpos.w - end_vert.cpos.w);
					auto intersected_vert = TRShadingPipeline::VertexData::lerp(beg_vert, end_vert, t);
					inside_vertices.push_back(intersected_vert);
				}
				//If current vertices is inside
				if (end_is_inside > 0)
				{
					inside_vertices.push_back(end_vert);
				}
			}
		}

		return inside_vertices;
	}

	std::vector<TRShadingPipeline::VertexData> TRRenderer::clipingSutherlandHodgeman_aux(
		const std::vector<TRShadingPipeline::VertexData> &polygon,
		const int &axis,
		const int &side)
	{
		std::vector<TRShadingPipeline::VertexData> inside_polygon;

		int num_verts = polygon.size();
		for (int i = 0; i < num_verts; ++i)
		{
			const auto &beg_vert = polygon[(i - 1 + num_verts) % num_verts];
			const auto &end_vert = polygon[i];
			char beg_is_inside = ((side * (beg_vert.cpos[axis]) <= beg_vert.cpos.w) ? 1 : -1);
			char end_is_inside = ((side * (end_vert.cpos[axis]) <= end_vert.cpos.w) ? 1 : -1);
			//One of them is outside
			if (beg_is_inside * end_is_inside < 0)
			{
				// t = (w1 - y1)/((w1-y1)-(w2-y2))
				float t = (beg_vert.cpos.w - side * beg_vert.cpos[axis])
					/ ((beg_vert.cpos.w - side * beg_vert.cpos[axis]) - (end_vert.cpos.w - side * end_vert.cpos[axis]));
				auto intersected_vert = TRShadingPipeline::VertexData::lerp(beg_vert, end_vert, t);
				inside_polygon.push_back(intersected_vert);
			}
			//If current vertices is inside
			if (end_is_inside > 0)
			{
				inside_polygon.push_back(end_vert);
			}
		}
		return inside_polygon;
	}

}
