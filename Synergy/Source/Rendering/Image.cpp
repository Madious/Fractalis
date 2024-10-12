
#include "Snpch.h"
#include "Image.h"
#include "API/VulkanContext.h"
#include "Core/Application.h"


namespace SNG
{

	namespace Utils
	{
		static uint32_t FindMemoryType(VkMemoryPropertyFlags properties, uint32_t type)
		{
			VkPhysicalDeviceMemoryProperties props;
			vkGetPhysicalDeviceMemoryProperties(VulkanContext::GetPhysicalDevice(), &props);

			for (uint32_t i = 0; i < props.memoryTypeCount; i++)
			{
				if (type & (1 << i) && (props.memoryTypes[i].propertyFlags & properties) == properties)
					return i;
			}

			return UINT_MAX;
		}
	}



	Image::Image(uint32_t width, uint32_t height)
	{
		AllocateMemory(width, height);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VulkanContext::GetCommandPool();
		allocInfo.commandBufferCount = VulkanContext::GetSwapchainImageCount();
		m_CommandBuffers.resize(VulkanContext::GetSwapchainImageCount());
		SNG_CHECK_VULKAN_RESULT(vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo,
			m_CommandBuffers.data()), "Failed to allocate a Vulkan command buffer");
	}

	Image::~Image()
	{
		ReleaseMemory();
	}

	void Image::SetData(void* data)
	{
		size_t dataSize = m_Width * m_Height * 4; // 4 = RGBA (1 byte per channel)

		/* Create stagin buffer */
		{
			if (!m_StaginBuffer)
			{
				VkBufferCreateInfo bufferInfo{};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				bufferInfo.size = dataSize;

				SNG_CHECK_VULKAN_RESULT(vkCreateBuffer(VulkanContext::GetDevice(), &bufferInfo,
					nullptr, &m_StaginBuffer), "Failed to create a Vulkan buffer");

				VkMemoryRequirements memRequirements;
				vkGetBufferMemoryRequirements(VulkanContext::GetDevice(), m_StaginBuffer, &memRequirements);

				VkMemoryAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = memRequirements.size;
				allocInfo.memoryTypeIndex = Utils::FindMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memRequirements.memoryTypeBits);

				SNG_CHECK_VULKAN_RESULT(vkAllocateMemory(VulkanContext::GetDevice(), &allocInfo,
					nullptr, &m_StaginBufferMemory), "Failed to allocate Vulkan buffer memory");

				SNG_CHECK_VULKAN_RESULT(vkBindBufferMemory(VulkanContext::GetDevice(), m_StaginBuffer,
					m_StaginBufferMemory, 0), "Failed to bind Vulkan buffer memory");
			}
		}

		/* Upload to buffer */
		{
			void* memData;
			vkMapMemory(VulkanContext::GetDevice(), m_StaginBufferMemory, 0, dataSize, 0, &memData);
			memcpy(memData, data, dataSize);

			VkMappedMemoryRange range{};
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.memory = m_StaginBufferMemory;
			range.size = dataSize;

			vkFlushMappedMemoryRanges(VulkanContext::GetDevice(), 1, &range);
			vkUnmapMemory(VulkanContext::GetDevice(), m_StaginBufferMemory);
		}

		/* Command buffer */
		{
			uint32_t currentFrame = VulkanContext::GetCurrentFrameIndex();

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			SNG_CHECK_VULKAN_RESULT(vkBeginCommandBuffer(m_CommandBuffers[currentFrame],
				&beginInfo), "Failed to begin a Vulkan command buffer");

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_Image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(m_CommandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			VkBufferImageCopy region{};
			region.imageExtent = { m_Width, m_Height, 1 };
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			vkCmdCopyBufferToImage(m_CommandBuffers[currentFrame], m_StaginBuffer,
				m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vkCmdPipelineBarrier(m_CommandBuffers[currentFrame], VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			vkEndCommandBuffer(m_CommandBuffers[currentFrame]);
			VulkanContext::PushCommandBuffer(m_CommandBuffers[currentFrame], false);
		}
	}

	void Image::Resize(uint32_t width, uint32_t height)
	{
		if (m_Width == width && m_Height == height)
			return;

		Application::Get()->SubmitFunction([this, width, height]()
		{
			ReleaseMemory();
			AllocateMemory(width, height);
		});
	}

	void Image::AllocateMemory(uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;

		/* Image and its memory */
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.extent = { m_Width, m_Height, 1 };
			imageInfo.arrayLayers = 1;
			imageInfo.mipLevels = 1;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

			SNG_CHECK_VULKAN_RESULT(vkCreateImage(VulkanContext::GetDevice(), &imageInfo,
				nullptr, &m_Image), "Failed to create a Vulkan image");

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(VulkanContext::GetDevice(), m_Image, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memRequirements.memoryTypeBits);

			SNG_CHECK_VULKAN_RESULT(vkAllocateMemory(VulkanContext::GetDevice(), &allocInfo,
				nullptr, &m_ImageMemory), "Failed to allocate Vulkan image memory");

			SNG_CHECK_VULKAN_RESULT(vkBindImageMemory(VulkanContext::GetDevice(), m_Image,
				m_ImageMemory, 0), "Failed to bind Vulkan image memory");
		}

		/* Image view */
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
			viewInfo.image = m_Image;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			SNG_CHECK_VULKAN_RESULT(vkCreateImageView(VulkanContext::GetDevice(), &viewInfo,
				nullptr, &m_ImageView), "Failed to create a Vulkan image view");
		}

		/* Sampler */
		{
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.minLod = -1000;
			samplerInfo.maxLod = 1000;
			samplerInfo.maxAnisotropy = 1.0f;

			SNG_CHECK_VULKAN_RESULT(vkCreateSampler(VulkanContext::GetDevice(), &samplerInfo,
				nullptr, &m_Sampler), "Failed to create a Vulkan sampler");
		}

		/* Descriptor Set */
		m_DescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Image::ReleaseMemory()
	{
		VulkanContext::Wait();
		vkDestroySampler(VulkanContext::GetDevice(), m_Sampler, nullptr);
		vkDestroyImageView(VulkanContext::GetDevice(), m_ImageView, nullptr);
		vkFreeMemory(VulkanContext::GetDevice(), m_ImageMemory, nullptr);
		vkDestroyImage(VulkanContext::GetDevice(), m_Image, nullptr);
		ImGui_ImplVulkan_RemoveTexture(m_DescriptorSet);

		if (m_StaginBuffer != VK_NULL_HANDLE)
		{
			vkFreeMemory(VulkanContext::GetDevice(), m_StaginBufferMemory, nullptr);
			vkDestroyBuffer(VulkanContext::GetDevice(), m_StaginBuffer, nullptr);
		}

		m_Image = VK_NULL_HANDLE;
		m_ImageMemory = VK_NULL_HANDLE;
		m_ImageView = VK_NULL_HANDLE;
		m_Sampler = VK_NULL_HANDLE;
		m_DescriptorSet = VK_NULL_HANDLE;
		m_StaginBuffer = VK_NULL_HANDLE;
		m_StaginBufferMemory = VK_NULL_HANDLE;
	}
}
