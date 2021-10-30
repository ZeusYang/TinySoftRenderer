#include "TRTexture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "TRParallelWrapper.h"

#include <iostream>

namespace TinyRenderer
{
	//----------------------------------------------TRTexture2D----------------------------------------------

	TRTexture2D::TRTexture2D() :
		m_generateMipmap(false),
		m_warpMode(TRTextureWarpMode::TR_MIRRORED_REPEAT),
		m_filteringMode(TRTextureFilterMode::TR_LINEAR) {}
	
	TRTexture2D::TRTexture2D(bool generatedMipmap) :
		m_generateMipmap(generatedMipmap),
		m_warpMode(TRTextureWarpMode::TR_MIRRORED_REPEAT),
		m_filteringMode(TRTextureFilterMode::TR_LINEAR) {}

	void TRTexture2D::setWarpingMode(TRTextureWarpMode mode) { m_warpMode = mode; }
	void TRTexture2D::setFilteringMode(TRTextureFilterMode mode) { m_filteringMode = mode; }

	bool TRTexture2D::loadTextureFromFile(
		const std::string &filepath,
		TRTextureWarpMode warpMode,
		TRTextureFilterMode filterMode)
	{
		m_warpMode = warpMode;
		m_filteringMode = filterMode;
		std::vector<TRTextureHolder::ptr>().swap(m_texHolders);

		unsigned char *pixels = nullptr;

		//Load image from given file using stb_image.h
		//Refs: https://github.com/nothings/stb
		int width, height, channel;
		{
			//stbi_set_flip_vertically_on_load(true);
			pixels = stbi_load(filepath.c_str(), &width, &height, &channel, 0);

			if (pixels == nullptr)
			{
				std::cerr << "Failed to load image from " << filepath << std::endl;
				exit(1);
			}

			if (width <= 0 || width >= 65536 || height <= 0 || height >= 65536)
			{
				std::cerr << "Invalid size from image: " << filepath << std::endl;
				exit(1);
			}
		}

		//32bpp
		unsigned char *raw = new unsigned char[width * height * 4];
		parallelFor((int)0, (int)(width * height), [&](const int &index)
		{
			unsigned char &r = raw[index * 4 + 0];
			unsigned char &g = raw[index * 4 + 1];
			unsigned char &b = raw[index * 4 + 2];
			unsigned char &a = raw[index * 4 + 3];
			int fromIndex = index * channel;
			switch (channel)
			{
			case 1:
				r = g = b = pixels[fromIndex], a = 255;
				break;
			case 3:
				r = pixels[fromIndex], g = pixels[fromIndex + 1], b = pixels[fromIndex + 2], a = 255;
				break;
			case 4:
				r = pixels[fromIndex], g = pixels[fromIndex + 1], b = pixels[fromIndex + 2], a = pixels[fromIndex + 3];
				break;
			default:
				r = g = b = pixels[fromIndex], a = 255;
				break;
			}
		}, TRExecutionPolicy::TR_PARALLEL);
		channel = 4;
		stbi_image_free(pixels);
		pixels = nullptr;

		//Generate resolution pyramid for mipmap
		if (m_generateMipmap)
		{
			generateMipmap(raw, width, height, channel);
		}
		else
		{
			m_texHolders = { std::make_shared<TRZCurveTilingTextureHolder>(raw, width, height, channel) };
		}

		delete[] raw;

		return true;
	}

