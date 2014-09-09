/// \file functional.h
/// \brief Algorithm for functional programming. 
///
/// \author Raffaele D. Facendola

#include <tuple>
#include <vector>
#include <iterator>

using std::tuple;
using std::vector;

namespace gi_lib{

	template <typename TArg, typename... TRest>
	class ZipIterator;

	template <typename TArg>
	class ZipIterator < TArg > {

	public:

		using difference_type = typename TArg::difference_type;
		using value_type = tuple< typename TArg::value_type >;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::input_iterator_tag;

		/// \brief Create a new zip iterator.
		/// \param iterators iterators to zip together.
		ZipIterator(TArg iterator);

		/// \brief Dereferencing operator.
		value_type operator*(){
			
			return value_type(*iterator_);

		}

		/// \brief Iterator advance.
		/// \return Returns a reference to this instance.
		ZipIterator & operator++();

		/// \brief Inequality operator.
		/// \param other The other iterator to test against.
		/// \return Returns true if the two iterators are different, false otherwise.
		bool operator!=(const ZipIterator<TArg> & other);

	private:

		TArg iterator_;

	};

	template <typename TArg, typename... TRest>
	class ZipIterator < TArg, TRest... > : public ZipIterator<TRest...>{

	public:

		using difference_type = typename TArg::difference_type;
		using value_type = tuple < typename TArg::value_type, typename TRest::value_type... > ;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::input_iterator_tag;

		/// \brief Create a new zip iterator.
		/// \param iterators iterators to zip together.
		ZipIterator(TArg iterator, TRest... rest) :
			base(rest...),
			iterator_(iterator){}

		/// \brief Dereferencing operator.
		value_type operator*(){

			return tuple_cat(*iterator_, base::operator*());

		}

		/// \brief Iterator advance.
		/// \return Returns a reference to this instance.
		ZipIterator & operator++();

		/// \brief Inequality operator.
		/// \param other The other iterator to test against.
		/// \return Returns true if the two iterators are different, false otherwise.
		bool operator!=(const ZipIterator<TArg> & other);

	private:

		TArg iterator_;

	};

	template <typename... TArgs>
	ZipIterator<TArgs...> make_zip(TArgs&&... iterators){

		return ZipIterator<TArgs...>(std::forward<TArgs>(iterators)...);

	}

	//

	template <typename TArg>
	ZipIterator<TArg>::ZipIterator(TArg iterator) :
		iterator_(iterator){}

	template <typename TArg>
	ZipIterator<TArg> & ZipIterator<TArg>::operator++(){

		++iterator_;

		return *this;

	}

	template <typename TArg>
	bool ZipIterator<TArg>::operator!=(const ZipIterator<TArg> & other){

		return iterator_ != other.iterator_;

	}

	//

	template <typename TArg, typename... TRest>
	ZipIterator<TArg, TRest...> & ZipIterator<TArg, TRest...>::operator++(){

		base::operator++();

		++iterator_;

		return *this;

	}

	template <typename TArg, typename... TRest>
	bool ZipIterator<TArg, TRest...>::operator!=(const ZipIterator<TArg> & other){

		return iterator_ != other.iterator_ ||
			base::operator!=(other);

	}

}