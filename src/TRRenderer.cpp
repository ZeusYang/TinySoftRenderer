#include "TRRenderer.h"

#include "glm/gtc/matrix_transform.hpp"

#include "TRShadingPipeline.h"
#include "TRShaderProgram.h"
#include "TRMathUtils.h"
#include "TRParallelWrapper.h"

#include "tbb/parallel_pipeline.h"
#include "tbb/task_arena.h"

#include <mutex>
#include <atomic>
#include <thread>


namespace TinyRenderer
{
	using MutexType = tbb::spin_mutex;				//TBB thread mutex type
	static constexpr int PIPELINE_BATCH_SIZE = 512; //The number of faces processed for each batch

	//The cache for rasterized results. For example: the face i -> FragmentCache[i]
	using FragmentCache = std::array<std::vector<TRShadingPipeline::QuadFragments>, PIPELINE_BATCH_SIZE>;

	//----------------------------------------------DrawcallSetting----------------------------------------------
	//Draw call setting which would be utilized in shading parallel pipeline 
	class DrawcallSetting final
	{
	public:

		const TRVertexBuffer &m_vertexBuffer;			//Vertex data buffer
		const TRIndexBuffer  &m_indexBuffer;			//Index data buffer
		TRShadingPipeline *m_shaderHandler;			//Shader handler
		const TRShadingState &m_shadingState;			//Shading state
		const glm::mat4 &m_viewportMatrix;			//Viewport transformation matrix
		float m_near, m_far;							//Near plane and far plane of frustum
		TRFrameBuffer *m_frameBuffer;					//Framebuffer 

		explicit DrawcallSetting(const TRVertexBuffer &vbo, const TRIndexBuffer &ibo, TRShadingPipeline *handler,
			const TRShadingState &state, const glm::mat4 &viewportMat, float np, float fp, TRFrameBuffer *fb)
			: m_vertexBuffer(vbo), m_indexBuffer(ibo), m_shaderHandler(handler), m_shadingState(state),
			m_viewportMatrix(viewportMat), m_near(np), m_far(fp), m_frameBuffer(fb) {}
	};

	//----------------------------------------------FramebufferMutex----------------------------------------------
	//Each point (i,j) of framebuffer have its own mutex lock for avoiding accessing conflict among different threads
	class FramebufferMutex final
	{
	public:
		FramebufferMutex(int width, int height)
			: m_width(width), m_height(height)
		{
			m_mutexBuffer.resize(width * height, nullptr);
			for (int i = 0; i < width * height; ++i)
			{
				m_mutexBuffer[i] = new MutexType();
			}
		}

		~FramebufferMutex()
		{
			for (auto &mutex : m_mutexBuffer)
			{
				delete mutex;
				mutex = nullptr;
			}
		}

		MutexType &getLocker(const int &x, const int &y)
		{
			return *m_mutexBuffer[y * m_width + x];
		}

	public:
		using MutexBuffer = std::vector<MutexType*>;
		int m_width, m_height;
		MutexBuffer m_mutexBuffer;
	};

	//----------------------------------------------TBBVertexRastFilter----------------------------------------------
	//Vertex transformation, cliping, culling and rasterization.
	class TBBVertexRastFilter final
	{
	public:
		explicit TBBVertexRastFilter(int bs, int startIndex, int overIndex, const DrawcallSetting &drawcall,
			FragmentCache &cache) : m_batchSize(bs), m_startIndex(startIndex), m_overIndex(overIndex),
			m_drawCall(drawcall), m_fragmentCache(cache)
		{
			m_currIndex.store(startIndex);
		}