	void TRTexture2D::generateMipmap(unsigned char *pixels, int width, int height, int channel)
	{
		unsigned char *rawData = pixels;
		bool reAlloc = false;

		//Find the first greater number which equals to 2^n
		static auto firstGreaterPowOf2 = [](const int &num) -> int
		{
			int base = num - 1;
			base |= (base >> 1);
			base |= (base >> 2);
			base |= (base >> 4);
			base |= (base >> 8);
			base |= (base >> 16);
			return base + 1;
		};
		int nw = firstGreaterPowOf2(width);
		int nh = firstGreaterPowOf2(height);

		//Note: need to make sure that width and height equal to 2^n
		if (nw != width || nh != height || nw != nh)
		{
			//Reallocation for padding to 2^n * 2^n
			nw = glm::max(nw, nh);
			nh = glm::max(nw, nh);
			reAlloc = true;
			rawData = new unsigned char[nw * nh * channel];

			auto readPixels = [&](const int &x, const int &y, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a) -> void
			{
				int tx = (x >= width) ? (width - 1) : x;
				int ty = (y >= height) ? (height - 1) : y;
				r = pixels[(ty*width + tx) * channel + 0];
				g = pixels[(ty*width + tx) * channel + 1];
				b = pixels[(ty*width + tx) * channel + 2];
				a = pixels[(ty*width + tx) * channel + 3];
			};

			parallelFor((int)0, (int)(nw * nh), [&](const int &index) -> void
			{
				float x = (float)(index % nw)/(float)(nw - 1) * (width - 1);
				float y = (float)(index / nw)/(float)(nh - 1) * (height - 1);
				unsigned char r, g, b, a;
				//Binlear interpolation for scaling
				{
					int ix = (int)x, iy = (int)y;
					int fx = x - ix, fy = y - iy;

					unsigned char p[4][4];
					readPixels(ix, iy, p[0][0], p[0][1], p[0][2], p[0][3]);
					readPixels(ix + 1, iy, p[1][0], p[1][1], p[1][2], p[1][3]);
					readPixels(ix, iy + 1, p[2][0], p[2][1], p[2][2], p[2][3]);
					readPixels(ix + 1, iy + 1, p[3][0], p[3][1], p[3][2], p[3][3]);

					float w0 = (1.0f - fx) * (1.0f - fy), w1 = fx * (1.0f - fy);
					float w2 = (1.0f - fx) * fy, w3 = fx * fy;
					r = (unsigned char)(w0 * p[0][0] + w1 * p[1][0] + w2 * p[2][0] + w3 * p[3][0]);
					g = (unsigned char)(w0 * p[0][1] + w1 * p[1][1] + w2 * p[2][1] + w3 * p[3][1]);
					b = (unsigned char)(w0 * p[0][2] + w1 * p[1][2] + w2 * p[2][2] + w3 * p[3][2]);
					a = (unsigned char)(w0 * p[0][3] + w1 * p[1][3] + w2 * p[2][3] + w3 * p[3][3]);
				}
				rawData[index * channel] = r;
				rawData[index * channel + 1] = g; 
				rawData[index * channel + 2] = b;
				rawData[index * channel + 3] = a;
			});
			width = nw;
			height = nh;
			std::cout << "Warning: texture padding to 2^n * 2^n\n";
		}
		
		//First level
		int curW = width, curH = height;
		m_texHolders.push_back(std::make_shared<TRZCurveTilingTextureHolder>(rawData, curW, curH, channel));

		//The rest of levels
		unsigned char *previous = rawData;
		unsigned char *tmpAlloc = new unsigned char[curW * curH * channel];
		unsigned char *current = tmpAlloc;
		while(curW >= 2)
		{
			curW /= 2, curH /= 2;
			parallelFor((int)0, (int)(curW * curH), [&](const int &index)
			{
				int x = index % curW;
				int y = index / curW;
				unsigned char r, g, b, a;
				int destX = 2 * x, destY = 2 * y;
				int target1 = (destY * curW * 2 + destX) * channel;
				int target2 = (destY * curW * 2 + destX + 1) * channel;
				int target3 = ((destY + 1) * curW * 2 + destX) * channel;
				int target4 = ((destY + 1) * curW * 2 + destX + 1) * channel;
				//Box filtering for down-sampling
				r = (previous[target1 + 0] + previous[target2 + 0] + previous[target3 + 0] + previous[target4 + 0]) * 0.25;
				g = (previous[target1 + 1] + previous[target2 + 1] + previous[target3 + 1] + previous[target4 + 1]) * 0.25;
				b = (previous[target1 + 2] + previous[target2 + 2] + previous[target3 + 2] + previous[target4 + 2]) * 0.25;
				a = (previous[target1 + 3] + previous[target2 + 3] + previous[target3 + 3] + previous[target4 + 3]) * 0.25;
				current[index * channel + 0] = r;
				current[index * channel + 1] = g;
				current[index * channel + 2] = b;
				current[index * channel + 3] = a;
			});

			//Note: Tiling and ZCuve mapping are also time-consuming
			//if (curW >= 32)
			//{
			//	m_texHolders.push_back(std::make_shared<TRZCurveTilingTextureHolder>(current, curW, curH, channel));
			//}
			//else
			//{
			//	m_texHolders.push_back(std::make_shared<TRLinearTextureHolder>(current, curW, curH, channel));
			//}
			m_texHolders.push_back(std::make_shared<TRTilingTextureHolder>(current, curW, curH, channel));
			//m_texHolders.push_back(std::make_shared<TRLinearTextureHolder>(current, curW, curH, channel));
			std::swap(current, previous);
		}

		delete[] tmpAlloc;
		if (reAlloc)
		{
			delete[] rawData;
		}
	}

	void TRTexture2D::readPixel(const std::uint16_t &u, const std::uint16_t &v, unsigned char &r, 
		unsigned char &g, unsigned char &b, unsigned char &a, const int level) const
	{
		//Please make sure that loadTextureFromFile() had been called.
		//Note: guarantee that u and v are in [0,width-1],[0,height-1] respectively.
		std::uint32_t texel = m_texHolders[level]->read(u, v);
		r = (texel >> 24) & 0xFF;
		g = (texel >> 16) & 0xFF;
		b = (texel >>  8) & 0xFF;
		a = (texel >>  0) & 0xFF;
	}

