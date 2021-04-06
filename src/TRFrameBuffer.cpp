#include "TRFrameBuffer.h"

#include <cmath>
#include <algorithm>

#include "TRParallelWrapper.h"

namespace TinyRenderer
{
	TRFrameBuffer::TRFrameBuffer(int width, int height)
		: m_width(width), m_height(height)
	{
		m_maskBuffer.resize(m_width * m_height, 0);
		m_depthBuffer.resize(m_width * m_height, 1.0f);
		m_colorBuffer.resize(m_width * m_height, trBlack);
	}

	float TRFrameBuffer::readDepth(const unsigned int &x, const unsigned int &y, const unsigned int &i) const
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return 0.0f;
		//Note: i is the sampling point index
		return m_depthBuffer[y*m_width + x][i];
	}

	TRPixelRGBA TRFrameBuffer::readColor(const unsigned int &x, const unsigned int &y, const unsigned int &i) const
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return trBlack;
		//Note: i is the sampling point index
		return m_colorBuffer[y*m_width + x][i];
	}

	void TRFrameBuffer::clearDepth(const float &depth)
	{
		parallelFor((size_t)0, (size_t)(m_width * m_height), [&](const size_t &index)
		{
			m_depthBuffer[index] = depth;
			m_maskBuffer[index] = 0;
		});
	}

	void TRFrameBuffer::clearColor(const glm::vec4 &color)
	{
		unsigned char red = static_cast<unsigned char>(255 * color.x);
		unsigned char green = static_cast<unsigned char>(255 * color.y);
		unsigned char blue = static_cast<unsigned char>(255 * color.z);
		unsigned char alpha = static_cast<unsigned char>(255 * color.w);
		TRPixelRGBA clearColor = { red, green, blue, alpha };

		parallelFor((size_t)0, (size_t)(m_width * m_height), [&](const size_t &index)
		{
			m_maskBuffer[index] = 0;
			m_colorBuffer[index] = clearColor;
		});
	}

	void TRFrameBuffer::clearColorAndDepth(const glm::vec4 &color, const float &depth)
	{
		unsigned char red = static_cast<unsigned char>(255 * color.x);
		unsigned char green = static_cast<unsigned char>(255 * color.y);
		unsigned char blue = static_cast<unsigned char>(255 * color.z);
		unsigned char alpha = static_cast<unsigned char>(255 * color.w);
		TRPixelRGBA clearColor = { red, green, blue, alpha };

		parallelFor((size_t)0, (size_t)(m_width * m_height), [&](const size_t &index)
		{
			m_maskBuffer[index] = 0;
			m_depthBuffer[index] = depth;
			m_colorBuffer[index] = clearColor;
		});
	}

	void TRFrameBuffer::writeDepth(const unsigned int &x, const unsigned int &y, const unsigned int &i, const float &value)
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return;
		//Note: i is the sampling point index
		m_depthBuffer[y * m_width + x][i] = value;
	}

	void TRFrameBuffer::writeColor(const unsigned int &x, const unsigned int &y, const unsigned int &i, const glm::vec4 &color)
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return;
		//Note: i is the sampling point index
		TRPixelRGBA value;
		value[0] = static_cast<unsigned char>(color.x * 255);//RED
		value[1] = static_cast<unsigned char>(color.y * 255);//GREEN
		value[2] = static_cast<unsigned char>(color.z * 255);//BLUE
		value[3] = static_cast<unsigned char>(glm::min(255 * color.w, 255.0f));//ALPHA
		int index = y * m_width + x;
		m_colorBuffer[index][i] = value;
	}

	void TRFrameBuffer::writeCoverageMask(const unsigned int &x, const unsigned int &y, const TRMaskPixelSampler &mask)
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return;
		int index = y * m_width + x;
		for (int s = 0; s < mask.getSamplingNum(); ++s)
		{
			if (mask[s] == 1)
			{
				m_maskBuffer[index][s] = 1;
			}
		}
	}

	void TRFrameBuffer::writeColorWithMask(const unsigned int &x, const unsigned int &y, const glm::vec4 &color, const TRMaskPixelSampler &mask)
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return;
		TRPixelRGBA value;
		value[0] = static_cast<unsigned char>(color.x * 255);//RED
		value[1] = static_cast<unsigned char>(color.y * 255);//GREEN
		value[2] = static_cast<unsigned char>(color.z * 255);//BLUE
		value[3] = static_cast<unsigned char>(glm::min(255 * color.w, 255.0f));//ALPHA

		int index = y * m_width + x;
		//Only write color if the corresponding mask equals to 1
		for (int s = 0; s < mask.getSamplingNum(); ++s)
		{
			if (mask[s] == 1)
			{
				m_colorBuffer[index][s] = value;
			}
		}
	}

	void TRFrameBuffer::writeDepthWithMask(const unsigned int &x, const unsigned int &y, const TRDepthPixelSampler &depth, const TRMaskPixelSampler &mask)
	{
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
			return;
		int index = y * m_width + x;
		//Only write depth if the corresponding mask equals to 1
		for (int s = 0; s < mask.getSamplingNum(); ++s)
		{
			if (mask[s] == 1)
			{
				m_depthBuffer[index][s] = depth[s];
			}
		}
	}

	const TRColorBuffer &TRFrameBuffer::resolve()
	{
		//MSAA Resolve according to coverage mask
		parallelFor((size_t)0, (size_t)(m_width * m_height), [&](const size_t &index)
		{
			auto &currentSamper = m_colorBuffer[index];
			const auto &currentMaskSampler = m_maskBuffer[index];
			glm::vec4 sum(0.0f);
			//Average the sampling color for each shaded pixel.
			for (int s = 0; s < currentSamper.getSamplingNum(); ++s)
			{
				if (currentMaskSampler[s] == 1)
				{
					sum.x += currentSamper[s][0];//RED
					sum.y += currentSamper[s][1];//GREEN
					sum.z += currentSamper[s][2];//BLUE
					sum.w += currentSamper[s][3];//ALPHA
				}
			}
			sum /= currentSamper.getSamplingNum();
			TRPixelRGBA value;
			value[0] = static_cast<unsigned char>(sum.x);
			value[1] = static_cast<unsigned char>(sum.y);
			value[2] = static_cast<unsigned char>(sum.z);
			value[3] = static_cast<unsigned char>(sum.w);
			currentSamper[0] = value;
			
		}, TRExecutionPolicy::TR_PARALLEL);
		return m_colorBuffer;
	}

}
