#include "TRTextureHolder.h"

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

	void TRTextureHolder::loadTexture(
		unsigned int nElements, 
		unsigned char *data,
		std::uint16_t width,
		std::uint16_t height,
		int channel)
	{
		//Unified to 32bpp texel format
		m_data = new std::uint32_t[nElements];

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				unsigned char r, g, b, a;
				int index = (y * width + x) * channel;
				switch (channel)
				{
				case 1://R
					r = g = b = data[index];
					a = 255;
					break;
				case 3://RGB
					r = data[index + 0];
					g = data[index + 1];
					b = data[index + 2];
					a = 255;
					break;
				case 4://RGBA
					r = data[index + 0];
					g = data[index + 1];
					b = data[index + 2];
					a = data[index + 3];
					break;
				default:
					r = g = b = data[index];
					a = 255;
					break;
				}
				m_data[xyToIndex(x, y)] = (r << 24) | (g << 16) | (b << 8) | (a << 0);
			}
		}
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
		m_widthInTiles = (width + blockSize - 1) / blockSize;
		m_heightInTiles = (height + blockSize - 1) / blockSize;
		//Padding
		unsigned int nElements = m_widthInTiles * m_heightInTiles * blockSize_2;
		TRTextureHolder::loadTexture(nElements, data, width, height, channel);
	}

	unsigned int TRTilingTextureHolder::xyToIndex(const std::uint16_t &x, const std::uint16_t &y) const
	{
		//Tiling address mapping
		return ((int)(y / blockSize) * m_widthInTiles + (int)(x / blockSize)) * blockSize_2 + y % blockSize * blockSize + x % blockSize;
	}

	//----------------------------------------------TRZCurveTilingTextureHolder----------------------------------------------

	TRZCurveTilingTextureHolder::TRZCurveTilingTextureHolder(unsigned char *data, std::uint16_t width, std::uint16_t height, int channel)
		: TRTextureHolder(width, height)
	{
		m_widthInTiles = (width + blockSize - 1) / blockSize;
		m_heightInTiles = (height + blockSize - 1) / blockSize;
		//Padding
		unsigned int nElements = m_widthInTiles * m_heightInTiles * blockSize_2;
		TRTextureHolder::loadTexture(nElements, data, width, height, channel);
	}

	unsigned int TRZCurveTilingTextureHolder::xyToIndex(const std::uint16_t &x, const std::uint16_t &y) const
	{
		//Address mapping
		std::uint8_t rx = x % blockSize, ry = y % blockSize;
		std::uint16_t ri = 0;
		TRZCurveTilingTextureHolder::encodeMortonCurve(rx, ry, ri);
		return (y / blockSize * m_widthInTiles + x / blockSize) * blockSize_2 + ri;
	}
}