	glm::vec4 TRTexture2D::sample(const glm::vec2 &uv, const float &level) const
	{
		//Perform sampling procedure
		//Note: return texel that ranges from 0.0f to 1.0f instead of [0,255]
		float u = uv.x, v = uv.y;
		//Texture warpping mode
		{
			if (u < 0 || u > 1.0f)
			{
				switch (m_warpMode)
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
				switch (m_warpMode)
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
		//No mipmap: just sampling at the first level
		if (!m_generateMipmap)
		{
			switch (m_filteringMode)
			{
			case TRTextureFilterMode::TR_NEAREST:
				texel = TRTexture2DSampler::textureSamplingNearest(m_texHolders[0], glm::vec2(u, v));
				break;
			case TRTextureFilterMode::TR_LINEAR:
				texel = TRTexture2DSampler::textureSamplingBilinear(m_texHolders[0], glm::vec2(u, v));
				break;
			default:
				break;
			}
		}
		//Mipmap: linear interpolation between two levels
		else
		{
			glm::vec4 texel1(1.0f), texel2(1.0f);
			unsigned int level1 = glm::min((unsigned int)level, (unsigned int)m_texHolders.size() - 1);
			unsigned int level2 = glm::min((unsigned int)(level + 1), (unsigned int)m_texHolders.size() - 1);
			switch (m_filteringMode)
			{
			case TRTextureFilterMode::TR_NEAREST:
				if (level1 != level2)
				{
					texel1 = TRTexture2DSampler::textureSamplingNearest(m_texHolders[level1], glm::vec2(u, v));
					texel2 = TRTexture2DSampler::textureSamplingNearest(m_texHolders[level2], glm::vec2(u, v));
				}
				else
				{
					texel1 = TRTexture2DSampler::textureSamplingNearest(m_texHolders[level1], glm::vec2(u, v));
					texel2 = texel1;
				}
				break;
			case TRTextureFilterMode::TR_LINEAR:
				//Trilinear interpolation
				if (level1 != level2)
				{
					texel1 = TRTexture2DSampler::textureSamplingBilinear(m_texHolders[level1], glm::vec2(u, v));
					texel2 = TRTexture2DSampler::textureSamplingBilinear(m_texHolders[level2], glm::vec2(u, v));
				}
				else
				{
					texel1 = TRTexture2DSampler::textureSamplingBilinear(m_texHolders[level1], glm::vec2(u, v));
					texel2 = texel1;
				}
				break;
			}
			//Interpolation
			float frac = level - (int)level;
			texel = (1.0f - frac) * texel1 + frac * texel2;
		}

		return texel;
	}

	//----------------------------------------------TRTexture2DSampler----------------------------------------------

	glm::vec4 TRTexture2DSampler::textureSamplingNearest(TRTextureHolder::ptr texture, glm::vec2 uv)
	{
		//Perform nearest sampling procedure
		unsigned char r, g, b, a = 255;
		texture->read(
			(std::uint16_t)(uv.x * (texture->getWidth() - 1)  + 0.5f), //Rounding
			(std::uint16_t)(uv.y * (texture->getHeight() - 1) + 0.5f), //Rounding
			r, g, b, a);

		constexpr float denom = 1.0f / 255.0f;
		return glm::vec4(r, g, b, a) * denom;
	}

	glm::vec4 TRTexture2DSampler::textureSamplingBilinear(TRTextureHolder::ptr texture, glm::vec2 uv)
	{
		//Perform bilinear sampling procedure
		const auto &w = texture->getWidth();
		const auto &h = texture->getHeight();

		float fx = (uv.x * (w- 1)), fy = (uv.y * (h - 1));
		std::uint16_t ix = (std::uint16_t)fx, iy = (std::uint16_t)fy;
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
		texture->read(ix, iy, r, g, b, a);
		glm::vec4 p0(r, g, b, a);
		//p1
		texture->read((ix + 1 >= w) ? ix : (ix + 1), iy, r, g, b, a);
		glm::vec4 p1(r, g, b, a);
		//p2
		texture->read(ix, (iy + 1 >= h) ? iy : (iy + 1), r, g, b, a);
		glm::vec4 p2(r, g, b, a);
		//p3
		texture->read((ix + 1 >= w) ? ix : (ix + 1), (iy + 1 >= h) ? iy : (iy + 1), r, g, b, a);
		glm::vec4 p3(r, g, b, a);
		return ((1.0f - frac_x) * (1.0f - frac_y) * p0 + frac_x * (1.0f - frac_y) * p1 +
			   (1.0f - frac_x) * frac_y * p2 + frac_x * frac_y * p3) * denom;
	}
}