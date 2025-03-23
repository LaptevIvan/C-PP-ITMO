#pragma once
#include "bucket_storage.hpp"

template< typename T >
class BucketStorage;

typedef enum type
{
	nil,
	thisBlock,
	AnotherBlock
} typeConnection;

template< typename T >
class Block
{
  public:
	using size_type = typename BucketStorage< T >::size_type;
	friend BucketStorage< T >;

	struct Node
	{
		T* object;
		Block< T >::Node *prevNode, *nextNode, *prevDel;
		size_type idArray;
		typeConnection leftConnectRemNode, rightConnectRemNode;
		Block< T >* parent;

		Node() noexcept :
			object(nullptr), prevNode(nullptr), nextNode(nullptr), prevDel(nullptr), idArray(0),
			leftConnectRemNode(nil), rightConnectRemNode(nil), parent(nullptr)
		{
		}

		void setValues(
			T* object,
			Block< T >::Node* prevNode,
			Block< T >::Node* nextNode,
			Block< T >::Node* prevDel,
			size_type idArray,
			typeConnection leftConnectRemNode,
			typeConnection rightConnectRemNode,
			Block< T >* parent) noexcept;

		~Node() noexcept;
	};
	using Node = Block< T >::Node;

  private:
	size_type size, capacity;
	Block< T >*prevBlock, *prevBlockFreePlace, *nextBlock, *nextBlockFreePlace;
	Node *data, *empty, *first, *last, *deletedNodes;

	Block(size_type size,
		  size_type capacity,
		  Node* data,
		  size_type id,
		  Block< T >* prevBlock = nullptr,
		  Block< T >* prevBlockFreePlace = nullptr,
		  Block< T >* nextBlock = nullptr,
		  Block< T >* nextBlockFreePlace = nullptr);

	typeConnection reconnecting(Node* removed_val, Node* neighbour, bool prev) noexcept;

  public:
	const size_type id;

	Block(Block< T >* prevBlock, Block< T >* prevBlockFreePlace, Block< T >* nextBlock, Block< T >* nextBlockFreePlace, size_type capacity, size_type id);

	Block(const Block< T >& other);

	typename BucketStorage< T >::iterator insert(T* value) noexcept;

	bool erase(size_type idNode) noexcept;

	~Block() noexcept;
};

template< typename T >
void Block< T >::Node::setValues(
	T* object,
	Block< T >::Node* prevNode,
	Block< T >::Node* nextNode,
	Block< T >::Node* prevDel,
	Block::size_type idArray,
	typeConnection leftConnectRemNode,
	typeConnection rightConnectRemNode,
	Block< T >* parent) noexcept
{
	this->object = object;
	this->prevNode = prevNode;
	this->nextNode = nextNode;
	this->prevDel = prevDel;
	this->idArray = idArray;
	this->leftConnectRemNode = leftConnectRemNode;
	this->rightConnectRemNode = rightConnectRemNode;
	this->parent = parent;

	if (prevNode)
		prevNode->nextNode = this;
	if (nextNode)
		nextNode->prevNode = this;
}

template< typename T >
Block< T >::Node::~Node() noexcept
{
	delete object;
}

template< typename T >
typeConnection Block< T >::reconnecting(Block::Node* removed_val, Block::Node* neighbour, bool prev) noexcept
{
	if (!neighbour)
		return nil;

	if (prev)
		neighbour->prevNode = removed_val;
	else
		neighbour->nextNode = removed_val;
	return neighbour->parent->id == id ? thisBlock : AnotherBlock;
}

template< typename T >
Block< T >::Block(
	size_type size,
	size_type capacity,
	Node* data,
	size_type id,
	Block< T >* prevBlock,
	Block< T >* prevBlockFreePlace,
	Block< T >* nextBlock,
	Block< T >* nextBlockFreePlace) :
	size(size), capacity(capacity), prevBlock(prevBlock), prevBlockFreePlace(prevBlockFreePlace), nextBlock(nextBlock),
	nextBlockFreePlace(nextBlockFreePlace), data(data), id(id)
{
}

template< typename T >
Block< T >::Block(Block< T >* prevBlock, Block< T >* prevBlockFreePlace, Block< T >* nextBlock, Block< T >* nextBlockFreePlace, Block::size_type capacity, Block::size_type id) :
	Block< T >::Block(0, capacity, new Node[capacity], id, prevBlock, prevBlockFreePlace, nextBlock, nextBlockFreePlace)
{
	empty = first = last = data;
	deletedNodes = nullptr;
	if (prevBlock)
		prevBlock->nextBlock = this;
	if (nextBlock)
		nextBlock->prevBlock = this;
	if (prevBlockFreePlace)
		prevBlockFreePlace->nextBlockFreePlace = this;
	if (nextBlockFreePlace)
		nextBlockFreePlace->prevBlockFreePlace = this;
}

