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

		std::uint16_t getWidth() const { return m_width; }
		std::uint16_t getHeight() const { return m_height; }

		virtual std::uint32_t read(const std::uint16_t &x, const std::uint16_t &y) const;
		virtual void read(const std::uint16_t &x, const std::uint16_t &y, unsigned char &r, unsigned char &g,
			unsigned char &b, unsigned char &a) const;
		
	protected:
		std::uint16_t m_width, m_height;
		std::uint32_t *m_data;//32bpp texture format (RGBA)

		void loadTexture(const unsigned int &nElements, unsigned char *data, const std::uint16_t &width, 
			const std::uint16_t &height, const int &channel);
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
		//Change the k_blockSize, you should also change the corresponding code in xyToIndex
		static constexpr int k_blockSize = 4;
		static constexpr int k_blockSize2 = 16;

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
		static constexpr int k_blockSize = 32; //Note: block size should not exceed 256
		static constexpr int k_blockSize2 = 1024;
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

		inline static void encodeMortonCurve(const std::uint8_t &x, const std::uint8_t &y, std::uint16_t &index)
		{
			//Morton curve encoding
			//Refs: https://en.wikipedia.org/wiki/Z-order_curve
			index = 0;
			index |= ((x & (1 << 0)) << (0)) | ((y & (1 << 0)) << (1));
			index |= ((x & (1 << 1)) << (1)) | ((y & (1 << 1)) << (2));
			index |= ((x & (1 << 2)) << (2)) | ((y & (1 << 2)) << (3));
			index |= ((x & (1 << 3)) << (3)) | ((y & (1 << 3)) << (4));
			index |= ((x & (1 << 4)) << (4)) | ((y & (1 << 4)) << (5));
			index |= ((x & (1 << 5)) << (5)) | ((y & (1 << 5)) << (6));
			index |= ((x & (1 << 6)) << (6)) | ((y & (1 << 6)) << (7));
			index |= ((x & (1 << 7)) << (7)) | ((y & (1 << 7)) << (8));
			index |= ((x & (1 << 8)) << (8)) | ((y & (1 << 8)) << (9));
			index |= ((x & (1 << 9)) << (9)) | ((y & (1 << 9)) << (10));
		}
	};
}

#endif