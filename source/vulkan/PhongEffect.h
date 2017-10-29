#pragma once

#include <glm/glm.hpp>
#include "vulkan/Effect.h"
#include "vulkan/ShaderBuffer.h"
#include "vulkan/handles/Buffer.h"
#include "vulkan/PipelineInterface.h"
#include "Light.h"

namespace Vulkan
{
	class Renderer;
	class DescriptorSetLayout;
	class DescriptorPool;
	class DescriptorSet;
	class Pipeline2;
	class PipelineLayout;
	class VertexDescription;
	class Shader;

	/** \brief Most basic effect
	*
	* Simply transforms each vertex and sets a pixel color
	**/
	class PhongEffect : public Effect
	{
	public:
		enum PipelineType2
		{
			BASIC = 0,
			WIREFRAME,
			TEST,
			DEBUG
		};

		class VertexUniformBuffer : public ShaderBuffer
		{
		public:
			virtual void UpdateMemory(VkDevice device);
			virtual int GetSize();

			// Public data members
			struct {
				glm::mat4 projectionMatrix;
				glm::mat4 viewMatrix;
				glm::vec4 lightDir = glm::vec4(1.0f, -1.0f, 1.0f, 1.0f);
				glm::vec3 eyePos;
				float t;
			} camera;

			struct {
				bool useInstancing;
				glm::vec3 garbage;
			} constants; // Currently unused
		};

		class FragmentUniformBuffer : public Vulkan::ShaderBuffer
		{
		public:
			virtual void UpdateMemory(VkDevice device);
			virtual int GetSize();

			// Public data members
			std::vector<Vulkan::Light> lights;

			struct {
				float numLights;
				glm::vec3 garbage;
			} constants;
		};

		// Override the base class interfaces
		virtual void CreateDescriptorPool(Device* device);
		virtual void CreateVertexDescription(Device* device);
		virtual void CreatePipelineInterface(Device* device);
		virtual void CreateDescriptorSets(Device* device);
		virtual void CreatePipeline(Renderer* renderer);

		/* Updates the memory for the effects descriptors
		*/
		virtual void UpdateMemory(Device* device);

		PhongEffect();

		/* Shader descriptors */
		VertexUniformBuffer per_frame_vs;
		FragmentUniformBuffer per_frame_ps;

		DescriptorSet* mCameraDescriptorSet;
		DescriptorSet* mLightDescriptorSet;

		// TODO: Should this really be here?
		const uint32_t					MAX_NUM_TEXTURES = 64;
	};
}