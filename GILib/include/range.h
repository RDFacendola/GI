/// \file range.h
/// \brief Classes for range management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <cstddef>
#include <utility>
#include <iterator>

using std::pair;
using std::iterator;

namespace gi_lib{

	/// \brief Wraps a pair of iterators defining a range that can be iterated through.
	/// \tparam TIterator Type of the iterators to wrap. Any forward iterator compilant with the standard template library should work.
	/// \author Raffaele D. Facendola
	template <typename TIterator>
	class Range{

	public:

		/// \brief Standard STL defines.
		using iterator = TIterator;
		using difference_type = typename TIterator::difference_type;
		using size_type = size_t;
		using value_type = typename TIterator::value_type;
		using pointer = typename TIterator::pointer;
		using reference = typename TIterator::reference;

		/// \brief Create a new component range.
		/// \param range Range.
		Range(pair<iterator, iterator> range);

		/// \brief Create a new component range.
		/// \param begin Begin of the range.
		/// \param end End of the range.
		Range(iterator begin, iterator end);

		/// \brief Begin of the range.
		/// \return Returns an iterator pointing to the begin of the range.
		iterator begin();

		/// \brief End of the range.
		/// \return Returns an iterator pointing past the end of the range.
		iterator end();

		/// \brief Dereference the head of the range.
		/// Undefined behaviour if the range is empty.
		reference operator*();

		/// \brief Moves the head of the range forward by one element. Prefix.
		/// \return Returns the reference to this range.
		Range& operator++();

		/// \brief Moves the head of the range forward by one element. Postfix.
		/// \return Returns the old range.
		Range operator++(int);

		/// \brief Is this range empty?
		/// \return Returns true if the range is empty, false otherwise.
		bool IsEmpty();

	protected:

		iterator begin_;	///< \brief Points to the first element of the range.

		iterator end_;		///< \brief Points to one element past the end of the range.

	};

	/// \brief Wraps an input iterator that dereferences to another type.
	template <typename TIterator, typename TWrapped, typename TMapper>
	struct IteratorWrapper : std::iterator<std::input_iterator_tag, TWrapped>{

		/// \brief Create a new iterator.
		/// \param iterator Iterator to wrap.
		IteratorWrapper(const TIterator& iterator, TMapper mapper);

		/// \brief Copy constructor.
		/// \param iterator Iterator to copy.
		IteratorWrapper(const IteratorWrapper<TIterator, TWrapped, TMapper>& iterator);

		/// \brief Test for equality.
		bool operator==(const IteratorWrapper<TIterator, TWrapped, TMapper>& other);

		/// \brief Test for inequality.
		bool operator!=(const IteratorWrapper<TIterator, TWrapped, TMapper>& other);

		/// \brief Dereferencing operator.
		TWrapped& operator*();

		/// \brief Prefix increment.
		IteratorWrapper<TIterator, TWrapped, TMapper>& operator++();

		/// \brief Postfix increment.
		IteratorWrapper<TIterator, TWrapped, TMapper> operator++(int);

	private:

		TIterator iterator_;	/// \brief Wrapped iterator.

		TMapper mapper_;		/// \brief Used to map a value from the original wrapped iterator to the new type.

	};

	///////////////////////////////// RANGE /////////////////////////////////////////

	template <typename TIterator>
	Range<TIterator>::Range(pair<iterator, iterator> range) :
		begin_(range.first),
		end_(range.second){}

	template <typename TIterator>
	Range<TIterator>::Range(iterator begin, iterator end) :
		begin_(begin),
		end_(end){}
	
	template <typename TIterator>
	inline typename Range<TIterator>::iterator Range<TIterator>::begin(){

		return begin_;

	}

	template <typename TIterator>
	inline typename Range<TIterator>::iterator Range<TIterator>::end(){

		return end_;

	}

	template <typename TIterator>
	inline typename Range<TIterator>::reference Range<TIterator>::operator*(){

		return *begin_;

	}

	template <typename TIterator>
	inline Range<TIterator>& Range<TIterator>::operator++(){

		++begin_;

		return *this;

	}

	template <typename TIterator>
	inline Range<TIterator> Range<TIterator>::operator++(int){

		Range<TIterator> old(*this);	/// Copy ctor

		++begin_;

		return old;

	}

	template <typename TIterator>
	inline bool Range<TIterator>::IsEmpty(){

		return begin_ == end_;

	}
	
	///////////////////////////////// ITERATOR WRAPPER /////////////////////////////////////////

	template <typename TIterator, typename TWrapped, typename TMapper>
	IteratorWrapper<TIterator, TWrapped, TMapper>::IteratorWrapper(const TIterator& iterator, TMapper mapper) :
		iterator_(iterator),
		mapper_(mapper){}

	template <typename TIterator, typename TWrapped, typename TMapper>
	IteratorWrapper<TIterator, TWrapped, TMapper>::IteratorWrapper(const IteratorWrapper<TIterator, TWrapped, TMapper>& iterator) :
		iterator_(iterator.iterator_),
		mapper_(iterator.mapper_){}

	template <typename TIterator, typename TWrapped, typename TMapper>
	bool IteratorWrapper<TIterator, TWrapped, TMapper>::operator==(const IteratorWrapper<TIterator, TWrapped, TMapper>& other){

		return iterator_ == other.iterator_;

	}

	template <typename TIterator, typename TWrapped, typename TMapper>
	bool IteratorWrapper<TIterator, TWrapped, TMapper>::operator!=(const IteratorWrapper<TIterator, TWrapped, TMapper>& other){

		return iterator_ != other.iterator_;

	}

	template <typename TIterator, typename TWrapped, typename TMapper>
	typename TWrapped& IteratorWrapper<TIterator, TWrapped, TMapper>::operator*(){

		return mapper_(iterator_);

	}

	template <typename TIterator, typename TWrapped, typename TMapper>
	IteratorWrapper<TIterator, TWrapped, TMapper>& IteratorWrapper<TIterator, TWrapped, TMapper>::operator++(){

		++iterator_;

		return *this;

	}

	template <typename TIterator, typename TWrapped, typename TMapper>
	IteratorWrapper<TIterator, TWrapped, TMapper> IteratorWrapper<TIterator, TWrapped, TMapper>::operator++(int){

		IteratorWrapper<TIterator, TWrapped, TMapper> old(iterator_, mapper_);

		++iterator_;

		return old;

	}

}