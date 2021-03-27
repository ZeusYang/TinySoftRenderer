#ifndef TRTEXTURE_HOLDER_H
#define TRTEXTURE_HOLDER_H

#include <memory>

namespace TinyRenderer
{
	//Texture memory layout
	class TRTextureHolder
	{
	public:
		typedef std::shared_ptr<TRTextureHolder> ptr;

		TRTextureHolder(std::uint16_t width, std::uint16_t height);
		virtual ~TRTextureHolder();

		virtual std::uint32_t read(const std::uint16_t &x, const std::uint16_t &y) const;
		
	protected:
		std::uint16_t m_width, m_height;
		std::uint32_t *m_data;//32bpp texture format (RGBA)

		void loadTexture(unsigned int nElements,
			unsigned char *data,
			std::uint16_t width,
			std::uint16_t height,
			int channel);
		virtual unsigned int xyToIndex(const std::uint16_t &x, const std::uint16_t &y) const = 0;
		void freeTexture();
	};

	//Linear layout
	class TRLinearTextureHolder final : public TRTextureHolder
	{
	public:
		typedef std::shared_ptr<TRLinearTextureHolder> ptr;

		TRLinearTextureHolder(unsigned char *data, std::uint16_t width, std::uint16_t height, int channel);
		virtual ~TRLinearTextureHolder() = default;

	private:
		virtual unsigned int xyToIndex(const std::uint16_t &x, const std::uint16_t &y) const override;

	};

	//Tiling layout
	class TRTilingTextureHolder final : public TRTextureHolder
	{
	public:
		typedef std::shared_ptr<TRTilingTextureHolder> ptr;

		TRTilingTextureHolder(unsigned char *data, std::uint16_t width, std::uint16_t height, int channel);
		virtual ~TRTilingTextureHolder() = default;

	private:
		static constexpr int blockSize = 4;
		static constexpr int blockSize_2 = 16;

		int m_widthInTiles = 0;
		int m_heightInTiles = 0;

		virtual unsigned int xyToIndex(const std::uint16_t &x, const std::uint16_t & y) const override;

	};

	//Tiling and morton order layout
	//Refs: https://fgiesen.wordpress.com/2011/01/17/texture-tiling-and-swizzling/
	class TRZCurveTilingTextureHolder final : public TRTextureHolder
	{
	public:
		typedef std::shared_ptr<TRZCurveTilingTextureHolder> ptr;

		TRZCurveTilingTextureHolder(unsigned char *data, std::uint16_t width, std::uint16_t height, int channel);
		virtual ~TRZCurveTilingTextureHolder() = default;

	private:
		//Block size for tiling
		static constexpr int blockSize = 32; //Note: block size should not exceed 256
		static constexpr int blockSize_2 = 1024;
		static constexpr int bits = 5;

		int m_widthInTiles = 0;
		int m_heightInTiles = 0;

		virtual unsigned int xyToIndex(const std::uint16_t &x, const std::uint16_t & y) const override;

		static void decodeMortonCurve(const std::uint16_t &index, std::uint8_t &x, std::uint8_t &y)
		{
			//Morton curve decoding
			//Refs: https://en.wikipedia.org/wiki/Z-order_curve
			x = 0, y = 0;
			for (int i = 0; i < bits; ++i)
			{
				x |= (index & (1 << (2 * i))) >> i;
				y |= (index & (1 << (2 * i + 1))) >> (i + 1);
			}
		}

		static void encodeMortonCurve(const std::uint8_t &x, const std::uint8_t &y, std::uint16_t &index)
		{
			//Morton curve encoding
			//Refs: https://en.wikipedia.org/wiki/Z-order_curve
			index = 0;
			for (int i = 0; i < 2 * bits; ++i)
			{
				index |= ((x & (1 << i)) << (i)) | ((y & (1 << i)) << (i + 1));
			}
		}
	};
}

#endif