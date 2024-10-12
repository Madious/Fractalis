
#include "Snpch.h"
#include "ComputeShader.h"
#include "API/VulkanContext.h"
#include <shaderc/shaderc.hpp>


namespace SNG
{

	namespace Utils
	{
		
		static void ReadFile(const std::string& filepath, std::stringstream& fileData)
		{
			std::ifstream file(filepath);
			SNG_ASSERT(file);

			std::string line;
			while (std::getline(file, line))
				fileData << line << "\n";
		}

		static void CompileShader(const std::stringstream& sourceCode, std::vector<uint32_t>& compilationResult)
		{
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(sourceCode.str(), shaderc_glsl_compute_shader, "shader.glsl");

			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				SNG_ERROR("Failed to compile shader -> {0}", module.GetErrorMessage());

			compilationResult = { module.cbegin(), module.cend() };
		}

		static VkImageLayout ConvertToVulkanImageLayout(ImageLayout layout)
		{
			switch (layout)
			{
				case ImageLayout::Undefined:	return VK_IMAGE_LAYOUT_UNDEFINED;
				case ImageLayout::Present:		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				case ImageLayout::Shader:		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
		}

		static VkAccessFlags GetImageLayoutAccessFlags(VkImageLayout layout)
		{
			switch (layout)
			{
				case VK_IMAGE_LAYOUT_UNDEFINED:							return 0;
				case VK_IMAGE_LAYOUT_GENERAL:							return VK_ACCESS_SHADER_WRITE_BIT;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:			return VK_ACCESS_SHADER_READ_BIT;
			}
		}

		static VkPipelineStageFlags GetImageLayoutShaderStage(VkImageLayout layout)
		{
			switch (layout)
			{
				case VK_IMAGE_LAYOUT_UNDEFINED:							return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				case VK_IMAGE_LAYOUT_GENERAL:							return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
		}

		static void TransitionImageLayout(VkCommandBuffer commandBuffer, const Ref<Image>& image,
			VkImageLayout oldLayout, VkImageLayout newLayout)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image->Raw();
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = GetImageLayoutAccessFlags(oldLayout);
			barrier.dstAccessMask = GetImageLayoutAccessFlags(newLayout);

			vkCmdPipelineBarrier(commandBuffer, GetImageLayoutShaderStage(oldLayout),
				GetImageLayoutShaderStage(newLayout), 0, 0,
				nullptr, 0, nullptr, 1, &barrier);
		}
	}



	ComputeShader::ComputeShader(const std::string& filepath)
	{
		std::stringstream sourceCode;
		Utils::ReadFile(filepath, sourceCode);

		std::vector<uint32_t> compiledCode;
		Utils::CompileShader(sourceCode, compiledCode);

		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = (uint32_t)compiledCode.size() * sizeof(uint32_t);
		moduleCreateInfo.pCode = compiledCode.data();
		SNG_CHECK_VULKAN_RESULT(vkCreateShaderModule(VulkanContext::GetDevice(), &moduleCreateInfo,
			nullptr, &m_ShaderModule), "Failed to create a Vulkan shader module");

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VulkanContext::GetCommandPool();
		allocInfo.commandBufferCount = VulkanContext::GetSwapchainImageCount();
		m_CommandBuffers.resize(VulkanContext::GetSwapchainImageCount());
		SNG_CHECK_VULKAN_RESULT(vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo,
			m_CommandBuffers.data()), "Failed to allocate a Vulkan command buffer");
	}

