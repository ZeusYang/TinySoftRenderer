#ifndef TRSHADING_STATE_H
#define TRSHADING_STATE_H

#include "glm/glm.hpp"

namespace TinyRenderer
{

	//Texture warping mode
	enum TRTextureWarpMode
	{
		TR_REPEAT,
		TR_MIRRORED_REPEAT,
		TR_CLAMP_TO_EDGE	
	};

	//Texture filtering mode
	enum TRTextureFilterMode
	{
		TR_NEAREST,
		TR_LINEAR
	};

	//Polygon mode
	enum TRPolygonMode
	{
		TR_TRIANGLE_WIRE,
		TR_TRIANGLE_FILL
	};

	//Cull back face mode
	enum TRCullFaceMode
	{
		TR_CULL_DISABLE,
		TR_CULL_FRONT,
		TR_CULL_BACK
	};

	enum TRDepthTestMode
	{
		TR_DEPTH_TEST_DISABLE,
		TR_DEPTH_TEST_ENABLE
	};

	enum TRDepthWriteMode
	{
		TR_DEPTH_WRITE_DISABLE,
		TR_DEPTH_WRITE_ENABLE
	};

	enum TRLightingMode
	{
		TR_LIGHTING_DISABLE,
		TR_LIGHTING_ENABLE
	};

	enum TRAlphaBlendingMode
	{
		TR_ALPHA_DISABLE,
		TR_ALPHA_BLENDING,
		TR_ALPHA_TO_COVERAGE
	};

	class TRShadingState
	{
	public:
		TRCullFaceMode trCullFaceMode		 = TRCullFaceMode::TR_CULL_BACK;
		TRDepthTestMode trDepthTestMode		 = TRDepthTestMode::TR_DEPTH_TEST_ENABLE;
		TRDepthWriteMode trDepthWriteMode	 = TRDepthWriteMode::TR_DEPTH_WRITE_ENABLE;
		TRAlphaBlendingMode trAlphaBlendMode = TRAlphaBlendingMode::TR_ALPHA_DISABLE;
	};

}

#endif