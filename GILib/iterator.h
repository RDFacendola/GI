/// \file iterator.h
/// \brief Defines utility classes to use with iterators.
///
/// \author Raffaele D. Facendola

#pragma once

namespace gi_lib{

	/// \brief Packs two iterators together to iterate through a subset of a container.
	/// \tparam TValue Type of the objects drescribed by the range.
	/// \tparam TIterator Type of the iterator used to point to the objects.
	template <typename TValue, typename TIterator>
	class Range{

	public:

		/// \brief Iterator type.
		typedef TIterator iterator;
		typedef ptrdiff_t difference_type;
		typedef size_t size_type;
		typedef TValue value_type;
		typedef TValue * pointer;
		typedef TValue & reference;

		/// \brief Constructor.
		/// \param begin_it Iterator which points to the first element of the range.
		/// \param end_it Iterator which points to the element past the end of the range.
		Range(iterator begin_it, iterator end_it) :
			begin_(begin_it),
			end_(end_it){}

		/// \brief Begin of the range.
		/// \return Returns an iterator pointing to the first element of the range.
		inline iterator begin(){

			return begin_;

		}

		/// \brief One element past to the end of the range.
		/// \return Returns an iterator pointing to the element past the end of the range.
		inline iterator end() {

			return end_;

		}

		/// \brief Tells whether the range is empty or not.
		/// \return Returns true if the range is empty, false otherwise
		inline bool empty(){

			return begin_ == end_;

		}

	private:

		iterator begin_;

		iterator end_;

	};

}