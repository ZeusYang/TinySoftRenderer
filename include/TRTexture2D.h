#ifndef TRTEXTURE_2D_H
#define TRTEXTURE_2D_H

#include <string>
#include <vector>
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

		bool isGeneratedMipmap() const { return m_generateMipmap; }
		int getWidth() const { return m_texHolders[0]->getWidth(); }
		int getHeight() const { return m_texHolders[0]->getHeight(); }

		//Sampling options setting
		void setWarpingMode(TRTextureWarpMode mode);
		void setFilteringMode(TRTextureFilterMode mode);

		bool loadTextureFromFile(
			const std::string &filepath,
			TRTextureWarpMode warpMode = TRTextureWarpMode::TR_REPEAT,
			TRTextureFilterMode filterMode = TRTextureFilterMode::TR_LINEAR);

		//Sampling according to the given uv coordinate
		glm::vec4 sample(const glm::vec2 &uv, const float &level = 0.0f) const;

	private:
		//Auxiliary functions
		void readPixel(const std::uint16_t &u, const std::uint16_t &v, unsigned char &r, 
			unsigned char &g, unsigned char &b, unsigned char &a, const int level = 0) const;

		void generateMipmap(unsigned char *pixels, int width, int height, int channel);

	private:
		bool m_generateMipmap = false;
		std::vector<TRTextureHolder::ptr> m_texHolders;

		TRTextureWarpMode m_warpMode;
		TRTextureFilterMode m_filteringMode;

		friend class TRTexture2DSampler;
	};

	class TRTexture2DSampler final
	{
	public:

		//Sampling algorithm
		static glm::vec4 textureSamplingNearest(TRTextureHolder::ptr texture, glm::vec2 uv);
		static glm::vec4 textureSamplingBilinear(TRTextureHolder::ptr texture, glm::vec2 uv);
	};
}

#endif