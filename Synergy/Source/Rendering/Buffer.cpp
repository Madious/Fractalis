
#include "Snpch.h"
#include "Buffer.h"
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


	Buffer::Buffer(size_t size)
	{
		CreateBuffer(size);
	}

	Buffer::~Buffer()
	{
		Release();
	}

	void Buffer::Resize(size_t size)
	{
		if (m_Size == size) return;

		Application::Get()->SubmitFunction([this, size]()
		{
			Release();
			CreateBuffer(size);
		});
	}

	void Buffer::SetData(void* data, size_t size)
	{
		SNG_ASSERT(size <= m_Size);

		void* memData;
		vkMapMemory(VulkanContext::GetDevice(), m_BufferMemory, 0, size, 0, &memData);
		memcpy(memData, data, size);

		VkMappedMemoryRange range{};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.memory = m_BufferMemory;
		range.size = size;

		vkFlushMappedMemoryRanges(VulkanContext::GetDevice(), 1, &range);
		vkUnmapMemory(VulkanContext::GetDevice(), m_BufferMemory);
	}

	void Buffer::ResetData()
	{
		void* memData;
		vkMapMemory(VulkanContext::GetDevice(), m_BufferMemory, 0, m_Size, 0, &memData);
		memset(memData, 0, m_Size);

		VkMappedMemoryRange range{};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.memory = m_BufferMemory;
		range.size = m_Size;

		vkFlushMappedMemoryRanges(VulkanContext::GetDevice(), 1, &range);
		vkUnmapMemory(VulkanContext::GetDevice(), m_BufferMemory);
	}

	void Buffer::CreateBuffer(size_t size)
	{
		m_Size = size;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferInfo.size = size;
		SNG_CHECK_VULKAN_RESULT(vkCreateBuffer(VulkanContext::GetDevice(), &bufferInfo,
			nullptr, &m_Buffer), "Failed to create a Vulkan buffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanContext::GetDevice(), m_Buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memRequirements.memoryTypeBits);

		SNG_CHECK_VULKAN_RESULT(vkAllocateMemory(VulkanContext::GetDevice(), &allocInfo,
			nullptr, &m_BufferMemory), "Failed to allocate Vulkan buffer memory");

		SNG_CHECK_VULKAN_RESULT(vkBindBufferMemory(VulkanContext::GetDevice(), m_Buffer,
			m_BufferMemory, 0), "Failed to bind Vulkan buffer memory");
	}

	void Buffer::Release()
	{
		vkFreeMemory(VulkanContext::GetDevice(), m_BufferMemory, nullptr);
		vkDestroyBuffer(VulkanContext::GetDevice(), m_Buffer, nullptr);
	}
}
