#ifndef TRPIXEL_SAMPLER_H
#define TRPIXEL_SAMPLER_H

#include <array>
#include <vector>

#include "glm/glm.hpp"

namespace TinyRenderer
{
	template <typename T, size_t N>
	class TRIPixelSampler
	{
	public:
		std::array<T, N> samplers;

		static size_t getSamplingNum() { return N; }

		T& operator[](const int &index) { return samplers[index]; }
		const T& operator[](const int &index) const { return samplers[index]; }

	};

	//1x Sampling Point
	template <typename T>
	class TRPixelSampler1X : public TRIPixelSampler<T, 1>
	{
	public:

		TRPixelSampler1X(const T &value) { samplers.fill(value); }

		static const std::array<glm::vec2, 1> &getSamplingOffsets()
		{
			return { glm::vec2(0.0f, 0.0f) };
		}

	};

	//2x Sampling Point
	template <typename T>
	class TRPixelSampler2X : public TRIPixelSampler<T, 2>
	{
	public:

		TRPixelSampler2X(const T &value) { samplers.fill(value); }

		static const std::array<glm::vec2, 2> &getSamplingOffsets()
		{
			return { glm::vec2(-0.25f, -0.25f), glm::vec2(+0.25f, +0.25f) };
		}

	};

	//4x Sampling Point
	template <typename T>
	class TRPixelSampler4X : public TRIPixelSampler<T, 4>
	{
	public:

		TRPixelSampler4X(const T &value) { samplers.fill(value); }

		static const std::array<glm::vec2, 4> &getSamplingOffsets()
		{
			//Sampling points' offset
			//Note:Rotated grid sampling pattern
			//Refs: https://mynameismjp.wordpress.com/2012/10/24/msaa-overview/
			return
			{
				glm::vec2(+0.125f, +0.375f),
				glm::vec2(+0.375f, -0.125f),
				glm::vec2(-0.125f, -0.375f),
				glm::vec2(-0.375f, +0.125f)
			};
		}
	};

	//8x Sampling Point
	template <typename T>
	class TRPixelSampler8X : public TRIPixelSampler<T, 8>
	{
	public:

		TRPixelSampler8X(const T &value) { samplers.fill(value); }

		static const std::array<glm::vec2, 8> &getSamplingOffsets()
		{
			//Sampling points' offset
			//Note:Rotated grid sampling pattern
			//Refs: https://mynameismjp.wordpress.com/2012/10/24/msaa-overview/
			return
			{
				glm::vec2(-0.375f, +0.375f),
				glm::vec2(+0.125f, +0.375f),
				glm::vec2(-0.125f, +0.125f),
				glm::vec2(+0.375f, +0.125f),
				glm::vec2(-0.375f, -0.125f),
				glm::vec2(+0.125f, -0.125f),
				glm::vec2(-0.125f, -0.375f),
				glm::vec2(+0.375f, -0.375f)
			};
		}
	};

#define MSAA4X

#ifdef MSAA4X
	template<typename T>
	using TRPixelSampler = TRPixelSampler4X<T>;
	//Note: MSAA 8X is a little time-consuming
	//template<typename T>
	//using TRPixelSampler = TRPixelSampler8X<T>;
#else
	template<typename T>
	using TRPixelSampler = TRPixelSampler1X<T>;
#endif

	using TRPixelRGB = std::array<unsigned char, 3>;
	using TRPixelRGBA = std::array<unsigned char, 4>;
	using TRMaskPixelSampler = TRPixelSampler<unsigned char>;
	using TRDepthPixelSampler = TRPixelSampler<float>;
	using TRColorPixelSampler = TRPixelSampler<TRPixelRGBA>;

	//Framebuffer attachment
	using TRMaskBuffer = std::vector<TRMaskPixelSampler>;
	using TRDepthBuffer = std::vector<TRDepthPixelSampler>;
	using TRColorBuffer = std::vector<TRColorPixelSampler>;

	constexpr TRPixelRGBA k_trWhite = { 255, 255, 255 ,255 };
	constexpr TRPixelRGBA k_trBlack = { 0, 0, 0, 0 };
}

#endif