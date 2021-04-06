#ifndef TRFRAMEBUFFER_H
#define TRFRAMEBUFFER_H

#include <vector>
#include <memory>

#include "glm/glm.hpp"
#include "TRPixelSampler.h"

namespace TinyRenderer
{
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

		float readDepth(const unsigned int &x, const unsigned int &y, const unsigned int &i) const;
		TRPixelRGBA readColor(const unsigned int &x, const unsigned int &y, const unsigned int &i) const;

		void writeDepth(const unsigned int &x, const unsigned int &y, const unsigned int &i, const float &value);
		void writeColor(const unsigned int &x, const unsigned int &y, const unsigned int &i, const glm::vec4 &color);
		void writeCoverageMask(const unsigned int &x, const unsigned int &y, const TRMaskPixelSampler &mask);
		void writeColorWithMask(const unsigned int &x, const unsigned int &y, const glm::vec4 &color, const TRMaskPixelSampler &mask);
		void writeDepthWithMask(const unsigned int &x, const unsigned int &y, const TRDepthPixelSampler &depth, const TRMaskPixelSampler &mask);

		//MSAA resolve
		const TRColorBuffer &resolve();

	private:
		TRMaskBuffer m_maskBuffer;			   // Coverage mask
		TRDepthBuffer m_depthBuffer;           // Z-buffer
		TRColorBuffer m_colorBuffer;		   // Color buffer
		unsigned int m_width, m_height;
	};
}

#endif