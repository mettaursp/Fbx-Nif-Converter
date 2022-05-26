#pragma once

#include "VulkanSupport.h"

namespace Engine
{
	namespace Graphics
	{
		struct FrameSemaphores
		{
			vk::Fence Fence;
			vk::Semaphore ImageAcquired;
			vk::Semaphore ImageCompleted;
			vk::Semaphore ImageOwnership;
		};

		class FrameSwapFences
		{
		public:
			~FrameSwapFences();

			vk::Device Device;
			bool HasSeparatePresentQueue = false;

			void InitializeSemaphores(int maxQueueSize);
			void ReleaseSemaphores();

			int GetFrameQueueSize() const { return (int)FrameQueue.size(); }

			void AdvanceFrame();
			int GetCurrentFrame() const { return CurrentFrame; }

			const vk::Semaphore& GetAcquiredSemaphore(int index = -1) const;
			const vk::Semaphore& GetCompletedSemaphore(int index = -1) const;
			const vk::Semaphore& GetOwnershipSemaphore(int index = -1) const;
			const vk::Fence& GetFence(int index = -1) const;
			void WaitForFence(bool resetFence, int index = -1);

		private:
			bool InitializedSemaphores = false;
			bool InitializedSeparatePresentQueue = false;

			int CurrentFrame = 0;

			std::vector<FrameSemaphores> FrameQueue;
		};
	}
}