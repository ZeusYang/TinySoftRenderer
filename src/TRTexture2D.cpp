#include "TRTexture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace TinyRenderer
{
	//----------------------------------------------TRTexture2D----------------------------------------------

	TRTexture2D::TRTexture2D() : 
		m_width(0), m_height(0), m_channel(0), m_texHolder(nullptr), m_mipmap(false),
		m_warp_mode(TRTextureWarpMode::TR_REPEAT),
		m_filtering_mode(TRTextureFilterMode::TR_NEAREST) {}
	
	TRTexture2D::TRTexture2D(bool generatedMipmap) : 
		m_width(0), m_height(0), m_channel(0), m_texHolder(nullptr), m_mipmap(generatedMipmap),
		m_warp_mode(TRTextureWarpMode::TR_REPEAT),
		m_filtering_mode(TRTextureFilterMode::TR_NEAREST) {}

	void TRTexture2D::setWarpingMode(TRTextureWarpMode mode) { m_warp_mode = mode; }
	void TRTexture2D::setFilteringMode(TRTextureFilterMode mode) { m_filtering_mode = mode; }

	bool TRTexture2D::loadTextureFromFile(
		const std::string &filepath,
		TRTextureWarpMode warpMode,
		TRTextureFilterMode filterMode)
	{
		m_warp_mode = warpMode;
		m_filtering_mode = filterMode;

		unsigned char *pixels = nullptr;

		//Load image from given file using stb_image.h
		//Refs: https://github.com/nothings/stb
		{
			stbi_set_flip_vertically_on_load(true);
			pixels = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channel, 0);

			if (pixels == nullptr)
			{
				std::cerr << "Failed to load image from " << filepath << std::endl;
				exit(1);
			}

			if (m_width >= 65536 || m_height >= 65536)
			{
				std::cerr << "The image " << filepath << " is too large to load\n";
				exit(1);
			}
		}

		//Load the texture according to the meomory layout
		//m_texHolder = std::make_shared<TRLinearTextureHolder>(pixels, m_width, m_height, m_channel);
		//m_texHolder = std::make_shared<TRTilingTextureHolder>(pixels, m_width, m_height, m_channel);
		m_texHolder = std::make_shared<TRZCurveTilingTextureHolder>(pixels, m_width, m_height, m_channel);

		//Generate resolution pyramid for mipmap
		if (m_mipmap)
		{

		}

		stbi_image_free(pixels);

		return true;
	}

	void TRTexture2D::readPixel(
		const std::uint16_t &u, 
		const std::uint16_t &v,
		unsigned char &r, 
		unsigned char &g, 
		unsigned char &b, 
		unsigned char &a) const
	{
		//Please make sure that loadTextureFromFile() had been called.
		//Note: guarantee that u and v are in [0,width-1],[0,height-1] respectively.
		std::uint32_t texel = m_texHolder->read(u, v);
		r = (texel >> 24) & 0xFF;
		g = (texel >> 16) & 0xFF;
		b = (texel >>  8) & 0xFF;
		a = (texel >>  0) & 0xFF;
	}

	glm::vec4 TRTexture2D::sample(const glm::vec2 &uv) const
	{
		//Perform sampling procedure
		//Note: return texel that ranges from 0.0f to 1.0f instead of [0,255]

		float u = uv.x, v = uv.y;
		//Texture warpping mode
		{
			if (u < 0 || u > 1.0f)
			{
				switch (m_warp_mode)
				{
				case TRTextureWarpMode::TR_REPEAT:
					u = (u > 0) ? (u - (int)u) : (1.0f - ((int)u - u));
					break;
				case TRTextureWarpMode::TR_MIRRORED_REPEAT:
					u = (u > 0) ? (1.0f - (u - (int)u)) : ((int)u - u);
					break;
				case TRTextureWarpMode::TR_CLAMP_TO_EDGE:
					u = (u < 0) ? 0 : 1.0f;
					break;
				default:
					u = (u < 0) ? 0 : 1.0f;
					break;
				}
			}

			if (v < 0 || v > 1.0f)
			{
				switch (m_warp_mode)
				{
				case TRTextureWarpMode::TR_REPEAT:
					v = (v > 0) ? (v - (int)v) : (1.0f - ((int)v - v));
					break;
				case TRTextureWarpMode::TR_MIRRORED_REPEAT:
					v = (v > 0) ? (1.0f - (v - (int)v)) : ((int)v - v);
					break;
				case TRTextureWarpMode::TR_CLAMP_TO_EDGE:
					v = (v < 0) ? 0 : 1.0f;
					break;
				default:
					v = (v < 0) ? 0 : 1.0f;
					break;
				}
			}
		}

		glm::vec4 texel(1.0f);
		switch (m_filtering_mode)
		{
		case TRTextureFilterMode::TR_NEAREST:
			texel = TRTexture2DSampler::textureSampling_nearest(*this, glm::vec2(u,v));
			break;
		case TRTextureFilterMode::TR_LINEAR:
			texel = TRTexture2DSampler::textureSampling_bilinear(*this, glm::vec2(u,v));
			break;
		default:
			break;
		}

		return texel;
	}

	//----------------------------------------------TRTexture2DSampler----------------------------------------------

	glm::vec4 TRTexture2DSampler::textureSampling_nearest(const TRTexture2D &texture, glm::vec2 uv)
	{
		//Perform nearest sampling procedure
		unsigned char r, g, b, a = 255;
		texture.readPixel(
			(int)(uv.x * (texture.getWidth() - 1)  + 0.5f), //Rounding
			(int)(uv.y * (texture.getHeight() - 1) + 0.5f), //Rounding
			r, g, b, a);

		constexpr float denom = 1.0f / 255.0f;
		return glm::vec4(r, g, b, a) * denom;
	}

	glm::vec4 TRTexture2DSampler::textureSampling_bilinear(const TRTexture2D &texture, glm::vec2 uv)
	{
		//Perform bilinear sampling procedure

		const auto &w = texture.getWidth();
		const auto &h = texture.getHeight();

		float fx = (uv.x * (w- 1)), fy = (uv.y * (h - 1));
		int ix = (int)fx, iy = (int)fy;
		float frac_x = fx - ix, frac_y = fy - iy;

		/*********************
		 *   p2--p3
		 *   |   |
		 *   p0--p1 
		 * Note: p0 is (ix,iy)
		 ********************/
		static constexpr float denom = 1.0f / 255.0f;
		unsigned char r, g, b, a = 255;
		
		//p0
		texture.readPixel(ix, iy, r, g, b, a);
		glm::vec4 p0(r, g, b, a);

		//p1
		texture.readPixel((ix + 1 >= w) ? ix : (ix + 1), iy, r, g, b, a);
		glm::vec4 p1(r, g, b, a);

		//p2
		texture.readPixel(ix, (iy + 1 >= h) ? iy : (iy + 1), r, g, b, a);
		glm::vec4 p2(r, g, b, a);

		//p3
		texture.readPixel((ix + 1 >= w) ? ix : (ix + 1), (iy + 1 >= h) ? iy : (iy + 1), r, g, b, a);
		glm::vec4 p3(r, g, b, a);

		return ((1.0f - frac_x) * (1.0f - frac_y) * p0 + frac_x * (1.0f - frac_y) * p1 +
			   (1.0f - frac_x) * frac_y * p2 + frac_x * frac_y * p3) * denom;
	}
}