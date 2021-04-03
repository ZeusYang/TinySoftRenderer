#ifndef TRFRAMEBUFFER_H
#define TRFRAMEBUFFER_H

#include <vector>
#include <memory>

#include "glm/glm.hpp"

namespace TinyRenderer
{
	/**
	 * @projectName   TinyRenderer
	 * @brief         Frame buffer class.
	 * @author        Wencong Yang
	 * @date          2019-04-29
	 */
	class TRFrameBuffer final
	{
	public:
		typedef std::shared_ptr<TRFrameBuffer> ptr;

		// ctor/dtor.
		TRFrameBuffer(int width, int height);
		~TRFrameBuffer() = default;

		void clear(const glm::vec4 &color);

		// Getter.
		int getWidth()const { return m_width; }
		int getHeight()const { return m_height; }
		unsigned char *getColorBuffer() { return m_colorBuffer.data(); }

		float readDepth(const unsigned int &x, const unsigned int &y) const;
		void writeDepth(const unsigned int &x, const unsigned int &y, const float &value);
		void writeColor(const unsigned int &x, const unsigned int &y, const glm::vec4 &color);

	private:
		std::vector<float> m_depthBuffer;          // Z-buffer
		std::vector<unsigned char> m_colorBuffer;   // Color buffer
		unsigned int m_width, m_height, m_channel;  // Viewport
	};
}

#endif