#pragma once

#include <vector>
#include "vulkan/GBufferEffect.h"
#include "vulkan/DeferredEffect.h"
#include "utility/Common.h"

namespace Utopian
{
	class Renderable;
	class Light;
	class Camera;
	class BaseJob;

	namespace Vk
	{
		class Image;
		class Renderer;
		class RenderTarget;
		class BasicRenderTarget;
		class ScreenQuad;
	}

	struct SceneInfo
	{
		std::vector<Renderable*> renderables;
		std::vector<Light*> lights;
		std::vector<Camera*> cameras;
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::vec3 eyePos;
	};

	struct RenderingSettings
	{
		glm::vec4 fogColor;
		bool deferredPipeline;
		float fogStart;
		float fogDistance;
	};

	struct JobInput
	{
		JobInput(const SceneInfo& sceneInfo, const std::vector<BaseJob*>& jobs, const RenderingSettings& renderingSettings) 
			: sceneInfo(sceneInfo), jobs(jobs) , renderingSettings(renderingSettings) {

		}

		const SceneInfo& sceneInfo;
		const std::vector<BaseJob*>& jobs;
		const RenderingSettings& renderingSettings;
	};

	class BaseJob
	{
	public:
		virtual ~BaseJob() {};
		virtual void Init(const std::vector<BaseJob*>& jobs) = 0;

		virtual void Render(Vk::Renderer* renderer, const JobInput& jobInput) = 0;

	private:
	};

	class GBufferJob : public BaseJob
	{
	public:
		GBufferJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~GBufferJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::Image> positionImage;
		SharedPtr<Vk::Image> normalImage;
		SharedPtr<Vk::Image> albedoImage;
		SharedPtr<Vk::Image> depthImage;
		SharedPtr<Vk::RenderTarget> renderTarget;

		Vk::GBufferEffect mGBufferEffect;
	private:
	};

	class DeferredJob : public BaseJob
	{
	public:
		DeferredJob(Vk::Renderer* renderer, uint32_t width, uint32_t height);
		~DeferredJob();

		void Init(const std::vector<BaseJob*>& jobs) override;
		void Render(Vk::Renderer* renderer, const JobInput& jobInput) override;

		SharedPtr<Vk::BasicRenderTarget> renderTarget;

		Vk::DeferredEffect effect;
	private:
		SharedPtr<Vk::ScreenQuad> mScreenQuad;
	};
}