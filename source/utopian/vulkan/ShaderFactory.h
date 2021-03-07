#pragma once

#include <string>
#include <vector>
#include <map>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VertexDescription.h"
#include "utility/Module.h"
#include "utility/Common.h"
#include <glslang/Public/ShaderLang.h>

#define VERTEX_SHADER_IDX 0
#define PIXEL_SHADER_IDX 1

namespace Utopian::Vk
{
   enum UniformVariableType
   {
      UVT_SSBO,
      UVT_SAMPLER1D,
      UVT_SAMPLER2D,
      UVT_SAMPLER3D,
      UVT_SAMPLERCUBE
   };

   struct PushConstantDesc
   {
      std::string name;
      uint32_t size;
   };

   /* Can be SSBOs and samplers */
   struct UniformVariableDesc
   {
      UniformVariableType type;
      std::string name;
      uint32_t set;
      uint32_t binding;
      uint32_t arraySize;
   };

   /* UBOs */
   struct UniformBlockDesc
   {
      std::string name;
      uint32_t set;
      uint32_t binding;
      uint32_t size;
   };

   struct NameMapping
   {
      NameMapping() {}

      NameMapping(uint32_t _set, uint32_t _binding) :
         set(_set), binding(_binding) {}

      uint32_t set;
      uint32_t binding;
   };

   struct ShaderReflection
   {
      std::map<std::string, UniformBlockDesc> uniformBlocks;
      std::map<std::string, UniformBlockDesc> storageBuffers;
      std::map<std::string, UniformVariableDesc> combinedSamplers;
      std::map<std::string, UniformVariableDesc> images;
      std::map<std::string, PushConstantDesc> pushConstants;
      SharedPtr<VertexDescription> vertexDescription;

      // Maps text identifier to binding in a specific descriptor set
      std::map<std::string, NameMapping> nameMappings;
   };

   struct CompiledShader
   {
      std::vector<unsigned int> spirvBytecode;
      ShaderReflection reflection;
      VkShaderStageFlagBits shaderStage;
      VkShaderModule shaderModule;
   };

   struct ShaderCreateInfo
   {
      ShaderCreateInfo()
      {
         vertexShaderPath = "NONE";
         fragmentShaderPath = "NONE";
         geometryShaderPath = "NONE";
         tescShaderPath = "NONE";
         teseShaderPath = "NONE";
         computeShaderPath = "NONE";
      }

      std::string vertexShaderPath;
      std::string fragmentShaderPath;
      std::string geometryShaderPath;
      std::string tescShaderPath;
      std::string teseShaderPath;
      std::string computeShaderPath;
   };

   class Shader
   {
   public:
      Shader(Device* device);
      ~Shader();

      void AddShaderStage(VkPipelineShaderStageCreateInfo shaderStageCreateInfo);
      void AddCompiledShader(SharedPtr<CompiledShader> compiledShader);

      int NameToBinding(std::string name);
      int NameToSet(std::string name);

      const VertexDescription* GetVertexDescription() const;

      bool IsComputeShader() const;

      std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
      std::vector<SharedPtr<CompiledShader>> compiledShaders;

   private:
      Device* mDevice;
   };

   class ShaderFactory : public Module<ShaderFactory>
   {
   public:
      ShaderFactory(Device* device);
      ~ShaderFactory();

      SharedPtr<Shader> CreateShader(const ShaderCreateInfo& shaderCreateInfo);

      void AddIncludeDirectory(std::string directory);

   private:
      SharedPtr<CompiledShader> CompileShader(std::string filename);
      ShaderReflection ExtractShaderLayout(glslang::TProgram& program, EShLanguage shaderType);
      void ReflectVertexInput(glslang::TProgram& program, ShaderReflection* reflection);
   private:
      std::vector<Shader*> mLoadedShaders;
      std::vector<std::string> mIncludeDirectories;
      Device* mDevice;
   };

   ShaderFactory& gShaderFactory();
}
