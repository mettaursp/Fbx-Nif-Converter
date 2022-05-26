#pragma once

import <forward_list>;
import <string>;

// Here be dragons! Abandon all hope ye who enter here!
// this is all mostly book keeping shit

class BaseAllocator
{
public:
	template <typename T, typename... Args>
	T* Create(Args&&... args);

	template <typename T>
	void Destroy(T* object);

	virtual void* Allocate() = 0;
	virtual void Free(void* data) = 0;
};

template<int blockSize = 128, int pageSize = 4096>
class PageAllocator : public BaseAllocator
{
public:
	void* Allocate();
	void Free(void* data);

private:

	struct Page;

	struct BlockMarker
	{
		BlockMarker* Previous;
		BlockMarker* Next;

		void InsertBefore(BlockMarker* block);
		void InsertAfter(BlockMarker* block);
		void InsertBetween(BlockMarker* previous, BlockMarker* next);
		void ClearConnections();
	};

	struct Block
	{
		Page* Owner = nullptr;

		char Memory[blockSize > sizeof(nullptr) ? blockSize : sizeof(nullptr)] = {};
	};

	static const int BlocksPerPage = pageSize / sizeof(Block);

	struct Page
	{
		Block Blocks[BlocksPerPage];

		PageAllocator* Owner = nullptr;

		bool Open = true;
		Page* Previous = nullptr;
		Page* Next = nullptr;

		BlockMarker* First;

		int OpenBlocks = BlocksPerPage;

		Page(PageAllocator* owner, Page* previous = nullptr, Page* next = nullptr);

		Block* Fetch();
		void Release(Block* block);
		void InsertBefore(Page* page);
		void InsertAfter(Page* page);
		void InsertBetween(Page* previous, Page* next);
		void ClearConnections();
	};

	Page FirstPage = Page(this);

	Page* FullPages = nullptr;
	Page* OpenPages = &FirstPage;

	int OpenPageCount = 1;
	int FullPageCount = 0;
	int OpenBlocks = BlocksPerPage;
	int UsedBlocks = 0;
};

template <typename T, int pageSize = 4096>
using ClassAllocator = PageAllocator<sizeof(T), pageSize>;

template<int blockSize, int pageSize>
void* PageAllocator<blockSize, pageSize>::Allocate()
{
	Block* newBlock = OpenPages->Fetch();

	if (!OpenPages->Open)
	{
		Page* closing = OpenPages;

		OpenPages = OpenPages->Next;

		closing->InsertBefore(FullPages);
		closing->Open = false;

		FullPages = closing;

		--OpenPageCount;
		++FullPageCount;

		if (OpenPages == nullptr)
		{
			OpenPages = new Page(this);

			OpenPageCount = 1;
			OpenBlocks += BlocksPerPage;
		}
	}

	char* buffer = newBlock->Memory;

	--OpenBlocks;
	++UsedBlocks;

	return buffer;
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::Free(void* data)
{
	Block* block = reinterpret_cast<Block*>(reinterpret_cast<char*>(data) - sizeof(Block*));

	if (block->Owner->Owner != this)
		throw std::string("bad block free!"); // you pressed the bad free button. you shouldn't have done that

	Page* page = block->Owner;

	page->Release(block);

	if (!page->Open)
	{
		if (page == FullPages)
			FullPages = FullPages->Next;

		page->InsertBefore(OpenPages);
		page->Open = true;

		OpenPages = page;

		++OpenPageCount;
		--FullPageCount;
	}

	++OpenBlocks;
	--UsedBlocks;
}

template<int blockSize, int pageSize>
PageAllocator<blockSize, pageSize>::Page::Page(PageAllocator* owner, Page* previous, Page* next) : Owner(owner), Previous(previous), Next(next)
{
	BlockMarker* previousBlock = nullptr;

	for (int i = 0; i < BlocksPerPage; ++i)
	{
		Block* block = Blocks + i;
		BlockMarker* marker = reinterpret_cast<BlockMarker*>(block);

		marker->Previous = previousBlock;

		if (previousBlock != nullptr)
			previousBlock->Next = marker;

		previousBlock = marker;
	}

	if (previousBlock != nullptr)
		previousBlock->Next = nullptr;

	if (previous != nullptr)
		previous->Next = this;

	if (next != nullptr)
		next->Previous = this;

	First = reinterpret_cast<BlockMarker*>(Blocks);
}

template<typename T, typename ...Args>
T* BaseAllocator::Create(Args&&... args)
{
	return ::new (Allocate()) T(std::forward<Args>(args)...);
}

template<typename T>
void BaseAllocator::Destroy(T* object)
{
	object->~T();

	Free(object);
}

template<int blockSize, int pageSize>
typename PageAllocator<blockSize, pageSize>::Block* PageAllocator<blockSize, pageSize>::Page::Fetch()
{
	BlockMarker* marker = First;
	Block* block = reinterpret_cast<Block*>(marker);

	First = First->Next;
	block->Owner = this;

	if (First != nullptr)
		First->Previous = nullptr;
	else
		Open = false;

	--OpenBlocks;

	return block;
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::Page::Release(Block* block)
{
	BlockMarker* marker = reinterpret_cast<BlockMarker*>(block);

	if (First != nullptr)
		First->Previous = marker;

	marker->Previous = nullptr;
	marker->Next = First;

	First = marker;

	++OpenBlocks;
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::BlockMarker::InsertBefore(BlockMarker* block)
{
	InsertBetween(block->Previous, block);
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::BlockMarker::InsertAfter(BlockMarker* block)
{
	InsertBetween(block, block->Next);
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::BlockMarker::InsertBetween(BlockMarker* previous, BlockMarker* next)
{
	ClearConnections();

	if (previous != nullptr)
		previous->Next = this;

	if (next != nullptr)
		next->Previous = this;

	Previous = previous;
	Next = next;
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::BlockMarker::ClearConnections()
{
	if (Previous != nullptr)
		Previous->Next = Next;

	if (Next != nullptr)
		Next->Previous = Previous;

	Previous = nullptr;
	Next = nullptr;
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::Page::InsertBefore(Page* page)
{
	InsertBetween(page ? page->Previous : nullptr, page);
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::Page::InsertAfter(Page* page)
{
	InsertBetween(page, page ? page->Next : nullptr);
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::Page::InsertBetween(Page* previous, Page* next)
{
	ClearConnections();

	if (previous != nullptr)
		previous->Next = this;

	if (next != nullptr)
		next->Previous = this;

	Previous = previous;
	Next = next;
}

template<int blockSize, int pageSize>
void PageAllocator<blockSize, pageSize>::Page::ClearConnections()
{
	if (Previous != nullptr)
		Previous->Next = Next;

	if (Next != nullptr)
		Next->Previous = Previous;

	Previous = nullptr;
	Next = nullptr;
}
