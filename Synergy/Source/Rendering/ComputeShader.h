
#pragma once
#include "Image.h"
#include "Buffer.h"


namespace SNG
{

	enum class ImageLayout
	{
		Undefined, Present, Shader
	};

	struct ImageUniform
	{
		Ref<Image> Image;
		ImageLayout FinalLayout;
		uint32_t Binding;
	};

	struct BufferUniform
	{
		Ref<Buffer> Buffer;
		uint32_t Binding;
	};



	class ComputeShader
	{
	public:

		ComputeShader(const std::string& filepath);
		~ComputeShader();

		void AddUniformImage(const Ref<Image>& image, ImageLayout finalLayout);
		void AddUniformBuffer(const Ref<Buffer>& buffer);
		void FinalizeUniformCreation();
		void Run(uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ);
		
	private:

		void CreatePipeline();

	private:
		
		std::vector<ImageUniform> m_ImageUniforms;
		std::vector<BufferUniform> m_BufferUniforms;

		std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings;
		std::vector<VkDescriptorPoolSize> m_PoolSizes;

		VkShaderModule m_ShaderModule;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}
