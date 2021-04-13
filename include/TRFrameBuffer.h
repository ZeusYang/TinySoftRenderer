#ifndef TRFRAMEBUFFER_H
#define TRFRAMEBUFFER_H

#include <vector>
#include <memory>

#include "glm/glm.hpp"
#include "TRPixelSampler.h"

namespace TinyRenderer
{
	using uint = unsigned int;

	class TRFrameBuffer final
	{
	public:
		typedef std::shared_ptr<TRFrameBuffer> ptr;

		// ctor/dtor.
		TRFrameBuffer(int width, int height);
		~TRFrameBuffer() = default;

		void clearDepth(const float &depth);
		void clearColor(const glm::vec4 &color);
		void clearColorAndDepth(const glm::vec4 &color, const float &depth);

		// Getter.
		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }
		const TRDepthBuffer &getDepthBuffer() const { return m_depthBuffer; }
		const TRColorBuffer &getColorBuffer() const { return m_colorBuffer; }

		float readDepth(const uint &x, const uint &y, const uint &i) const;
		TRPixelRGBA readColor(const uint &x, const uint &y, const uint &i) const;

		void writeDepth(const uint &x, const uint &y, const uint &i, const float &value);
		void writeColor(const uint &x, const uint &y, const uint &i, const glm::vec4 &color);
		void writeColorWithMask(const uint &x, const uint &y, const glm::vec4 &color, const TRMaskPixelSampler &mask);
		void writeColorWithMaskAlphaBlending(const uint &x, const uint &y, const glm::vec4 &color, const TRMaskPixelSampler &mask);
		void writeDepthWithMask(const uint &x, const uint &y, const TRDepthPixelSampler &depth, const TRMaskPixelSampler &mask);

		//MSAA resolve
		const TRColorBuffer &resolve();

	private:
	
		TRDepthBuffer m_depthBuffer;           // Z-buffer
		TRColorBuffer m_colorBuffer;		   // Color buffer
		unsigned int m_width, m_height;
	};
}

#endif