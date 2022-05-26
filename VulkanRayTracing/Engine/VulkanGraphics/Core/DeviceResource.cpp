#include "DeviceResource.h"

#include "GraphicsContext.h"

namespace Engine
{
	namespace Graphics
	{
		DeviceResource::DeviceResource(const std::shared_ptr<GraphicsContext>& context)
		{
			Context = context.get();

			if (Context != nullptr)
				Context->WatchResource(this);
		}

		DeviceResource::~DeviceResource()
		{
			if (Context != nullptr)
				Context->ResourceDied(this);
		}

		void DeviceResource::DeviceDestroyed()
		{
			FreeData();

			Context = nullptr;
		}

		void DeviceResource::AttachToContext(const std::shared_ptr<GraphicsContext>& context)
		{
			AttachToContext(context.get());
		}

		void DeviceResource::AttachToContext(GraphicsContext* context)
		{
			if (Context == nullptr)
			{
				Context = context;

				Context->WatchResource(this);
			}
		}

		vk::Device DeviceResource::GetDevice() const
		{
			if (Context == nullptr)
				return vk::Device();

			return Context->GetDevice().Device;
		}
	}
}