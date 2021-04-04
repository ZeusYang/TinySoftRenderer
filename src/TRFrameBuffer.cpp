#include "TRFrameBuffer.h"

#include <cmath>
#include <algorithm>

#include "TRParallelWrapper.h"

namespace TinyRenderer
{
	TRFrameBuffer::TRFrameBuffer(int width, int height)
		: m_channel(4), m_width(width), m_height(height)
	{
		m_depthBuffer.resize(m_width * m_height, 1.0f);
		m_colorBuffer.resize(m_width * m_height * m_channel, 255);
	}

	float TRFrameBuffer::readDepth(const unsigned int &x, const unsigned int &y) const
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return 0.0f;
		return m_depthBuffer[y*m_width + x];
	}

	void TRFrameBuffer::clear(const glm::vec4 &color)
	{
		unsigned char red = static_cast<unsigned char>(255 * color.x);
		unsigned char green = static_cast<unsigned char>(255 * color.y);
		unsigned char blue = static_cast<unsigned char>(255 * color.z);
		unsigned char alpha = static_cast<unsigned char>(255 * color.w);

		parallelFor(0, (int)(m_width * m_height), [&](const int &index)
		{
			m_depthBuffer[index] = 1.0f;
			m_colorBuffer[index * m_channel + 0] = red;
			m_colorBuffer[index * m_channel + 1] = green;
			m_colorBuffer[index * m_channel + 2] = blue;
			m_colorBuffer[index * m_channel + 3] = alpha;
		});
	}

	void TRFrameBuffer::writeDepth(const unsigned int &x, const unsigned int &y, const float &value)
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return;
		unsigned int index = y * m_width + x;
		m_depthBuffer[index] = value;
	}

	void TRFrameBuffer::writeColor(const unsigned int &x, const unsigned int &y, const glm::vec4 &color)
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return;

		// Clamping in case overflow
		unsigned char red = static_cast<unsigned char>(color.x * 255);
		unsigned char green = static_cast<unsigned char>(color.y * 255);
		unsigned char blue = static_cast<unsigned char>(color.z * 255);
		unsigned char alpha = static_cast<unsigned char>(glm::min(255 * color.w, 255.0f));
		unsigned int index = y * m_width * m_channel + x * m_channel;
		m_colorBuffer[index + 0] = red;
		m_colorBuffer[index + 1] = green;
		m_colorBuffer[index + 2] = blue;
		m_colorBuffer[index + 3] = alpha;
	}

}
