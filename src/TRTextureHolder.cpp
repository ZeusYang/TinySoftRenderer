#include "TRTextureHolder.h"

#include "TRParallelWrapper.h"

namespace TinyRenderer
{
	//----------------------------------------------TRTextureHolder----------------------------------------------

	TRTextureHolder::TRTextureHolder(std::uint16_t width, std::uint16_t height)
		: m_width(width), m_height(height), m_data(nullptr) {}

	TRTextureHolder::~TRTextureHolder() { freeTexture(); }

	std::uint32_t TRTextureHolder::read(const std::uint16_t &x, const std::uint16_t &y) const
	{
		//Please guarantee that x and y are in [0,width-1],[0,height-1] respectively
		return m_data[xyToIndex(x, y)];
	}

	void TRTextureHolder::read(const std::uint16_t &x, const std::uint16_t &y, unsigned char &r, unsigned char &g,
		unsigned char &b, unsigned char &a) const
	{
		//Please guarantee that x and y are in [0,width-1],[0,height-1] respectively
		std::uint32_t texel = read(x, y);
		r = (texel >> 24) & 0xFF;
		g = (texel >> 16) & 0xFF;
		b = (texel >>  8) & 0xFF;
		a = (texel >>  0) & 0xFF;
	}

	void TRTextureHolder::loadTexture(const unsigned int &nElements, unsigned char *data, const std::uint16_t &width,
		const std::uint16_t &height, const int &channel)
	{
		//Unified to 32bpp texel format
		m_data = new std::uint32_t[nElements];
		parallelFor((int)0, (int)(height * width), [&](const int &index) -> void
		{
			int y = index / width, x = index % width;
			unsigned char r, g, b, a;
			int addres = index * channel;
			switch (channel)
			{
			case 1://R
				r = g = b = data[addres], a = 255;
				break;
			case 3://RGB
				r = data[addres + 0], g = data[addres + 1], b = data[addres + 2], a = 255;
				break;
			case 4://RGBA
				r = data[addres + 0], g = data[addres + 1], b = data[addres + 2], a = data[addres + 3];
				break;
			default:
				r = g = b = data[addres], a = 255;
				break;
			}
			m_data[xyToIndex(x, y)] = (r << 24) | (g << 16) | (b << 8) | (a << 0);
		});
	}

	void TRTextureHolder::freeTexture()
	{
		if (m_data != nullptr)
		{
			delete[] m_data;
			m_data = nullptr;
		}
	}

	//----------------------------------------------TRLinearTextureHolder----------------------------------------------

	TRLinearTextureHolder::TRLinearTextureHolder(unsigned char *data, std::uint16_t width, std::uint16_t height, int channel)
		: TRTextureHolder(width, height)
	{
		TRTextureHolder::loadTexture(width * height, data, width, height, channel);
	}

	unsigned int TRLinearTextureHolder::xyToIndex(const std::uint16_t &x, const std::uint16_t &y) const
	{
		//Linear address mapping
		return y * m_width + x;
	}

	//----------------------------------------------TRTilingTextureHolder----------------------------------------------

	TRTilingTextureHolder::TRTilingTextureHolder(unsigned char *data, std::uint16_t width, std::uint16_t height, int channel)
		: TRTextureHolder(width, height)
	{
		m_widthInTiles = (width + k_blockSize - 1) / k_blockSize;
		m_heightInTiles = (height + k_blockSize - 1) / k_blockSize;
		//Padding
		unsigned int nElements = m_widthInTiles * m_heightInTiles * k_blockSize2;
		TRTextureHolder::loadTexture(nElements, data, width, height, channel);
	}

	unsigned int TRTilingTextureHolder::xyToIndex(const std::uint16_t &x, const std::uint16_t &y) const
	{
		//Tiling address mapping
		//Note: this is naive version
		//return ((int)(y / k_blockSize) * m_widthInTiles + (int)(x / k_blockSize)) * k_blockSize2 + (y % k_blockSize) * k_blockSize + x % k_blockSize;
		//Note: this is optimized version
		return (((int)(y >> 2) * m_widthInTiles + (int)(x >> 2)) << 4) + ((y & 3) << 2) + (x & 3);
	}

	//----------------------------------------------TRZCurveTilingTextureHolder----------------------------------------------

	TRZCurveTilingTextureHolder::TRZCurveTilingTextureHolder(unsigned char *data, std::uint16_t width, std::uint16_t height, int channel)
		: TRTextureHolder(width, height)
	{
		m_widthInTiles = (width + k_blockSize - 1) / k_blockSize;
		m_heightInTiles = (height + k_blockSize - 1) / k_blockSize;
		//Padding
		unsigned int nElements = m_widthInTiles * m_heightInTiles * k_blockSize2;
		TRTextureHolder::loadTexture(nElements, data, width, height, channel);
	}

	unsigned int TRZCurveTilingTextureHolder::xyToIndex(const std::uint16_t &x, const std::uint16_t &y) const
	{
		//Address mapping
		std::uint8_t rx = x & (k_blockSize - 1), ry = y & (k_blockSize - 1);
		std::uint16_t ri = 0;
		TRZCurveTilingTextureHolder::encodeMortonCurve(rx, ry, ri);
		return ((y >> bits) * m_widthInTiles + (x  >> bits)) * k_blockSize2 + ri;
	}
}