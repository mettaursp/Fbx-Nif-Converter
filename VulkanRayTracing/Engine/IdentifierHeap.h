#pragma once

import <forward_list>;
import <vector>;

template <typename T>
class IDHeap
{
public:
	struct Node
	{
		Node() {}
		Node(const Node& other) : ID(other.ID)
		{
			new (reinterpret_cast<T*>(Data)) T(*reinterpret_cast<const T*>(other.Data));
		}
		Node(Node&& original) : ID(original.ID)
		{
			new (reinterpret_cast<T*>(Data)) T(std::move(*reinterpret_cast<T*>(original.Data)));
		}

		T& GetData();
		const T& GetData() const;
		int GetID() const;

		Node& operator=(const Node& other)
		{
			ID = other.ID;
			*reinterpret_cast<T*>(Data) = *reinterpret_cast<const T*>(other.Data);
		
			return *this;
		}

		friend class IDHeap<T>;

	private:
		int ID = 0;
		char Data[sizeof(T)] = { 0 };
	};

	~IDHeap();

	template <typename... Args>
	int RequestID(Args&&... constructorArguments);
	void Release(int id);
	int Size() const;
	int AllocatedIDs() const;
	Node& GetNode(int id);
	const Node& GetNode(int id) const;
	bool NodeAllocated(int id) const;
	void Clear();

private:
	struct HeapNode
	{
		bool Used = false;
		bool ChildrenUsed = false;

		HeapNode() {}
		HeapNode(HeapNode&& original) noexcept : Used(original.Used), ChildrenUsed(original.ChildrenUsed), Item(std::move(original.Item)) {}
		HeapNode(HeapNode& other) : Used(other.Used), ChildrenUsed(other.ChildrenUsed), Item(other.Item) {}

		//HeapNode() {}
		//HeapNode(const HeapNode& other) : Used(other.Used), ChildrenUsed(other.ChildrenUsed), Item(other.Item) {}
		//HeapNode(HeapNode&& original) : Item(std::move(original.Item)), Used(original.Used), ChildrenUsed(original.ChildrenUsed) {}
		//
		HeapNode& operator=(const HeapNode& other)
		{
			Used = other.Used;
			ChildrenUsed = other.ChildrenUsed;
			Item = other.Item;
		
			return *this;
		}

		Node Item;
	};

	typedef std::vector<HeapNode> NodeHeap;

	NodeHeap Nodes;
	int Allocated = 0;

	int Allocate();
};

template <typename T>
IDHeap<T>::~IDHeap()
{
	Clear();
}

template <typename T>
template <typename... Args>
int IDHeap<T>::RequestID(Args&&... constructorArguments)
{
	int id = Allocate();

	Nodes[id].Used = true;

	if (id > 0)
	{
		int currentID = (id - 1) / 2;

		while (currentID >= 0)
		{
			int left = 2 * currentID + 1;
			int right = 2 * currentID + 2;

			Nodes[currentID].ChildrenUsed = (left >= int(Nodes.size()) || Nodes[left].ChildrenUsed) && (right >= int(Nodes.size()) || Nodes[right].ChildrenUsed);

			if (currentID == 0)
				currentID = -1;
			else
				currentID = (currentID - 1) / 2;
		}
	}

	::new (reinterpret_cast<T*>(Nodes[id].Item.Data)) T(std::forward<Args>(constructorArguments)...);

	return id;
}

template <typename T>
void IDHeap<T>::Release(int id)
{
	if (id >= Nodes.size() || !Nodes[id].Used)
		return;

	reinterpret_cast<T*>(Nodes[id].Item.Data)->~T();

	Nodes[id].Used = false;

	if (id == 0)
		return;

	id = (id - 1) / 2;

	while (id >= 0)
	{
		Nodes[id].ChildrenUsed = false;

		if (id == 0)
			id = -1;
		else
			id = (id - 1) / 2;
	}
}

template <typename T>
int IDHeap<T>::Allocate()
{
	if (Nodes.size() > 0 && !Nodes[0].Used)
		return 0;
	if (Nodes.size() > 0 && !Nodes[0].ChildrenUsed)
	{
		int id = 0;

		while (Nodes[id].Used)
		{
			int left = 2 * id + 1;
			int right = 2 * id + 2;

			if (left < int(Nodes.size()) && (!Nodes[left].Used || !Nodes[left].ChildrenUsed))
				id = left;
			else if (right < int(Nodes.size()) && (!Nodes[right].Used || !Nodes[right].ChildrenUsed))
				id = right;
			else
				throw "shts fucked";
		}

		return id;
	}
	else
	{
		int id = int(Nodes.size());

		Nodes.push_back(HeapNode());

		Nodes[id].Item.ID = id;
		Nodes[id].ChildrenUsed = true;

		return id;
	}
}

template <typename T>
int IDHeap<T>::Size() const
{
	return int(Nodes.size());
}

template <typename T>
int IDHeap<T>::AllocatedIDs() const
{
	return Allocated;
}

template <typename T>
int IDHeap<T>::Node::GetID() const
{
	return ID;
}

template <typename T>
T& IDHeap<T>::Node::GetData()
{
	return *reinterpret_cast<T*>(Data);
}

template <typename T>
const T& IDHeap<T>::Node::GetData() const
{
	return *reinterpret_cast<const T*>(Data);
}

template <typename T>
typename IDHeap<T>::Node& IDHeap<T>::GetNode(int id)
{
	if (id >= 0 && id < int(Nodes.size()))
	{
		if (Nodes[id].Used)
			return Nodes[id].Item;
		else
			throw "Attempt to access unallocated ID";
	}
	else
		throw "Attempt to access out of bounds ID";
}

template <typename T>
const typename IDHeap<T>::Node& IDHeap<T>::GetNode(int id) const
{
	if (id >= 0 && id < int(Nodes.size()))
	{
		if (Nodes[id].Used)
			return Nodes[id].Item;
		else
			throw "Attempt to access unallocated ID";
	}
	else
		throw "Attempt to access out of bounds ID";
}

template <typename T>
bool IDHeap<T>::NodeAllocated(int id) const
{
	if (id >= 0 && id < int(Nodes.size()))
		return Nodes[id].Used;
	else
		throw "Attempt to access out of bounds ID";
}

template <typename T>
void IDHeap<T>::Clear()
{
	for (int i = 0; i < Size(); ++i)
	{
		if (NodeAllocated(i))
			reinterpret_cast<T*>(Nodes[i].Item.Data)->~T();

		Nodes[i].ChildrenUsed = 2 * i + 2 < Size();
		Nodes[i].Used = false;
	}
}