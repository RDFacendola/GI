/// \file handle.h
/// \brief Classes and functions to manage weak references.
///
/// \author Raffaele D. Facendola

#include <memory>

using ::std::shared_ptr;

namespace gi_lib{

	/// \brief Represent a weak reference to a shared resource.
	/// \author Raffaele D. Facendola.
	template <typename TResource>
	class Handle{

	public:

		/// \brief Create an empty handle.
		Handle() :
			resource_(nullptr){}

		/// \brief Create an handle to a shared resource.
		/// \tparam TShared type of the resource to handle.
		/// \param resource Shared pointer to the resource.
		template <typename TShared>
		Handle(TShared&& resource) :
			resource_(std::forward<TShared>(resource)){}

		/// \brief Dereferencing operator.
		/// \return Returns a reference to the shared pointer holding the underlying resource.
		shared_ptr<TResource> & operator->(){

			return resource_;

		}

		/// \brief Dereferencing operator.
		/// \return Returns a reference to the shared pointer holding the underlying resource.
		shared_ptr<const TResource> & operator->() const{

			return resource_;

		}

		/// \brief Dereferencing operator.
		/// \return Returns a reference to the underlying resource.
		TResource & operator*(){

			return *resource_;

		}

		/// \brief Dereferencing operator.
		/// \return Returns a reference to the underlying resource.
		const TResource & operator*() const{

			return *resource_;

		}

		/// \brief Check whether the handle is valid or not.
		/// \return Returns true if the handle can be dereferenced. Returns false otherwise.
		operator bool() const{

			return resource_ != nullptr;

		}

	private:

		shared_ptr<TResource> resource_;

	};

}