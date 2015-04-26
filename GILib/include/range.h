/// \file range.h
/// \brief Classes for range management.
///
/// \author Raffaele D. Facendola

#pragma once

#include <cstddef>
#include <utility>
#include <iterator>
#include <type_traits>

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

		/// \brief Reference to the element pointed by head.
		reference operator*();

		/// \brief Pointer to the element pointed by head.
		pointer operator->();

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
	/// \tparam TPointerMap Type of the functor that maps the iterator's value. It requires operator()(TIterator::reference) -> TWrapped*
	template <typename TIterator, typename TWrapped, typename TPointerMap>
	struct IteratorWrapper : std::iterator < std::forward_iterator_tag, TWrapped > {

		/// \brief Create a new iterator.
		/// \param iterator Iterator to wrap.
		/// \param pointer_map Functor used to map the iterator's value.
		IteratorWrapper(const TIterator& iterator, TPointerMap pointer_map);

		/// \brief Create a new iterator.
		/// \param iterator Iterator to wrap.
		IteratorWrapper(const TIterator& iterator);

		/// \brief Copy constructor.
		/// \param iterator Iterator to copy.
		IteratorWrapper(const IteratorWrapper& iterator);

		/// \brief Test for equality.
		bool operator==(const IteratorWrapper& other);

		/// \brief Test for inequality.
		bool operator!=(const IteratorWrapper& other);

		/// \brief Dereferencing operator.
		reference operator*();

		/// \brief Arrow operator.
		pointer operator->();

		/// \brief Prefix increment.
		IteratorWrapper& operator++();

		/// \brief Postfix increment.
		IteratorWrapper operator++(int);

	private:

		TIterator iterator_;				/// \brief Wrapped iterator.

		TPointerMap pointer_map_;			/// \brief Maps the pointer type.

	};

	/// \brief Wraps a collection and an iterator. The iterator is used to visit the collection in a non-linear way.
	/// \tparam TDirect Type of the collection. Must support the bracket operator: TDirect::operator[](...) -> TType&
	/// \tparam TIndex Type of the index iterator. Must be compliant with an input iterator.
	/// \tparam TType Type of elements referenced by the collection. See TDirect.
	template <typename TDirect, typename TIndex, typename TType>
	struct IndexedIterator : std::iterator < std::forward_iterator_tag, TType > {

		/// \brief Create a new iterator.
		/// \param direct Direct collection.
		/// \param index Index iterator used as offset.
		IndexedIterator(TDirect direct, TIndex index);

		/// \brief Copy constructor.
		/// \param iterator Iterator to copy.
		IndexedIterator(const IndexedIterator& iterator);

		/// \brief Test for equality.
		bool operator==(const IndexedIterator& other);

		/// \brief Test for inequality.
		bool operator!=(const IndexedIterator& other);

		/// \brief Dereferencing operator.
		reference operator*();

		/// \brief Arrow operator.
		pointer operator->();

		/// \brief Prefix increment.
		IndexedIterator& operator++();

		/// \brief Postfix increment.
		IndexedIterator operator++(int);

	private:

		TDirect direct_;				/// \brief Direct collection.

		TIndex index_;					/// \brief Indirect iterator.

	};

	/// \brief Utility function to make indexed iterators.
	/// \param direct Direct collection.
	/// \param index Index iterator.
	template <typename TDirect, typename TIndex>
	auto make_indexed(TDirect&& direct, TIndex&& index) -> IndexedIterator < typename std::remove_reference<TDirect&&>::type, 
																		     typename std::remove_reference<TIndex&&>::type, 
																		     typename std::remove_reference<decltype(*direct)>::type > {

		return IndexedIterator < std::remove_reference<TDirect&&>::type, 
								 std::remove_reference<TIndex&&>::type, 
								 std::remove_reference<decltype(*direct)>::type >( std::forward<TDirect&&>(direct),
																				   std::forward<TIndex&&>(index) );

	}

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

		return begin_.operator*();

	}

	template <typename TIterator>
	inline typename Range<TIterator>::pointer Range<TIterator>::operator->(){

		return begin_.operator->();

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

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	IteratorWrapper< TIterator, TWrapped, TPointerMap >::IteratorWrapper(const TIterator& iterator, TPointerMap pointer_map) :
		iterator_(iterator),
		pointer_map_(pointer_map){}

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	IteratorWrapper< TIterator, TWrapped, TPointerMap >::IteratorWrapper(const TIterator& iterator) :
		iterator_(iterator){}

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	IteratorWrapper< TIterator, TWrapped, TPointerMap >::IteratorWrapper(const IteratorWrapper& iterator) :
		iterator_(iterator.iterator_),
		pointer_map_(iterator.pointer_map_){}

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	bool IteratorWrapper< TIterator, TWrapped, TPointerMap >::operator==(const IteratorWrapper& other){

		return iterator_ == other.iterator_;

	}

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	bool IteratorWrapper< TIterator, TWrapped, TPointerMap >::operator!=(const IteratorWrapper& other){

		return iterator_ != other.iterator_;

	}

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	typename IteratorWrapper< TIterator, TWrapped, TPointerMap >::reference IteratorWrapper< TIterator, TWrapped, TPointerMap >::operator*(){

		return *pointer_map_(*iterator_);

	}

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	typename IteratorWrapper< TIterator, TWrapped, TPointerMap >::pointer IteratorWrapper< TIterator, TWrapped, TPointerMap >::operator->(){

		return pointer_map_(*iterator_);

	}

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	IteratorWrapper< TIterator, TWrapped, TPointerMap >& IteratorWrapper< TIterator, TWrapped, TPointerMap >::operator++(){

		++iterator_;

		return *this;

	}

	template <typename TIterator, typename TWrapped, typename TPointerMap>
	IteratorWrapper< TIterator, TWrapped, TPointerMap > IteratorWrapper< TIterator, TWrapped, TPointerMap >::operator++(int){

		IteratorWrapper< TIterator, TWrapped, TPointerMap > old(*this);

		++iterator_;

		return old;

	}

	///////////////////////////////// INDEXED ITERATOR /////////////////////////////////////////

	template <typename TDirect, typename TIndex, typename TType>
	IndexedIterator<TDirect, TIndex, TType>::IndexedIterator(TDirect direct, TIndex index) :
		direct_(direct),
		index_(index){}

	template <typename TDirect, typename TIndex, typename TType>
	IndexedIterator<TDirect, TIndex, TType>::IndexedIterator(const IndexedIterator& iterator) :
		direct_(iterator.direct_),
		index_(iterator.index_){}

	template <typename TDirect, typename TIndex, typename TType>
	bool IndexedIterator<TDirect, TIndex, TType>::operator==(const IndexedIterator& other){

		return direct_[*index_] == other.direct_[*other.index_];

	}

	template <typename TDirect, typename TIndex, typename TType>
	bool IndexedIterator<TDirect, TIndex, TType>::operator!=(const IndexedIterator& other){

		return direct_[*index_] != other.direct_[*other.index_];

	}

	template <typename TDirect, typename TIndex, typename TType>
	typename IndexedIterator<TDirect, TIndex, TType>::reference IndexedIterator<TDirect, TIndex, TType>::operator*(){

		return direct_[*index_];

	}

	template <typename TDirect, typename TIndex, typename TType>
	typename IndexedIterator<TDirect, TIndex, TType>::pointer IndexedIterator<TDirect, TIndex, TType>::operator->(){

		return &(direct_[*index_]);

	}

	template <typename TDirect, typename TIndex, typename TType>
	IndexedIterator<TDirect, TIndex, TType>& IndexedIterator<TDirect, TIndex, TType>::operator++(){

		++index_;

		return *this;

	}

	template <typename TDirect, typename TIndex, typename TType>
	IndexedIterator<TDirect, TIndex, TType> IndexedIterator<TDirect, TIndex, TType>::operator++(int){

		IndexedIterator<TDirect, TIndex, TType> old(*this);

		++index_;

		return old;

	}
	
}