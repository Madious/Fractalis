
#pragma once
#include "Core/Core.h"


namespace SNG
{

	class Image
	{
	public:

		Image(uint32_t width, uint32_t height);
		~Image();

		void SetData(void* data);
		void Resize(uint32_t width, uint32_t height);

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
		VkImage Raw() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VkSampler GetSampler() const { return m_Sampler; }

	private:

		void AllocateMemory(uint32_t width, uint32_t height);
		void ReleaseMemory();

	private:

		uint32_t m_Width, m_Height;

		VkImage m_Image = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

		VkBuffer m_StaginBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_StaginBufferMemory = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}
