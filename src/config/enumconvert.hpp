#pragma once

#include "attachment.hpp"
#include "compute.hpp"
#include "pipeline.hpp"

#include <vulkan/vulkan.hpp>

namespace config {

inline vk::Format convertFormat(const Format &format, const vk::Format &rtFormat) {
	switch (format) {
	case config::Format::RenderTarget:
		return rtFormat;
	case config::Format::DepthBuffer:
		return vk::Format::eD32Sfloat;
	case config::Format::ShareColorAttachment:
		return rtFormat;
	case config::Format::SignedShareColorAttachment:
		return vk::Format::eR8G8B8A8Snorm;
	default:
		throw;
	}
}

inline vk::ImageLayout getImageLayoutFromFormat(const Format &format) {
	switch (format) {
	case config::Format::RenderTarget:
		return vk::ImageLayout::ePresentSrcKHR;
	case config::Format::DepthBuffer:
		return vk::ImageLayout::eDepthStencilAttachmentOptimal;
	case config::Format::ShareColorAttachment:
		return vk::ImageLayout::eShaderReadOnlyOptimal;
	case config::Format::SignedShareColorAttachment:
		return vk::ImageLayout::eShaderReadOnlyOptimal;
	default:
		throw;
	}
}

inline vk::ImageUsageFlags getImageUsageFromFormat(const config::Format &format) {
	switch (format) {
	case config::Format::DepthBuffer:
		return vk::ImageUsageFlagBits::eDepthStencilAttachment;
	case config::Format::ShareColorAttachment:
		return vk::ImageUsageFlagBits::eColorAttachment
			| vk::ImageUsageFlagBits::eInputAttachment
			| vk::ImageUsageFlagBits::eSampled;
	case config::Format::SignedShareColorAttachment:
		return vk::ImageUsageFlagBits::eColorAttachment
			| vk::ImageUsageFlagBits::eInputAttachment
			| vk::ImageUsageFlagBits::eSampled;
	default:
		throw;
	}
}

inline vk::ImageAspectFlags getImageAspectFromFormat(const config::Format &format) {
	switch (format) {
	case config::Format::RenderTarget:
		return vk::ImageAspectFlagBits::eColor;
	case config::Format::DepthBuffer:
		return vk::ImageAspectFlagBits::eDepth;
	case config::Format::ShareColorAttachment:
		return vk::ImageAspectFlagBits::eColor;
	case config::Format::SignedShareColorAttachment:
		return vk::ImageAspectFlagBits::eColor;
	default:
		throw;
	}
}

inline vk::DescriptorType convertDescriptorType(const DescriptorType &dt) {
	switch (dt) {
	case config::DescriptorType::Texture:
		return vk::DescriptorType::eSampledImage;
	case config::DescriptorType::Sampler:
		return vk::DescriptorType::eSampler;
	case config::DescriptorType::UniformBuffer:
		return vk::DescriptorType::eUniformBuffer;
	case config::DescriptorType::StorageBuffer:
		return vk::DescriptorType::eStorageBuffer;
	case config::DescriptorType::InputAttachment:
		return vk::DescriptorType::eInputAttachment;
	default:
		throw;
	}
}

inline vk::ShaderStageFlags convertShaderStages(const ShaderStages &ss) {
	switch (ss) {
	case config::ShaderStages::Vertex:
		return vk::ShaderStageFlagBits::eVertex;
	case config::ShaderStages::Fragment:
		return vk::ShaderStageFlagBits::eFragment;
	case config::ShaderStages::VertexAndFragment:
		return vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
	default:
		throw;
	}
}

inline vk::DescriptorType convertComputeDescriptorType(const ComputeDescriptorType &dt) {
	switch (dt) {
	case config::ComputeDescriptorType::Texture:
		return vk::DescriptorType::eSampledImage;
	case config::ComputeDescriptorType::Sampler:
		return vk::DescriptorType::eSampler;
	case config::ComputeDescriptorType::UniformBuffer:
		return vk::DescriptorType::eUniformBuffer;
	case config::ComputeDescriptorType::StorageBuffer:
		return vk::DescriptorType::eStorageBuffer;
	case config::ComputeDescriptorType::StorageImage:
		return vk::DescriptorType::eStorageImage;
	default:
		throw;
	}
}

} // namespace config
