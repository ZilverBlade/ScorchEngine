#pragma once

#include "buffer.h"
#include <iostream>
#include <cassert>

namespace ScorchEngine {
	SEBuffer::SEBuffer(
		SEDevice& device,
		VkDeviceSize instanceSize,
		uint32_t instanceCount,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize minOffsetAlignment
	) : seDevice(device),
		instanceSize( instanceSize ),
		instanceCount( instanceCount ),
		usageFlags( usageFlags ),
		memoryPropertyFlags( memoryPropertyFlags ) {
		alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
		bufferSize = alignmentSize * instanceCount;

		device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
	}
	
	SEBuffer::~SEBuffer() {
		unmap();
		vkDestroyBuffer(seDevice.getDevice(), buffer, nullptr);
		vkFreeMemory(seDevice.getDevice(), memory, nullptr);
	}
	
	VkDeviceSize SEBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
		if (minOffsetAlignment > 0) {
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}
		return instanceSize;
	}

	VkResult SEBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
		assert(buffer && memory && "Called map on buffer before create");
		return vkMapMemory(seDevice.getDevice(), memory, offset, size, 0, &mapped);
	}
	
	void SEBuffer::unmap() {
		if (mapped) {
			vkUnmapMemory(seDevice.getDevice(), memory);
			mapped = nullptr;
		}
	}
	
	void SEBuffer::writeToBuffer(const void* data, VkDeviceSize offset, VkDeviceSize size) {
		assert(mapped && "Cannot copy to unmapped buffer");
	
		char* memOffset = (char*)mapped;
		memOffset += offset;
		memcpy(memOffset, data, size == VK_WHOLE_SIZE ? bufferSize - offset : size);
	}

	VkResult SEBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(seDevice.getDevice(), 1, &mappedRange);
	}

	VkResult SEBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(seDevice.getDevice(), 1, &mappedRange);
	}

	VkDescriptorBufferInfo SEBuffer::getDescriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
		return VkDescriptorBufferInfo{
			buffer,
			offset,
			size,
		};
	}

	void SEBuffer::writeToIndex(const void* data, int index) {
		writeToBuffer(data, instanceSize, index * alignmentSize);
	}

	VkResult SEBuffer::flushIndex(int index) {
		return flush(alignmentSize, index * alignmentSize);
	}
	

	VkDescriptorBufferInfo SEBuffer::getDescriptorInfoForIndex(int index) {
		return getDescriptorInfo(alignmentSize, index * alignmentSize);
	}

	VkResult SEBuffer::invalidateIndex(int index) {
		return invalidate(alignmentSize, index * alignmentSize);
	}
}
