#include "descriptors.h"
#include <cassert>

namespace ScorchEngine {
	// *************** Descriptor Set Layout Builder *********************

	SEDescriptorSetLayout::Builder& SEDescriptorSetLayout::Builder::addBinding(
		uint32_t binding,
		VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags,
		uint32_t count) {
		assert(bindings.count(binding) == 0 && "Binding already in use");
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		bindings[binding] = layoutBinding;
		return *this;
	}

	std::unique_ptr<SEDescriptorSetLayout> SEDescriptorSetLayout::Builder::build() const {
		return std::make_unique<SEDescriptorSetLayout>(seDevice, bindings);
	}

	// *************** Descriptor Set Layout *********************

	SEDescriptorSetLayout::SEDescriptorSetLayout(
		SEDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: seDevice(device), bindings(bindings) {
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		for (auto [binding, layoutBinding] : bindings) {
			setLayoutBindings.push_back(layoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
		
		if (vkCreateDescriptorSetLayout(seDevice.getDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	SEDescriptorSetLayout::~SEDescriptorSetLayout() {
		vkDestroyDescriptorSetLayout(seDevice.getDevice(), descriptorSetLayout, nullptr);
	}

	// *************** Descriptor Pool Builder *********************

	SEDescriptorPool::Builder& SEDescriptorPool::Builder::addPoolSize(
		VkDescriptorType descriptorType, uint32_t count) {
		poolSizes.push_back({ descriptorType, count });
		return *this;
	}

	SEDescriptorPool::Builder& SEDescriptorPool::Builder::setPoolFlags(
		VkDescriptorPoolCreateFlags flags) {
		poolFlags = flags;
		return *this;
	}
	SEDescriptorPool::Builder& SEDescriptorPool::Builder::setMaxSets(uint32_t count) {
		maxSets = count;
		return *this;
	}

	std::unique_ptr<SEDescriptorPool> SEDescriptorPool::Builder::build() const {
		return std::make_unique<SEDescriptorPool>(seDevice, maxSets, poolFlags, poolSizes);
	}

	// *************** Descriptor Pool *********************

	SEDescriptorPool::SEDescriptorPool(
		SEDevice& device,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes
	) : seDevice(device) {
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;
		
		if (vkCreateDescriptorPool(seDevice.getDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	SEDescriptorPool::~SEDescriptorPool() {
		vkDestroyDescriptorPool(seDevice.getDevice(), descriptorPool, nullptr);
	}

	bool SEDescriptorPool::allocateDescriptor(
		const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		if (vkAllocateDescriptorSets(seDevice.getDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
			SELOG_ERR("Descriptor pool overfilled! Allocation failed!");
			return false;
		}
		return true;
	}

	void SEDescriptorPool::freeDescriptors(const std::vector<VkDescriptorSet>& descriptors) const {
		vkFreeDescriptorSets(
			seDevice.getDevice(),
			descriptorPool,
			static_cast<uint32_t>(descriptors.size()),
			descriptors.data());
	}

	void SEDescriptorPool::resetPool() {
		vkResetDescriptorPool(seDevice.getDevice(), descriptorPool, 0);
	}

	// *************** Descriptor Writer *********************

	SEDescriptorWriter::SEDescriptorWriter(SEDescriptorSetLayout& setLayout, SEDescriptorPool& pool)
		: setLayout{ setLayout }, pool{ pool } {}

	SEDescriptorWriter& SEDescriptorWriter::writeBuffer(
		uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto& bindingDescription = setLayout.bindings[binding];

		assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;
		
		writes.push_back(write);
		return *this;
	}

	SEDescriptorWriter& SEDescriptorWriter::writeImage(
		uint32_t binding, VkDescriptorImageInfo* imageInfo) {
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

		auto& bindingDescription = setLayout.bindings[binding];

		assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	bool SEDescriptorWriter::build(VkDescriptorSet& set) {
		bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
		if (!success) {
			return false;
		}
		overwrite(set);
		return true;
	}

	void SEDescriptorWriter::overwrite(VkDescriptorSet& set) {
		for (auto& write : writes) {
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(pool.seDevice.getDevice(), writes.size(), writes.data(), 0, nullptr);
	}

}
