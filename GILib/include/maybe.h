/// \file maybe.h
/// \brief Classes and methods to manage the null object pattern.
///
/// \author Raffaele D. Facendola

#pragma once

namespace gi_lib{

	/// \brief Maybe is either TType or nothing.
	template<typename TType> class Maybe{};

	/// \brief L-value reference maybe specialization.
	/// \author Raffaele D. Facendola.
	template<typename TType>
	class Maybe < TType& > {

	public:

		/// \brief Create a maybe object.
		/// \param value The value to store inside the maybe object.
		Maybe(TType & value) :
			value_ptr_(&value),
			is_null_(false){}

		/// \brief Create an empty object.
		Maybe() :
			is_null_(true),
			value_ptr_(nullptr){}

		/// \brief Copy constructor.
		/// \param other The object to copy.
		Maybe(const Maybe & other) :
			is_null_(other.is_null_),
			value_ptr_(other.value_ptr_){}
		
		/// \brief Assignment operator.
		Maybe & operator=(const Maybe & other){

			is_null_ = other.is_null_;
			value_ptr_ = other.value_ptr_;

		}

		/// \brief Check whether the object stores a value or not.
		/// \return Returns true if the object can be dereferenced. Returns false otherwise.
		operator bool() const{

			return !is_null_;

		}

		/// \brief Dereferencing operator.
		/// \return Returns a reference to the stored value.
		TType & operator*(){

			return *value_ptr_;

		}
		
		/// \brief Dereferencing operator.
		/// \return Returns a pointer to the value.
		TType * operator->(){

			return value_ptr_;

		}

	private:

		bool is_null_;

		TType * value_ptr_;

	};



}
