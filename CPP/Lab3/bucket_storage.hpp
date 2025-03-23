#pragma once

#include "Block.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>

template< typename T >
class BucketStorage
{
  public:
	class const_iterator
	{
		friend class BucketStorage< T >;

	  protected:
		typename Block< T >::Node* ptrNode;

	  public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = const T*;
		using reference = const T&;
		using iterator_category = std::bidirectional_iterator_tag;

		explicit const_iterator(typename Block< T >::Node* pointerNode) noexcept;

		const_iterator& operator++();

		const_iterator operator++(int);

		bool operator==(const const_iterator& other) const noexcept;

		bool operator<(const const_iterator& other) const noexcept;

		bool operator<=(const const_iterator& other) const noexcept;

		const_iterator& operator=(const typename BucketStorage< T >::const_iterator& other) noexcept;

		const_iterator& operator--();

		const_iterator operator--(int);

		bool operator>(const const_iterator& other) const noexcept;

		bool operator>=(const const_iterator& other) const noexcept;

		bool operator!=(const const_iterator& other) const noexcept;
	};

	class iterator : public const_iterator
	{
		friend class BucketStorage< T >;

	  public:
		using pointer = T*;
		using reference = T&;

		explicit iterator(typename Block< T >::Node* val) noexcept;
		reference operator*() noexcept;
		pointer operator->() noexcept;
		iterator& operator--();
		iterator operator--(int);
	};

	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using iterator = BucketStorage< T >::iterator;
	using const_iterator = BucketStorage< T >::const_iterator;
	using difference_type = std::ptrdiff_t;
	using size_type = size_t;

  private:
	Block< T >*blockFreePlace, *endBlock, *headBlock, *tailBlock;
	size_type cntBlock, cntObject, capacityBlock;

	BucketStorage(
		size_type capacityBlock,
		size_type cntBlock,
		size_type cntObject = 0,
		Block< T >* endBlock = nullptr,
		Block< T >* headBlock = nullptr,
		Block< T >* tailBlock = nullptr,
		Block< T >* blockFreePlace = nullptr);

	iterator insertValue(value_type* value);

	void deleteListBlock(Block< T >** start, Block< T >* end) noexcept;

	void switchClear(bool all = false) noexcept;

	void connectBlock(Block< T >* blockLeft, Block< T >* blockRight) noexcept;

  public:
	explicit BucketStorage(size_type block_capacity = 64);

	BucketStorage(const BucketStorage< T >& other);

	BucketStorage< T >& operator=(const BucketStorage< T >& other);

	BucketStorage(BucketStorage< T >&& other) noexcept;

	BucketStorage< T >& operator=(BucketStorage< T >&& other) noexcept;

	iterator insert(const value_type& value);

	iterator insert(value_type&& value);

	iterator erase(const_iterator it);

	bool empty() const noexcept;

	size_type size() const noexcept;

	size_type capacity() const noexcept;

	void shrink_to_fit();

	void clear() noexcept;

	void swap(BucketStorage& other) noexcept;

	iterator begin() noexcept;

	const_iterator begin() const noexcept;

	const_iterator cbegin() const noexcept;

	iterator end() noexcept;

	const_iterator end() const noexcept;

	const_iterator cend() const noexcept;

	iterator get_to_distance(iterator it, const difference_type distance);

	~BucketStorage() noexcept;
};

template< typename T >
BucketStorage< T >::const_iterator::const_iterator(typename Block< T >::Node* pointerNode) noexcept :
	ptrNode(pointerNode)
{
}

