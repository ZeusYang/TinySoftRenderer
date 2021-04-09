#ifndef TRSHADERPROGRAM_H
#define TRSHADERPROGRAM_H

#include "TRShadingPipeline.h"

namespace TinyRenderer
{
	class TR3DShadingPipeline : public TRShadingPipeline
	{
	public:

		typedef std::shared_ptr<TR3DShadingPipeline> ptr;

		virtual ~TR3DShadingPipeline() = default;

		virtual void vertexShader(VertexData &vertex) const override;
		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const override;

	};

	class TRDoNothingShadingPipeline : public TRShadingPipeline
	{
	public:

		typedef std::shared_ptr<TRDoNothingShadingPipeline> ptr;

		virtual ~TRDoNothingShadingPipeline() = default;

		virtual void vertexShader(VertexData &vertex) const override;
		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const override;
	};

	class TRTextureShadingPipeline final : public TR3DShadingPipeline
	{
	public:

		typedef std::shared_ptr<TRTextureShadingPipeline> ptr;

		virtual ~TRTextureShadingPipeline() = default;

		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const override;
	};

	class TRLODVisualizePipeline final : public TR3DShadingPipeline
	{
	public:

		typedef std::shared_ptr<TRLODVisualizePipeline> ptr;

		virtual ~TRLODVisualizePipeline() = default;

		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const override;
	};

	class TRPhongShadingPipeline final : public TR3DShadingPipeline
	{
	public:
		typedef std::shared_ptr<TRPhongShadingPipeline> ptr;

		virtual ~TRPhongShadingPipeline() = default;

		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const override;
	};

	class TRBlinnPhongShadingPipeline final : public TR3DShadingPipeline
	{
	public:
		typedef std::shared_ptr<TRBlinnPhongShadingPipeline> ptr;

		virtual ~TRBlinnPhongShadingPipeline() = default;

		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const override;
	};

	class TRBlinnPhongNormalMapShadingPipeline final : public TR3DShadingPipeline
	{
	public:
		typedef std::shared_ptr<TRBlinnPhongNormalMapShadingPipeline> ptr;

		virtual ~TRBlinnPhongNormalMapShadingPipeline() = default;

		virtual void vertexShader(VertexData &vertex) const override;
		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const override;
	};

	class TRAlphaBlendingShadingPipeline final : public TR3DShadingPipeline
	{
	public:
		typedef std::shared_ptr<TRAlphaBlendingShadingPipeline> ptr;

		virtual ~TRAlphaBlendingShadingPipeline() = default;

		virtual void fragmentShader(const FragmentData &data, glm::vec4 &fragColor,
			const glm::vec2 &dUVdx, const glm::vec2 &dUVdy) const override;
	};
}

#endif