#include "FrameSwapFences.h"

#include "VulkanErrorHandling.h"
#include "GraphicsWindow.h"

namespace Engine
{
	namespace Graphics
	{
		FrameSwapFences::~FrameSwapFences()
		{
			ReleaseSemaphores();
		}

		void FrameSwapFences::InitializeSemaphores(int maxQueueSize)
		{
			if (InitializedSemaphores)
				ReleaseSemaphores();

			FrameQueue.resize(maxQueueSize);

			InitializedSeparatePresentQueue = HasSeparatePresentQueue;

			auto const semaphoreCreateInfo = vk::SemaphoreCreateInfo();
			auto const fence_ci = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);

			for (size_t i = 0; i < FrameQueue.size(); ++i)
			{
				VK_CALL(Device.createFence, &fence_ci, nullptr, &FrameQueue[i].Fence);

				VK_CALL(Device.createSemaphore, &semaphoreCreateInfo, nullptr, &FrameQueue[i].ImageAcquired);
				VK_CALL(Device.createSemaphore, &semaphoreCreateInfo, nullptr, &FrameQueue[i].ImageCompleted);

				if (HasSeparatePresentQueue)
					VK_CALL(Device.createSemaphore, &semaphoreCreateInfo, nullptr, &FrameQueue[i].ImageOwnership);
			}

			InitializedSemaphores = true;
		}

		void FrameSwapFences::ReleaseSemaphores()
		{
			if (!InitializedSemaphores) return;

			for (size_t i = 0; i < FrameQueue.size(); ++i)
			{
				WaitForFence(false, (int)i);

				Device.destroyFence(FrameQueue[i].Fence, nullptr);

				Device.destroySemaphore(FrameQueue[i].ImageAcquired, nullptr);
				Device.destroySemaphore(FrameQueue[i].ImageCompleted, nullptr);

				if (InitializedSeparatePresentQueue)
					Device.destroySemaphore(FrameQueue[i].ImageOwnership, nullptr);
			}

			CurrentFrame = 0;

			InitializedSemaphores = false;
		}

		void FrameSwapFences::AdvanceFrame()
		{
			CurrentFrame = (CurrentFrame + 1) % (int)FrameQueue.size();
		}

		const vk::Semaphore& FrameSwapFences::GetAcquiredSemaphore(int index) const
		{
			if (index == -1) index = CurrentFrame;

			if (index < 0 || index >= (int)FrameQueue.size())
				throw "out of bounds";

			return FrameQueue[index].ImageAcquired;
		}

		const vk::Semaphore& FrameSwapFences::GetCompletedSemaphore(int index) const
		{
			if (index == -1) index = CurrentFrame;

			if (index < 0 || index >= (int)FrameQueue.size())
				throw "out of bounds";

			return FrameQueue[index].ImageCompleted;
		}

		const vk::Semaphore& FrameSwapFences::GetOwnershipSemaphore(int index) const
		{
			if (index == -1) index = CurrentFrame;

			if (index < 0 || index >= (int)FrameQueue.size())
				throw "out of bounds";

			return FrameQueue[index].ImageOwnership;
		}

		const vk::Fence& FrameSwapFences::GetFence(int index) const
		{
			if (index == -1) index = CurrentFrame;

			if (index < 0 || index >= (int)FrameQueue.size())
				throw "out of bounds";

			return FrameQueue[index].Fence;
		}

		void FrameSwapFences::WaitForFence(bool resetFence, int index)
		{
			if (index == -1) index = CurrentFrame;

			if (index < 0 || index >= (int)FrameQueue.size())
				throw "out of bounds";

			VK_CALL(Device.waitForFences, 1, &FrameQueue[index].Fence, VK_TRUE, UINT64_MAX);

			if (resetFence)
				VK_CALL(Device.resetFences, { FrameQueue[index].Fence });
		}
	}
}