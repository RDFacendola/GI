/// \file maybe.h
/// \brief Classes and methods to manage the null object pattern.
///
/// \author Raffaele D. Facendola


namespace gi_lib{

	/// \brief Maybe<TValue> is either a TValue or nothing.
	/// \author Raffaele D. Facendola
	template <typename TValue>
	class Maybe{

	public:

		/// \brief Create a maybe object.
		/// \param value The value to store inside the maybe object.
		Maybe(TValue&& value) :
			value_(std::forward<TValue>(value)),
			is_null_(false){}
		
		/// \brief Copy constructor.
		/// \param other The object to copy.
		Maybe(const Maybe & other) :
			is_null_(other.is_null_){

			if (!is_null_){

				value_ = other.value_;

			}

		}

		/// \brief Move constructor.
		/// \param other The object to move.
		Maybe(Maybe && other) :
			is_null_(other.is_null_){

			if (!is_null_){

				value_ = std::move(other.value_);

			}

		}

		/// \brief Create an empty object.
		Maybe() :
			is_null_(true){}

		/// \brief Check whether the object stores a value or not.
		/// \return Returns true if the object can be dereferenced. Returns false otherwise.
		operator bool() const{

			return !is_null_;

		}

		/// \brief Dereferencing operator.
		/// \return Returns a reference to the stored value.
		TValue& operator*(){

			return value_;

		}

		/// \brief Dereferencing operator.
		/// \return Returns a pointer to the stored value.
		auto operator->() -> decltype(&(operator*())){

			return &value_;

		}

	private:

		bool is_null_;

		union
		{

			TValue value_;

		};
		
	};

}
