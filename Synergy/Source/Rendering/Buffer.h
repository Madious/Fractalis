
#pragma once
#include "Core/Core.h"


namespace SNG
{

	class Buffer
	{
	public:

		Buffer(size_t size);
		~Buffer();

		void Resize(size_t size);
		void SetData(void* data, size_t size);
		void ResetData();
		VkBuffer Raw() const { return m_Buffer; }
		size_t GetSize() const { return m_Size; }

	private:

		void CreateBuffer(size_t size);
		void Release();

	private:

		size_t m_Size;
		VkBuffer m_Buffer;
		VkDeviceMemory m_BufferMemory;
	};
}