	ComputeShader::~ComputeShader()
	{
		VulkanContext::Wait();
		vkDestroyPipeline(VulkanContext::GetDevice(), m_Pipeline, nullptr);
		vkDestroyPipelineLayout(VulkanContext::GetDevice(), m_PipelineLayout, nullptr);
		vkDestroyDescriptorPool(VulkanContext::GetDevice(), m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(VulkanContext::GetDevice(), m_DescriptorSetLayout, nullptr);
		vkDestroyShaderModule(VulkanContext::GetDevice(), m_ShaderModule, nullptr);
	}

	void ComputeShader::AddUniformImage(const Ref<Image>& image, ImageLayout finalLayout)
	{
		uint32_t binding = m_LayoutBindings.size();

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		m_LayoutBindings.push_back(layoutBinding);

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSize.descriptorCount = VulkanContext::GetSwapchainImageCount();
		m_PoolSizes.push_back(poolSize);

		ImageUniform imageUniform{};
		imageUniform.Image = image;
		imageUniform.FinalLayout = finalLayout;
		imageUniform.Binding = binding;
		m_ImageUniforms.push_back(imageUniform);
	}

	void ComputeShader::AddUniformBuffer(const Ref<Buffer>& buffer)
	{
		uint32_t binding = m_LayoutBindings.size();

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		m_LayoutBindings.push_back(layoutBinding);

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSize.descriptorCount = VulkanContext::GetSwapchainImageCount();
		m_PoolSizes.push_back(poolSize);

		BufferUniform bufferUniform{};
		bufferUniform.Buffer = buffer;
		bufferUniform.Binding = binding;
		m_BufferUniforms.push_back(bufferUniform);
	}

	void ComputeShader::FinalizeUniformCreation()
	{
		uint32_t imageCount = VulkanContext::GetSwapchainImageCount();

		VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = m_LayoutBindings.size();
		layoutCreateInfo.pBindings = m_LayoutBindings.data();
		SNG_CHECK_VULKAN_RESULT(vkCreateDescriptorSetLayout(VulkanContext::GetDevice(), &layoutCreateInfo,
			nullptr, &m_DescriptorSetLayout), "Failed to create a Vulkan descriptor set layout");

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t)m_PoolSizes.size();
		poolInfo.pPoolSizes = m_PoolSizes.data();
		poolInfo.maxSets = imageCount;
		SNG_CHECK_VULKAN_RESULT(vkCreateDescriptorPool(VulkanContext::GetDevice(), &poolInfo,
			nullptr, &m_DescriptorPool), "Failed to create a Vulkan descriptor pool");

		std::vector<VkDescriptorSetLayout> layouts(imageCount, m_DescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = imageCount;
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(imageCount);
		SNG_CHECK_VULKAN_RESULT(vkAllocateDescriptorSets(VulkanContext::GetDevice(), &allocInfo,
			m_DescriptorSets.data()), "Failed to allocate Vulkan descriptor sets");

		CreatePipeline();
	}

	void ComputeShader::Run(uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ)
	{
		uint32_t currentFrame = VulkanContext::GetCurrentFrameIndex();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		SNG_CHECK_VULKAN_RESULT(vkBeginCommandBuffer(m_CommandBuffers[currentFrame], &beginInfo),
			"Failed to begin a Vulkan command buffer");

		for (auto& imageUniform : m_ImageUniforms)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo.imageView = imageUniform.Image->GetImageView();
			imageInfo.sampler = VK_NULL_HANDLE;

			VkWriteDescriptorSet writeDescriptor{};
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.pImageInfo = &imageInfo;
			writeDescriptor.dstSet = m_DescriptorSets[currentFrame];
			writeDescriptor.dstBinding = imageUniform.Binding;
			writeDescriptor.dstArrayElement = 0;
			vkUpdateDescriptorSets(VulkanContext::GetDevice(), 1, &writeDescriptor, 0, VK_NULL_HANDLE);

			Utils::TransitionImageLayout(m_CommandBuffers[currentFrame], imageUniform.Image,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
		}

		for (auto& bufferUniform : m_BufferUniforms)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = bufferUniform.Buffer->Raw();
			bufferInfo.range = bufferUniform.Buffer->GetSize();
			bufferInfo.offset = 0;

			VkWriteDescriptorSet writeDescriptor{};
			writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeDescriptor.descriptorCount = 1;
			writeDescriptor.pBufferInfo = &bufferInfo;
			writeDescriptor.dstSet = m_DescriptorSets[currentFrame];
			writeDescriptor.dstBinding = bufferUniform.Binding;
			writeDescriptor.dstArrayElement = 0;
			vkUpdateDescriptorSets(VulkanContext::GetDevice(), 1, &writeDescriptor, 0, VK_NULL_HANDLE);
		}

		vkCmdBindPipeline(m_CommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
		vkCmdBindDescriptorSets(m_CommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout,
			0, 1, &m_DescriptorSets[currentFrame], 0, nullptr);
		vkCmdDispatch(m_CommandBuffers[currentFrame], dispatchX, dispatchY, dispatchZ);

		for (auto& imageUniform : m_ImageUniforms)
		{
			Utils::TransitionImageLayout(m_CommandBuffers[currentFrame], imageUniform.Image,
				VK_IMAGE_LAYOUT_GENERAL, Utils::ConvertToVulkanImageLayout(imageUniform.FinalLayout));
		}

		vkEndCommandBuffer(m_CommandBuffers[currentFrame]);
		VulkanContext::SubmitComputeShader(m_CommandBuffers[currentFrame]);
	}

	void ComputeShader::CreatePipeline()
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStageInfo.module = m_ShaderModule;
		shaderStageInfo.pName = "main";

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		layoutInfo.setLayoutCount = 1;
		SNG_CHECK_VULKAN_RESULT(vkCreatePipelineLayout(VulkanContext::GetDevice(), &layoutInfo,
			nullptr, &m_PipelineLayout), "Failed to create a Vulkan pipeline layout");

		VkComputePipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.stage = shaderStageInfo;
		SNG_CHECK_VULKAN_RESULT(vkCreateComputePipelines(VulkanContext::GetDevice(), VK_NULL_HANDLE,
			1, &pipelineCreateInfo, nullptr, &m_Pipeline), "Failed to create a Vulkan compute pipeline");
	}
}