template< typename T >
Block< T >::Block(const Block< T >& other) :
	Block< T >::Block(other.size, other.capacity, new Node[other.capacity], other.id)
{
	try
	{
		Node* otherData = other.data;
		Node *curNode, *otherCurNode, *prevOther, *nextOther, *prevDelOther;
		for (size_type i = 0; i < capacity; ++i)
		{
			curNode = &data[i];
			otherCurNode = &otherData[i];
			prevOther = otherCurNode->prevNode;
			nextOther = otherCurNode->nextNode;
			prevDelOther = otherCurNode->prevDel;
			const T* otherObj = otherCurNode->object;
			curNode->setValues(
				otherObj ? new T(*otherObj) : nullptr,
				!prevOther || prevOther->parent->id != other.id ? nullptr : &data[prevOther->idArray],
				!nextOther || nextOther->parent->id != other.id ? nullptr : &data[nextOther->idArray],
				prevDelOther ? &data[prevDelOther->idArray] : nullptr,
				i,
				otherCurNode->leftConnectRemNode,
				otherCurNode->rightConnectRemNode,
				this);
		}
		deletedNodes = other.deletedNodes ? &data[other.deletedNodes->idArray] : nullptr;
		size_type diff = other.empty - otherData;
		empty = diff < capacity ? &data[diff] : nullptr;
		first = &data[other.first->idArray];
		last = &data[other.last->idArray];
	} catch (...)
	{
		delete[] data;
		throw;
	}
}

template< typename T >
typename BucketStorage< T >::iterator Block< T >::insert(T* value) noexcept
{
	if (deletedNodes)
	{
		size_type ind = deletedNodes->idArray;

		typeConnection left = deletedNodes->leftConnectRemNode;
		typeConnection right = deletedNodes->rightConnectRemNode;

		Node* leftConnectNode =
			left == thisBlock ? deletedNodes->prevNode : (left == AnotherBlock && prevBlock ? prevBlock->last : nullptr);
		Node* rightConnectNode =
			right == thisBlock ? deletedNodes->nextNode : (right == AnotherBlock && nextBlock ? nextBlock->first : nullptr);

		deletedNodes = deletedNodes->prevDel;
		data[ind].setValues(value, leftConnectNode, rightConnectNode, nullptr, ind, nil, nil, this);
		if (first->idArray > ind)
			first = &data[ind];
		else if (last->idArray < ind)
			last = &data[ind];
		++size;
		return typename BucketStorage< T >::iterator(&data[ind]);
	}
	else
	{
		Node* prev = size ? empty - 1 : (prevBlock ? prevBlock->last : nullptr);
		Node* next = nextBlock ? nextBlock->first : nullptr;
		empty->setValues(value, prev, next, nullptr, size++, nil, nil, this);
		last = empty;
		return typename BucketStorage< T >::iterator(empty++);
	}
}

template< typename T >
bool Block< T >::erase(Block::size_type idNode) noexcept
{
	delete data[idNode].object;
	data[idNode].object = nullptr;
	Node* removeNode = &data[idNode];

	Node* rightNode = removeNode->nextNode;
	typeConnection leftConnect = reconnecting(removeNode->prevNode, rightNode, true);

	Node* leftNode = removeNode->prevNode;
	typeConnection rightConnect = reconnecting(removeNode->nextNode, leftNode, false);

	removeNode->rightConnectRemNode = leftConnect;
	removeNode->leftConnectRemNode = rightConnect;

	removeNode->prevDel = deletedNodes;
	deletedNodes = removeNode;
	if (size == 1)
		first = last = nullptr;
	else
	{
		if (first == removeNode)
			first = removeNode->nextNode;
		else if (last == removeNode)
			last = removeNode->prevNode;
	}
	return size-- == capacity;
}

template< typename T >
Block< T >::~Block() noexcept
{
	delete[] data;
	if (prevBlock)
		prevBlock->nextBlock = nextBlock;
	if (prevBlockFreePlace)
		prevBlockFreePlace->nextBlockFreePlace = nextBlockFreePlace;
	if (nextBlock)
		nextBlock->prevBlock = prevBlock;
	if (nextBlockFreePlace)
		nextBlockFreePlace->prevBlockFreePlace = prevBlockFreePlace;
}
