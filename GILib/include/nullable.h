/// \file nullable.h
/// \brief Classes and methods to manage the null object pattern.
///
/// \author Raffaele D. Facendola


namespace gi_lib{

	/// \brief Nullable object to abstract the null object pattern.
	/// \tparam TType Type of the object the nullable holds.
	/// \author Raffaele D. Facendola
	template <typename TType>
	class Nullable{

	public:

		/// \brief Create a nullable from a reference.
		/// \param object Reference to the object to store.
		Nullable(TType & object) :
			object_(&object){}

		/// \brief Create a null nullable.
		Nullable():
			object_(nullptr){}

		/// \brief Get a reference to the object.
		/// return Returns a reference to the object.
		TType & Value() const{

			return *object_;

		}

		/// \brief Get a reference to the object.
		/// return Returns a reference to the object.
		TType & operator*() const{

			return *object_;

		}

		/// \brief Get a pointer to the object.
		/// return Returns a pointer to the object.
		TType * operator->() const{

			return object_;

		}

		/// \brief Check whether the nullable has an object or not.
		/// \return Returns true if this class holds an object, false otherwise.
		operator bool() const{

			return object_ != nullptr;

		}

	private:

		TType * object_;

	};

	/// \brief Create a new nullable.
	/// \tparam TType Type of the object the nullable can hold.
	/// \return Returns a nullable holding the specified object.
	template <typename TType>
	Nullable<TType> make_nullable(TType& object){

		return Nullable<TType>(object);

	}

}