template< typename T >
typename BucketStorage< T >::const_iterator& BucketStorage< T >::const_iterator::operator++()
{
	ptrNode = ptrNode->nextNode;
	return *this;
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::const_iterator::operator++(int)
{
	const_iterator copy(*this);
	++(*this);
	return copy;
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator==(const typename BucketStorage::const_iterator& other) const noexcept
{
	return ptrNode == other.ptrNode;
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator<(const typename BucketStorage::const_iterator& other) const noexcept
{
	size_type id = ptrNode->parent->id;
	size_type otherId = other.ptrNode->parent->id;
	return id < otherId || (id == otherId && ptrNode < other.ptrNode);
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator<=(const typename BucketStorage::const_iterator& other) const noexcept
{
	return (*this < other) || *this == other;
}

template< typename T >
typename BucketStorage< T >::const_iterator&
	BucketStorage< T >::const_iterator::operator=(const typename BucketStorage< T >::const_iterator& other) noexcept
{
	ptrNode = other.ptrNode;
	return *this;
}

template< typename T >
typename BucketStorage< T >::const_iterator& BucketStorage< T >::const_iterator::operator--()
{
	ptrNode = ptrNode->prevNode;
	return *this;
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::const_iterator::operator--(int)
{
	const_iterator copy(*this);
	--(*this);
	return copy;
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator>(const typename BucketStorage::const_iterator& other) const noexcept
{
	return !(*this <= other);
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator>=(const typename BucketStorage::const_iterator& other) const noexcept
{
	return !(*this < other);
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator!=(const typename BucketStorage::const_iterator& other) const noexcept
{
	return !(*this == other);
}

template< typename T >
BucketStorage< T >::iterator::iterator(typename Block< T >::Node* val) noexcept : const_iterator(val)
{
}

template< typename T >
typename BucketStorage< T >::iterator::reference BucketStorage< T >::iterator::operator*() noexcept
{
	return *(this->ptrNode->object);
}

template< typename T >
typename BucketStorage< T >::iterator::pointer BucketStorage< T >::iterator::operator->() noexcept
{
	return this->ptrNode->object;
}

template< typename T >
typename BucketStorage< T >::iterator& BucketStorage< T >::iterator::operator--()
{
	const_iterator::operator--();
	return *this;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::iterator::operator--(int)
{
	iterator copy(*this);
	--(*this);
	return copy;
}

template< typename T >
BucketStorage< T >::BucketStorage(
	BucketStorage::size_type capacityBlock,
	BucketStorage::size_type cntBlock,
	BucketStorage::size_type cntObject,
	Block< T >* endBlock,
	Block< T >* headBlock,
	Block< T >* tailBlock,
	Block< T >* blockFreePlace) :
	blockFreePlace(blockFreePlace), endBlock(endBlock), headBlock(headBlock), tailBlock(tailBlock), cntBlock(cntBlock),
	cntObject(cntObject), capacityBlock(capacityBlock)
{
}

template< typename T >
BucketStorage< T >::BucketStorage(BucketStorage::size_type block_capacity) :
	BucketStorage< T >::BucketStorage(block_capacity, 0, 0, new Block< T >(nullptr, nullptr, nullptr, nullptr, 1, UINT64_MAX))
{
	headBlock = tailBlock = endBlock;
	endBlock->first->setValues(nullptr, nullptr, nullptr, nullptr, 0, nil, nil, endBlock);
}

template< typename T >
BucketStorage< T >::BucketStorage(const BucketStorage< T >& other) :
	BucketStorage< T >::BucketStorage(other.capacityBlock, other.cntBlock, other.cntObject, new Block< T >(*other.headBlock))
{
	try
	{
		Block< T >* otherCurBlock = other.headBlock;
		headBlock = tailBlock = endBlock;
		while (true)
		{
			if (otherCurBlock->prevBlockFreePlace || otherCurBlock->nextBlockFreePlace || otherCurBlock == other.blockFreePlace)
			{
				endBlock->prevBlockFreePlace = blockFreePlace;
				if (blockFreePlace)
					blockFreePlace->nextBlockFreePlace = endBlock;
				blockFreePlace = endBlock;
			}
			otherCurBlock = otherCurBlock->nextBlock;
			if (otherCurBlock)
			{
				tailBlock = endBlock;
				endBlock = new Block< T >(*otherCurBlock);
				connectBlock(tailBlock, endBlock);
			}
			else
				break;
		}
	} catch (...)
	{
		if (headBlock)
			switchClear(true);
		throw;
	}
}

template< typename T >
BucketStorage< T >& BucketStorage< T >::operator=(const BucketStorage< T >& other)
{
	if (this == &other)
		return *this;
	BucketStorage< T > copy(other);
	swap(copy);
	return *this;
}

template< typename T >
BucketStorage< T >::BucketStorage(BucketStorage< T >&& other) noexcept : BucketStorage< T >::BucketStorage(0, 0)
{
	swap(other);
}

template< typename T >
BucketStorage< T >& BucketStorage< T >::operator=(BucketStorage< T >&& other) noexcept
{
	if (this == &other)
		return *this;
	BucketStorage< T > temp(std::move(other));
	swap(temp);
	return *this;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insertValue(value_type* value)
{
	if (!blockFreePlace)
	{
		blockFreePlace = new Block< T >(cntBlock ? tailBlock : nullptr, nullptr, endBlock, nullptr, capacityBlock, cntBlock);
		if (!(cntBlock++))
			headBlock = blockFreePlace;
		tailBlock = blockFreePlace;
	}
	++cntObject;
	iterator it = blockFreePlace->insert(value);
	if (blockFreePlace->size == capacityBlock)
		blockFreePlace = blockFreePlace->prevBlockFreePlace;
	return it;
}

template< typename T >
void BucketStorage< T >::deleteListBlock(Block< T >** start, Block< T >* end) noexcept
{
	Block< T >* startValue = *start;
	Block< T >* curBlock = startValue;
	while (curBlock != end)
	{
		startValue = startValue->nextBlock;
		delete curBlock;
		curBlock = startValue;
	}
	*start = curBlock;
}

template< typename T >
void BucketStorage< T >::switchClear(bool all) noexcept
{
	deleteListBlock(&headBlock, endBlock);
	cntBlock = 0;
	cntObject = 0;
	tailBlock = endBlock;
	blockFreePlace = nullptr;
	if (all)
		delete endBlock;
}

template< typename T >
void BucketStorage< T >::connectBlock(Block< T >* blockLeft, Block< T >* blockRight) noexcept
{
	if (!blockLeft || !blockRight)
		return;
	blockLeft->nextBlock = blockRight;
	blockRight->prevBlock = blockLeft;
	typename Block< T >::Node* lastLeft = blockLeft->last;
	typename Block< T >::Node* firstRight = blockRight->first;
	lastLeft->nextNode = firstRight;
	firstRight->prevNode = lastLeft;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(const value_type& value)
{
	value_type* addValue = new value_type(value);
	return insertValue(addValue);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(value_type&& value)
{
	value_type* addValue = new value_type(std::move(value));
	return insertValue(addValue);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::erase(BucketStorage::const_iterator it)
{
	if (it == end())
		throw std::invalid_argument("Cannot delete through the end iterator");

	--cntObject;
	typename Block< T >::Node* node = it.ptrNode;
	Block< T >* block = node->parent;
	iterator ans(node->nextNode);
	if (block->erase(node->idArray))
	{
		block->prevBlockFreePlace = blockFreePlace;
		if (blockFreePlace)
			blockFreePlace->nextBlockFreePlace = block;
		blockFreePlace = block;
	}
	else if (!block->size)
	{
		if (block == headBlock)
			headBlock = block->nextBlock;
		else if (block == tailBlock)
			tailBlock = block->prevBlock;
		if (!(--cntBlock))
			headBlock = tailBlock = endBlock;
		blockFreePlace = blockFreePlace->prevBlockFreePlace;
		delete block;
	}
	return ans;
}

template< typename T >
bool BucketStorage< T >::empty() const noexcept
{
	return cntObject == 0;
}

template< typename T >
typename BucketStorage< T >::size_type BucketStorage< T >::size() const noexcept
{
	return cntObject;
}

template< typename T >
typename BucketStorage< T >::size_type BucketStorage< T >::capacity() const noexcept
{
	return cntBlock * capacityBlock;
}

template< typename T >
void BucketStorage< T >::clear() noexcept
{
	switchClear();
}

template< typename T >
void BucketStorage< T >::shrink_to_fit()
{
	if (size() == capacity())
		return;

	if (!cntObject)
	{
		switchClear();
		return;
	}

	Block< T >*newTail = nullptr, *newHead = nullptr;
	try
	{
		typename BucketStorage< T >::iterator it = begin();
		uint64_t added = 0, newId = 0;
		newTail = new Block< T >(nullptr, nullptr, endBlock, nullptr, capacityBlock, newId++);
		newHead = newTail;
		do
		{
			if (newTail->size == capacityBlock)
			{
				Block< T >* nextNewTail = new Block< T >(newTail, nullptr, endBlock, nullptr, capacityBlock, newId++);
				newTail = nextNewTail;
			}
			typename Block< T >::Node* nd = it.ptrNode;
			newTail->insert(nd->object);
			nd->object = nullptr;
			++it;
		} while (++added != cntObject);
		switchClear();
		headBlock = newHead;
		tailBlock = newTail;
		cntBlock = newId;
		cntObject = added;
		connectBlock(tailBlock, endBlock);
		blockFreePlace = tailBlock->size == capacityBlock ? nullptr : tailBlock;
	} catch (std::bad_alloc& error)
	{
		if (newHead)
			deleteListBlock(&newHead, nullptr);
		throw error;
	}
}

template< typename T >
void BucketStorage< T >::swap(BucketStorage& other) noexcept
{
	std::swap(blockFreePlace, other.blockFreePlace);
	std::swap(endBlock, other.endBlock);
	std::swap(headBlock, other.headBlock);
	std::swap(tailBlock, other.tailBlock);
	std::swap(cntBlock, other.cntBlock);
	std::swap(cntObject, other.cntObject);
	std::swap(capacityBlock, other.capacityBlock);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::begin() noexcept
{
	return iterator(headBlock->first);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::begin() const noexcept
{
	return const_iterator(headBlock->first);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cbegin() const noexcept
{
	return const_iterator(headBlock->first);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::end() noexcept
{
	return iterator(endBlock->first);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::end() const noexcept
{
	return const_iterator(endBlock->first);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cend() const noexcept
{
	return const_iterator(endBlock->first);
}

template< typename T >
typename BucketStorage< T >::iterator
	BucketStorage< T >::get_to_distance(BucketStorage< T >::iterator it, const BucketStorage::difference_type distance)
{
	size_type absDist = distance >= 0 ? distance : -distance;
	for (size_type i = 0; i < absDist; ++i)
		distance > 0 ? ++it : --it;
	return it;
}

template< typename T >
BucketStorage< T >::~BucketStorage() noexcept
{
	switchClear(true);
}
