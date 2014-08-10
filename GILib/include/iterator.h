/// \file iterator.h
/// \brief Defines utility classes to use with iterators.
///
/// \author Raffaele D. Facendola

#pragma once

namespace gi_lib{

	/// \brief Packs two iterators together to iterate through a subset of a container.
	/// \tparam TContainer Type of the container the iterators have been generated from. It must be STL-compliant container.
	template <typename TContainer>
	class Range{

	public:

		typedef typename TContainer::allocator_type allocator_type;
		typedef typename TContainer::size_type size_type;
		typedef typename TContainer::difference_type difference_type;
		typedef typename TContainer::pointer pointer;
		typedef typename TContainer::const_pointer const_pointer;
		typedef typename TContainer::reference reference;
		typedef typename TContainer::const_reference const_reference;
		typedef typename TContainer::iterator iterator;
		typedef typename TContainer::const_iterator const_iterator;
		typedef typename TContainer::value_type value_type;

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

		/// \brief Begin of the range.
		/// \return Returns an iterator pointing to the first element of the range.
		inline const_iterator cbegin() const{

			return begin_;

		}

		/// \brief One element past to the end of the range.
		/// \return Returns an iterator pointing to the element past the end of the range.
		inline const_iterator cend() const {

			return end_;

		}

	private:

		iterator begin_;

		iterator end_;

	};

}