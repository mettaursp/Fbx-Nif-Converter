#pragma once

#include <Engine/Objects/Object.h>
#include "VulkanSupport.h"

namespace Engine
{
	namespace Graphics
	{
		class GraphicsContext;

		class DeviceResource : public Object
		{
		public:
			DeviceResource(const std::shared_ptr<GraphicsContext>& context = nullptr);
			~DeviceResource();

			virtual void FreeData() {}

			void DeviceDestroyed();
			GraphicsContext* GetContext() const { return Context; }
			void AttachToContext(const std::shared_ptr<GraphicsContext>& context);
			void AttachToContext(GraphicsContext* context);
			vk::Device GetDevice() const;

			friend class GraphicsContext;

		private:
			size_t ArrayIndex = 0;
			GraphicsContext* Context = nullptr;
		};
	}
}