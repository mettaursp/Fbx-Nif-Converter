#pragma once

import <memory>;

#include "PageAllocator.h"

#define Engine_PoolPageSize 0x10000
#define Engine_PoolBlockSizeIncrements 0x20

namespace Engine
{
	template <int BlockSize = Engine_PoolBlockSizeIncrements>
	class GameObjectAllocators
	{
	public:
		static PageAllocator<BlockSize, Engine_PoolPageSize> Allocator;
	};

	template<int BlockSize>
	PageAllocator<BlockSize, Engine_PoolPageSize> GameObjectAllocators<BlockSize>::Allocator = PageAllocator<BlockSize, Engine_PoolPageSize>();

	template <typename T>
	class GameObjectAllocator
	{
	public:
		static const int RealSize = sizeof(T) + sizeof(void*);
		static const int Padding = RealSize % Engine_PoolBlockSizeIncrements;
		static const int SelectedPool = RealSize - Padding + int(Padding > 0) * Engine_PoolBlockSizeIncrements - sizeof(void*);

		typedef PageAllocator<SelectedPool, Engine_PoolPageSize> SelectedAllocator;

		static SelectedAllocator& Allocator;

		template <typename... Arguments>
		static std::shared_ptr<T> Create(Arguments&&... arguments)
		{
			T* object = Allocator.template Create<T>(arguments...);

			auto handle = std::shared_ptr<T>(object, Free);

			object->This = handle;

			return handle;
		}

	private:
		static void Free(T* data)
		{
			Allocator.template Destroy<T>(data);
		}
	};

	template <typename T>
	typename GameObjectAllocator<T>::SelectedAllocator& GameObjectAllocator<T>::Allocator = GameObjectAllocators<SelectedPool>::Allocator;
}

template <typename T>
using GameObjectAllocator = Engine::GameObjectAllocator<T>;
