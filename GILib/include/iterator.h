/// \file iterator.h
/// \brief Classes and methods for iterator and range management.
///
/// \author Raffaele D. Facendola

#include <utility>

namespace gi_lib{

	/// \brief Range class.
	/// \author Alisdair Meredith
	/// \remarks Based on http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2009/n2977.pdf
	template<typename Iter>
	struct range : pair<Iter, Iter> {
		
		using pair::pair;

	};

	/// \brief Begin of a range.
	/// \return Returns the begin of a range.
	/// \author Alisdair Meredith
	template<typename Iter>
	Iter begin(const range<Iter> & p){

		return p.first;

	}

	/// \brief End of a range.
	/// \return Returns the end of a range.
	/// \author Alisdair Meredith
	template<typename Iter>
	Iter end(const range<Iter> & p){
		
		return p.second;

	}
	 
	/// \brief Create a range.
	/// \return Returns the range
	/// \author Alisdair Meredith
	template<typename T>
	auto make_range(T & t)->range<decltype(begin(t))>{

		return range<decltype(begin(t))>{ begin(t), end(t) };

	}
		  
}