		int operator()(tbb::flow_control &fc) const
		{
			//Note: process the faces in [startIndex, overIndex) parallely
			int faceIndex = 0;
			//Fetch the face index that needs to be processed.
			{
				//Note: an atomic add herein for excusively accesing the face
				if ((faceIndex = m_currIndex.fetch_add(1)) >= m_overIndex)
				{
					fc.stop();//Exceed range, stop the processing flow
					return -1;
				}
			}

			//The fragment cache index
			int order = faceIndex - m_startIndex;
			faceIndex *= 3;

			TRShadingPipeline::VertexData v[3];
			const auto &indexBuffer = m_drawCall.m_indexBuffer;
			const auto &vertexBuffer = m_drawCall.m_vertexBuffer;
#pragma unroll 3
			for (int i = 0; i < 3; ++i)
			{
				v[i].m_pos = vertexBuffer[indexBuffer[faceIndex + i]].m_vpositions;
				v[i].m_nor = vertexBuffer[indexBuffer[faceIndex + i]].m_vnormals;
				v[i].m_tex = vertexBuffer[indexBuffer[faceIndex + i]].m_vtexcoords;
				v[i].m_tbn[0] = vertexBuffer[indexBuffer[faceIndex + i]].m_vtangent;
				v[i].m_tbn[1] = vertexBuffer[indexBuffer[faceIndex + i]].m_vbitangent;
			}

			//Vertex shader stage
			m_drawCall.m_shaderHandler->vertexShader(v[0]);
			m_drawCall.m_shaderHandler->vertexShader(v[1]);
			m_drawCall.m_shaderHandler->vertexShader(v[2]);

			//Homogeneous space cliping
			std::vector<TRShadingPipeline::VertexData> clipped_vertices;
			clipped_vertices = TRRenderer::clipingSutherlandHodgeman(v[0], v[1], v[2], m_drawCall.m_near, m_drawCall.m_far);
			if (clipped_vertices.empty())
			{
				return -1; //Totally outside
			}

			//Perspective division: from clip space -> ndc space
			for (auto &vert : clipped_vertices)
			{
				TRShadingPipeline::VertexData::prePerspCorrection(vert);
				vert.m_cpos *= vert.m_rhw;
			}

			int num_verts = clipped_vertices.size();
			for (int i = 0; i < num_verts - 2; ++i)
			{
				//Triangle assembly
				TRShadingPipeline::VertexData vert[3] = { clipped_vertices[0], clipped_vertices[i + 1], clipped_vertices[i + 2] };

				//Transform to screen space
				vert[0].m_spos = glm::ivec2(m_drawCall.m_viewportMatrix * vert[0].m_cpos + glm::vec4(0.5f));
				vert[1].m_spos = glm::ivec2(m_drawCall.m_viewportMatrix * vert[1].m_cpos + glm::vec4(0.5f));
				vert[2].m_spos = glm::ivec2(m_drawCall.m_viewportMatrix * vert[2].m_cpos + glm::vec4(0.5f));

				//Backface culling
				if (shouldCulled(vert[0].m_spos, vert[1].m_spos, vert[2].m_spos, m_drawCall.m_shadingState.m_trCullFaceMode))
				{
					continue;
				}

				//Rasterization
				TRShadingPipeline::rasterizeFillEdgeFunction(vert[0], vert[1], vert[2],
					m_drawCall.m_frameBuffer->getWidth(), m_drawCall.m_frameBuffer->getHeight(), m_fragmentCache[order]);
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
		int m_batchSize;
		const int m_startIndex;
		const int m_overIndex;
		const DrawcallSetting &m_drawCall;

		//this is for excessively accessing to face among threads
		static std::atomic<int> m_currIndex;

		FragmentCache &m_fragmentCache;
	};

	std::atomic<int> TBBVertexRastFilter::m_currIndex;

	//----------------------------------------------TBBFragmentFilter----------------------------------------------
	//Fragment shader execution
	class TBBFragmentFilter final
	{
	public:
		explicit TBBFragmentFilter(int bs, const DrawcallSetting &drawcall, FragmentCache &cache, FramebufferMutex &fbMutex)
			: m_batchSize(bs), m_drawCall(drawcall), m_fragmentCache(cache), m_framebufferMutex(fbMutex) {}

		void operator()(int index) const
		{
			//No fragments
			if (index == -1 || m_fragmentCache[index].empty())
				return;

			//Fragment shader & Depth testing
			auto fragment_func = [&](TRShadingPipeline::FragmentData &fragment, const glm::vec2 &dUVdx, const glm::vec2 &dUVdy)
			{
				//Note: spos.x equals -1 -> invalid fragment
				if (fragment.m_spos.x == -1)
					return;

				auto &coverage = fragment.m_coverage;
				const auto &fragCoord = fragment.m_spos;
				auto &framebuffer = m_drawCall.m_frameBuffer;
				const auto &shadingState = m_drawCall.m_shadingState;

				//A mutex locker herein for (x,y) to prevent from simultanenously accessing depth buffer at the same place
				MutexType::scoped_lock lock(m_framebufferMutex.getLocker(fragCoord.x, fragCoord.y));

				const int samplingNum = TRMaskPixelSampler::getSamplingNum();

				int num_failed = 0;
				//Depth testing for each sampling point (Early Z strategy herein)
				if (shadingState.m_trDepthTestMode == TRDepthTestMode::TR_DEPTH_TEST_ENABLE)
				{
					const auto &coverageDepth = fragment.m_coverageDepth;
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
				m_drawCall.m_shaderHandler->fragmentShader(fragment, fragColor, dUVdx, dUVdy);

				//Alpha to coverage
				//Note: alpha to coverage only work with MSAA
				//Refs: http://www.zwqxin.com/archives/opengl/talk-about-alpha-to-coverage.html
				if (shadingState.m_trAlphaBlendMode == TRAlphaBlendingMode::TR_ALPHA_TO_COVERAGE && samplingNum >= 4)
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
				switch (shadingState.m_trAlphaBlendMode)
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
					break;
				}

				//Depth writing
				if (shadingState.m_trDepthWriteMode == TRDepthWriteMode::TR_DEPTH_WRITE_ENABLE)
				{
					framebuffer->writeDepthWithMask(fragCoord.x, fragCoord.y, fragment.m_coverageDepth, coverage);
				}
				
			};

			//Note: 2x2 fragment block as an execution unit for calculating dFdx, dFdy.
			parallelFor((size_t)0, (size_t)m_fragmentCache[index].size(), [&](const size_t &f)
			{
				auto &block = m_fragmentCache[index][f];

				//Perspective correction restore
				block.aftPrespCorrectionForBlocks();

				//Calculate dUVdx, dUVdy for mipmap
				glm::vec2 dUVdx(block.dUdx(), block.dVdx());
				glm::vec2 dUVdy(block.dUdy(), block.dVdy());

				fragment_func(block.m_fragments[0], dUVdx, dUVdy);
				fragment_func(block.m_fragments[1], dUVdx, dUVdy);
				fragment_func(block.m_fragments[2], dUVdx, dUVdy);
				fragment_func(block.m_fragments[3], dUVdx, dUVdy);

			}, TRExecutionPolicy::TR_PARALLEL);

			m_fragmentCache[index].clear();
		}

	private:
		int m_batchSize;
		const DrawcallSetting &m_drawCall;
		FragmentCache &m_fragmentCache;
		FramebufferMutex &m_framebufferMutex;
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
		if (m_shaderHandler == nullptr)
			return;
		m_shaderHandler->setViewerPos(viewer);
	}

	int TRRenderer::addLightSource(TRLight::ptr lightSource) { return TRShadingPipeline::addLight(lightSource); }

	TRLight::ptr TRRenderer::getLightSource(const int &index) { return TRShadingPipeline::getLight(index); }

	void TRRenderer::setExposure(const float &exposure) { TRShadingPipeline::setExposure(exposure); }

	unsigned int TRRenderer::renderAllDrawableMeshes()
	{
		if (m_shaderHandler == nullptr)
		{
			m_shaderHandler = std::make_shared<TR3DShadingPipeline>();
		}

		//Load the matrices
		m_shaderHandler->setModelMatrix(m_modelMatrix);
		m_shaderHandler->setViewProjectMatrix(m_projectMatrix * m_viewMatrix);

		//Draw a mesh step by step
		unsigned int num_triangles = 0;

		for (size_t m = 0; m < m_drawableMeshes.size(); ++m)
		{
			num_triangles += renderDrawableMesh(m);
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

		unsigned int num_triangles = 0;
		const auto &drawable = m_drawableMeshes[index];
		const auto &submeshes = drawable->getDrawableSubMeshes();

		//Configuration
		m_shadingState.m_trCullFaceMode = drawable->getCullfaceMode();
		m_shadingState.m_trDepthTestMode = drawable->getDepthtestMode();
		m_shadingState.m_trDepthWriteMode = drawable->getDepthwriteMode();
		m_shadingState.m_trAlphaBlendMode = drawable->getAlphablendMode();

		//Setup the shading options
		m_shaderHandler->setModelMatrix(drawable->getModelMatrix());
		m_shaderHandler->setLightingEnable(drawable->getLightingMode() == TRLightingMode::TR_LIGHTING_ENABLE);
		m_shaderHandler->setAmbientCoef(drawable->getAmbientCoff());
		m_shaderHandler->setDiffuseCoef(drawable->getDiffuseCoff());
		m_shaderHandler->setSpecularCoef(drawable->getSpecularCoff());
		m_shaderHandler->setEmissionColor(drawable->getEmissionCoff());
		m_shaderHandler->setShininess(drawable->getSpecularExponent());
		m_shaderHandler->setTransparency(drawable->getTransparency());

		//Note: For those drawables which need the alpha blending, we should make sure the faces rendered in a fixed order 
		tbb::filter_mode executeMopde = m_shadingState.m_trAlphaBlendMode == TRAlphaBlendingMode::TR_ALPHA_DISABLE ?
			tbb::filter_mode::parallel : tbb::filter_mode::serial_in_order;

		//Setting for drawcall
		static int ntokens = tbb::this_task_arena::max_concurrency() * 128;
		static FragmentCache fragmentCache;
		static FramebufferMutex framebufferMutex(m_backBuffer->getWidth(), m_backBuffer->getHeight());

		for (size_t s = 0; s < submeshes.size(); ++s)
		{
			const auto &submesh = submeshes[s];
			int faceNum = submesh.getIndices().size() / 3;
			num_triangles += faceNum;

			//Texture setting
			m_shaderHandler->setDiffuseTexId(submesh.getDiffuseMapTexId());
			m_shaderHandler->setSpecularTexId(submesh.getSpecularMapTexId());
			m_shaderHandler->setNormalTexId(submesh.getNormalMapTexId());
			m_shaderHandler->setGlowTexId(submesh.getGlowMapTexId());

			//Draw call setting
			DrawcallSetting drawCall(submesh.getVertices(), submesh.getIndices(), m_shaderHandler.get(),
				m_shadingState, m_viewportMatrix, m_frustumNearFar.x, m_frustumNearFar.y, m_backBuffer.get());

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

		return num_triangles;
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
			if (isPointInsideInClipingFrustum(v0.m_cpos, near, far) &&
				isPointInsideInClipingFrustum(v1.m_cpos, near, far) &&
				isPointInsideInClipingFrustum(v2.m_cpos, near, far))
			{
				return { v0,v1,v2 };
			}

			//Totally outside
			if (v0.m_cpos.w < near && v1.m_cpos.w < near && v2.m_cpos.w < near)
				return{};
			if (v0.m_cpos.w > far && v1.m_cpos.w > far && v2.m_cpos.w > far)
				return{};
			if (v0.m_cpos.x > v0.m_cpos.w && v1.m_cpos.x > v1.m_cpos.w && v2.m_cpos.x > v2.m_cpos.w)
				return{};
			if (v0.m_cpos.x < -v0.m_cpos.w && v1.m_cpos.x < -v1.m_cpos.w && v2.m_cpos.x < -v2.m_cpos.w)
				return{};
			if (v0.m_cpos.y > v0.m_cpos.w && v1.m_cpos.y > v1.m_cpos.w && v2.m_cpos.y > v2.m_cpos.w)
				return{};
			if (v0.m_cpos.y < -v0.m_cpos.w && v1.m_cpos.y < -v1.m_cpos.w && v2.m_cpos.y < -v2.m_cpos.w)
				return{};
			if (v0.m_cpos.z > v0.m_cpos.w && v1.m_cpos.z > v1.m_cpos.w && v2.m_cpos.z > v2.m_cpos.w)
				return{};
			if (v0.m_cpos.z < -v0.m_cpos.w && v1.m_cpos.z < -v1.m_cpos.w && v2.m_cpos.z < -v2.m_cpos.w)
				return{};
		}

		std::vector<TRShadingPipeline::VertexData> insideVertices;
		std::vector<TRShadingPipeline::VertexData> tmp = { v0, v1, v2 };
		enum Axis { X = 0, Y = 1, Z = 2 };

		//w=x plane & w=-x plane
		{
			insideVertices = clipingSutherlandHodgemanAux(tmp, Axis::X, +1);
			tmp = insideVertices;

			insideVertices = clipingSutherlandHodgemanAux(tmp, Axis::X, -1);
			tmp = insideVertices;
		}

		//w=y plane & w=-y plane
		{
			insideVertices = clipingSutherlandHodgemanAux(tmp, Axis::Y, +1);
			tmp = insideVertices;

			insideVertices = clipingSutherlandHodgemanAux(tmp, Axis::Y, -1);
			tmp = insideVertices;
		}

		//w=z plane & w=-z plane
		{
			insideVertices = clipingSutherlandHodgemanAux(tmp, Axis::Z, +1);
			tmp = insideVertices;

			insideVertices = clipingSutherlandHodgemanAux(tmp, Axis::Z, -1);
			tmp = insideVertices;
		}

		//w=1e-5 plane
		{
			insideVertices = {};
			int numVerts = tmp.size();
			constexpr float wClippingPlane = 1e-5;
			for (int i = 0; i < numVerts; ++i)
			{
				const auto &begVert = tmp[(i - 1 + numVerts) % numVerts];
				const auto &endVert = tmp[i];
				float begIsInside = (begVert.m_cpos.w < wClippingPlane) ? -1 : 1;
				float endIsInside = (endVert.m_cpos.w < wClippingPlane) ? -1 : 1;
				//One of them is outside
				if (begIsInside * endIsInside < 0)
				{
					// t = (w_clipping_plane-w1)/((w1-w2)
					float t = (wClippingPlane - begVert.m_cpos.w) / (begVert.m_cpos.w - endVert.m_cpos.w);
					auto intersectedVert = TRShadingPipeline::VertexData::lerp(begVert, endVert, t);
					insideVertices.push_back(intersectedVert);
				}
				//If current vertices is inside
				if (endIsInside > 0)
				{
					insideVertices.push_back(endVert);
				}
			}
		}

		return insideVertices;
	}

	std::vector<TRShadingPipeline::VertexData> TRRenderer::clipingSutherlandHodgemanAux(
		const std::vector<TRShadingPipeline::VertexData> &polygon,
		const int &axis,
		const int &side)
	{
		std::vector<TRShadingPipeline::VertexData> insidePolygon;

		int numVerts = polygon.size();
		for (int i = 0; i < numVerts; ++i)
		{
			const auto &begVert = polygon[(i - 1 + numVerts) % numVerts];
			const auto &endVert = polygon[i];
			char begIsInside = ((side * (begVert.m_cpos[axis]) <= begVert.m_cpos.w) ? 1 : -1);
			char endIsInside = ((side * (endVert.m_cpos[axis]) <= endVert.m_cpos.w) ? 1 : -1);
			//One of them is outside
			if (begIsInside * endIsInside < 0)
			{
				// t = (w1 - y1)/((w1-y1)-(w2-y2))
				float t = (begVert.m_cpos.w - side * begVert.m_cpos[axis])
					/ ((begVert.m_cpos.w - side * begVert.m_cpos[axis]) - (endVert.m_cpos.w - side * endVert.m_cpos[axis]));
				auto intersectedVert = TRShadingPipeline::VertexData::lerp(begVert, endVert, t);
				insidePolygon.push_back(intersectedVert);
			}
			//If current vertices is inside
			if (endIsInside > 0)
			{
				insidePolygon.push_back(endVert);
			}
		}
		return insidePolygon;
	}

}
