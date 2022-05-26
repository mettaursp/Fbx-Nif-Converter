#include "VulkanErrorHandling.h"

import <iostream>;
#include <vulkan/vulkan.hpp>

namespace Engine
{
	namespace Graphics
	{
		std::source_location NoThrowLocation;

		vk::Result CheckError(vk::Result result, const char* functionName, const char* message, const std::source_location& caller)
		{
			if (result != vk::Result::eSuccess)
			{
				std::cout << caller.file_name() << " [" << caller.line() << "]: " << caller.function_name() << ": vulkan error " << int(result) << " '" << vk::to_string(result) << "'";

				if (functionName)
				{
					std::cout << " from " << functionName;
				}

				std::cout << std::endl;

				if (&caller != &NoThrowLocation)
					throw VulkanException(result, functionName, message ? message : "vulkan error");
			}

			return result;
		}

		VulkanException::VulkanException(vk::Result result, const char* functionName, const char* message, const std::source_location& caller) throw()
		{
			Error = result;
			FunctionName = functionName;
			Message = message;
			Caller = caller;

			std::stringstream error;

			error << Caller.file_name() << " [" << Caller.line() << "]: " << Caller.function_name() << ": vulkan error " << int(Error) << " '" << vk::to_string(Error) << "': " << Message;

			ErrorMessage = error.str();
		}

		const char* VulkanException::what() throw()
		{
			return ErrorMessage.c_str();
		}
	}
}