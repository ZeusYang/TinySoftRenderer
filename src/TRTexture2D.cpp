#include "TRTexture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace TinyRenderer
{
	//----------------------------------------------TRTexture2D----------------------------------------------

	TRTexture2D::TRTexture2D() : 
		m_width(0), m_height(0), m_channel(0), m_pixels(nullptr), m_mipmap(false),
		m_warp_mode(TRTextureWarpMode::TR_REPEAT),
		m_filtering_mode(TRTextureFilterMode::TR_NEAREST) {}
	
	TRTexture2D::TRTexture2D(bool generatedMipmap) : 
		m_width(0), m_height(0), m_channel(0), m_pixels(nullptr), m_mipmap(generatedMipmap), 
		m_warp_mode(TRTextureWarpMode::TR_REPEAT),
		m_filtering_mode(TRTextureFilterMode::TR_NEAREST) {}

	TRTexture2D::~TRTexture2D() { freeLoadedImage(); }

	void TRTexture2D::setWarpingMode(TRTextureWarpMode mode) { m_warp_mode = mode; }

	void TRTexture2D::setFilteringMode(TRTextureFilterMode mode) { m_filtering_mode = mode; }

	bool TRTexture2D::loadTextureFromFile(
		const std::string &filepath,
		TRTextureWarpMode warpMode,
		TRTextureFilterMode filterMode)
	{
		freeLoadedImage();

		m_warp_mode = warpMode;
		m_filtering_mode = filterMode;

		//Load image from given file using stb_image.h
		//Refs: https://github.com/nothings/stb
		{
			stbi_set_flip_vertically_on_load(true);
			m_pixels = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channel, 0);
		}

		if (m_pixels == nullptr)
		{
			std::cerr << "Failed to load image from " << filepath << std::endl;
			exit(1);
		}

		//Generate resolution pyramid for mipmap
		if (m_mipmap)
		{

		}

		return true;
	}

	void TRTexture2D::readPixel(int u, int v, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a) const
	{
		//Handling out of range situation
		{
			if (u < 0 || u >= m_width)
			{
				switch (m_warp_mode)
				{
				case TRTextureWarpMode::TR_REPEAT:
					u = u % m_width;
					break;
				case TRTextureWarpMode::TR_MIRRORED_REPEAT:
					u = m_width - u % m_width;
					break;
				case TRTextureWarpMode::TR_CLAMP_TO_EDGE:
					u = (u < 0) ? 0 : m_width - 1;
					break;
				default:
					u = (u < 0) ? 0 : m_width - 1;
					break;
				}
			}

			if (v < 0 || v >= m_height)
			{
				switch (m_warp_mode)
				{
				case TRTextureWarpMode::TR_REPEAT:
					v = v % m_height;
					break;
				case TRTextureWarpMode::TR_MIRRORED_REPEAT:
					v = m_height - v % m_height;
					break;
				case TRTextureWarpMode::TR_CLAMP_TO_EDGE:
					v = (v < 0) ? 0 : m_height - 1;
					break;
				default:
					v = (v < 0) ? 0 : m_height - 1;
					break;
				}
			}
		}

		int index = (v * m_height + u) * m_channel;
		r = m_pixels[index + 0];
		g = m_pixels[index + 1];
		b = m_pixels[index + 2];
		a = (m_channel >= 4) ? m_pixels[index + 3] : a;

		return;
	}

	void TRTexture2D::freeLoadedImage()
	{
		if (m_pixels != nullptr)
		{
			stbi_image_free(m_pixels);
		}

		m_pixels = nullptr;
		m_width = m_height = m_channel = 0;
	}

	glm::vec4 TRTexture2D::sample(const glm::vec2 &uv) const
	{
		//Perform sampling procedure
		//Note: return texel that ranges from 0.0f to 1.0f instead of [0,255]
		glm::vec4 texel(1.0f);
		switch (m_filtering_mode)
		{
		case TRTextureFilterMode::TR_NEAREST:
			texel = TRTexture2DSampler::textureSampling_nearest(*this, uv);
			break;
		case TRTextureFilterMode::TR_LINEAR:
			texel = TRTexture2DSampler::textureSampling_bilinear(*this, uv);
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
			(int)(uv.x * texture.getWidth()  + 0.5f), //Rounding
			(int)(uv.y * texture.getHeight() + 0.5f), //Rounding
			r, g, b, a);

		constexpr float denom = 1.0f / 255.0f;
		return glm::vec4(r * denom, g * denom, b * denom, a * denom);
	}

	glm::vec4 TRTexture2DSampler::textureSampling_bilinear(const TRTexture2D &texture, glm::vec2 uv)
	{
		//Perform bilinear sampling procedure
		float fx = (uv.x * texture.getWidth());
		float fy = (uv.y * texture.getHeight());
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
		glm::vec4 p0(r * denom, g * denom, b * denom, a *denom);

		//p1
		texture.readPixel(ix + 1, iy, r, g, b, a);
		glm::vec4 p1(r * denom, g * denom, b * denom, a *denom);

		//p2
		texture.readPixel(ix, iy + 1, r, g, b, a);
		glm::vec4 p2(r * denom, g * denom, b * denom, a *denom);

		//p3
		texture.readPixel(ix + 1, iy + 1, r, g, b, a);
		glm::vec4 p3(r * denom, g * denom, b * denom, a *denom);

		return (1.0f - frac_x) * (1.0f - frac_y) * p0 + frac_x * (1.0f - frac_y) * p1 +
			   (1.0f - frac_x) * frac_y * p2 + frac_x * frac_y * p3;
	}
}