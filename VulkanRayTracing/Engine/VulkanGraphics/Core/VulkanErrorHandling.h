#pragma once

import <source_location>;
import <string>;

#define DEBUG_VULKAN 1

#if DEBUG_VULKAN
#define VK_CALL(function, ...) Graphics::CheckError(function(__VA_ARGS__), #function)
#define VK_PROCESS_CALL(handler, function, ...) Graphics::CheckError(handler(function(__VA_ARGS__)), #function)
#define VK_LOOSE_CALL(function, ...) Graphics::CheckError(function(__VA_ARGS__), #function, nullptr, NoThrowLocation)
#else
#define VK_CALL(function, ...) function(__VA_ARGS__)
#define VK_PROCESS_CALL(function, ...) function(__VA_ARGS__)
#define VK_LOOSE_CALL(function, ...) function(__VA_ARGS__)
#endif

namespace vk
{
	enum class Result;
}

namespace Engine
{
	namespace Graphics
	{
		extern std::source_location NoThrowLocation;

		vk::Result CheckError(vk::Result result, const char* functionName, const char* message = nullptr, const std::source_location& caller = std::source_location::current());

		class VulkanException : public std::exception
		{
		public:
			vk::Result Error;
			const char* FunctionName = nullptr;
			const char* Message = nullptr;
			std::source_location Caller;
			std::string ErrorMessage;

			VulkanException(vk::Result result, const char* functionName, const char* message, const std::source_location& caller = std::source_location::current()) throw();

			virtual const char* what() throw();
		};
	}
}