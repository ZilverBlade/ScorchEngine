#pragma once
#include <scorch/vkapi/device.h>
#include <unordered_map>

namespace ScorchEngine {
	class SEDescriptorSetLayout {
	public:
		class Builder {
		public:
			Builder(SEDevice& device) : seDevice{ device } {}

			Builder& addBinding(
				uint32_t binding,
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t count = 1
			);
			std::unique_ptr<SEDescriptorSetLayout> build() const;
		private:
			SEDevice& seDevice;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};

		SEDescriptorSetLayout(SEDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~SEDescriptorSetLayout();

		SEDescriptorSetLayout(const SEDescriptorSetLayout&) = delete;
		SEDescriptorSetLayout& operator=(const SEDescriptorSetLayout&) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
	private:
		SEDevice& seDevice;
		VkDescriptorSetLayout descriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class SEDescriptorWriter;
	};

	class SEDescriptorPool {
	public:
		class Builder {
		public:
			Builder(SEDevice& device) : seDevice{ device } {}

			Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder& setMaxSets(uint32_t count);
			std::unique_ptr<SEDescriptorPool> build() const;

		private:
			SEDevice& seDevice;
			std::vector<VkDescriptorPoolSize> poolSizes{};
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};

		SEDescriptorPool(
			SEDevice& device,
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize>& poolSizes
		);
		~SEDescriptorPool();

		SEDescriptorPool(const SEDescriptorPool&) = delete;
		SEDescriptorPool& operator=(const SEDescriptorPool&) = delete;

		bool allocateDescriptor(
			const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

		void freeDescriptors(const std::vector<VkDescriptorSet>& descriptors) const;

		void resetPool();

	private:
		SEDevice& seDevice;
		VkDescriptorPool descriptorPool;

		friend class SEDescriptorWriter;
	};

	class SEDescriptorWriter {
	public:
		SEDescriptorWriter(SEDescriptorSetLayout& setLayout, SEDescriptorPool& pool);

		SEDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		SEDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		bool build(VkDescriptorSet& set);
		void overwrite(VkDescriptorSet& set);

	private:
		SEDescriptorSetLayout& setLayout;
		SEDescriptorPool& pool;
		std::vector<VkWriteDescriptorSet> writes;
	};
} 