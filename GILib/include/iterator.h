/// \file iterator.h
/// \brief Classes and methods for iterator and range management.
///
/// \author Raffaele D. Facendola

#include <iterator>

namespace gi_lib{

	/// \brief Begin of a pair-range.
	/// \tparam TIterator Type of iterator used by the range.
	/// \return Returns the begin of a range.
	template<typename TIterator>
	TIterator begin(const std::pair<TIterator, TIterator> & p){

		return p.first;

	}

	/// \brief End of a pair-range.
	/// \tparam TIterator Type of iterator used by the range.
	/// \return Returns the end of a range.
	template<typename TIterator>
	TIterator end(const std::pair<TIterator, TIterator> & p){
		
		return p.second;

	}
	 

}
