#ifndef TRTEXTURE_2D_H
#define TRTEXTURE_2D_H

#include <string>
#include <memory>

#include "glm/glm.hpp"

#include "TRShadingState.h"
#include "TRTextureHolder.h"

namespace TinyRenderer
{
	class TRTexture2D final
	{
	public:
		typedef std::shared_ptr<TRTexture2D> ptr;

		TRTexture2D();
		TRTexture2D(bool generatedMipmap);
		~TRTexture2D() = default;

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }

		//Sampling options setting
		void setWarpingMode(TRTextureWarpMode mode);
		void setFilteringMode(TRTextureFilterMode mode);

		bool loadTextureFromFile(
			const std::string &filepath,
			TRTextureWarpMode warpMode = TRTextureWarpMode::TR_REPEAT,
			TRTextureFilterMode filterMode = TRTextureFilterMode::TR_LINEAR);

		//Sampling according to the given uv coordinate
		glm::vec4 sample(const glm::vec2 &uv) const;

	private:
		//Auxiliary functions
		void readPixel(
			const std::uint16_t &u,
			const std::uint16_t &v, 
			unsigned char &r, 
			unsigned char &g, 
			unsigned char &b, 
			unsigned char &a) const;

	private:
		int m_width, m_height, m_channel;
		TRTextureHolder::ptr m_texHolder;

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