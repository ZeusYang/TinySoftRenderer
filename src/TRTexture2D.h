#ifndef TRTEXTURE_2D_H
#define TRTEXTURE_2D_H

#include <string>
#include <memory>

#include "glm/glm.hpp"

#include "TRShadingState.h"

namespace TinyRenderer
{
	class TRTexture2D final
	{
	public:
		typedef std::shared_ptr<TRTexture2D> ptr;

		TRTexture2D();
		TRTexture2D(bool generatedMipmap);
		~TRTexture2D();

		//Sampling options setting
		void setWarpingMode(TRTextureWarpMode mode);
		void setFilteringMode(TRTextureFilterMode mode);

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }
		int getChannel() const { return m_channel; }

		bool loadTextureFromFile(
			const std::string &filepath,
			TRTextureWarpMode warpMode = TRTextureWarpMode::TR_CLAMP_TO_EDGE,
			TRTextureFilterMode filterMode = TRTextureFilterMode::TR_LINEAR);

		//Sampling according to the given uv coordinate
		glm::vec4 sample(const glm::vec2 &uv) const;

	private:
		//Auxiliary functions
		void readPixel(int u, int v, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a) const;
		void freeLoadedImage();

	private:
		int m_width, m_height, m_channel;
		unsigned char *m_pixels;
		bool m_mipmap = false;

		TRTextureWarpMode m_warp_mode;
		TRTextureFilterMode m_filtering_mode;

		friend class TRTexture2DSampler;
	};

	class TRTexture2DSampler final
	{
	public:

		//Sampling algorithm
		static glm::vec4 textureSampling_nearest(const TRTexture2D &texture, glm::vec2 uv);
		static glm::vec4 textureSampling_bilinear(const TRTexture2D &texture, glm::vec2 uv);
	};
}

#endif