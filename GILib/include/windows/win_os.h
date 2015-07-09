/// \file win_os.h
/// \brief Windows-specific interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <string>

#include "exceptions.h"
#include "macros.h"

/// \brief If the provided expression fails the caller returns the expression value, otherwise nothing happens.
/// The expression fails if FAILED(.) is true.
#define RETURN_ON_FAIL_1(expr) \
do{ \
	HRESULT __hr = expr; \
	if (FAILED(__hr)) return __hr; \
}WHILE0

/// \brief If the provided expression fails the caller returns the expression value, otherwise nothing happens.
/// The expression fails if FAILED(.) is true.
#define RETURN_ON_FAIL_2(expr, retrn) \
do{ \
	HRESULT __hr = expr; \
	if (FAILED(__hr)) return retrn; \
}WHILE0

/// \brief Macro selector. If the provided expression fails the caller returns either the failure value or a specified value.
#define RETURN_ON_FAIL(...) EXPAND( SELECT_3RD(__VA_ARGS__ , RETURN_ON_FAIL_2, RETURN_ON_FAIL_1)(__VA_ARGS__ ) )

/// \brief If the provided expression fails the caller throws an exception with the error code, otherwise nothing happens.
/// The expression fails if FAILED(.) is true.
#define THROW_ON_FAIL_1(expr) \
do{ \
	HRESULT __hr = expr; \
	if(FAILED(__hr)) THROW(std::to_wstring(__hr)); \
}WHILE0

/// \brief If the provided expression fails the caller throws an exception with the error code followed by the specified error message, otherwise nothing happens.
/// The expression fails if FAILED(.) is true.
#define THROW_ON_FAIL_2(expr, thrw_mssg) \
do{ \
	HRESULT __hr = expr; \
	if(FAILED(__hr)) THROW(std::to_wstring(__hr) + L": " + thrw_mssg); \
}WHILE0

/// \brief Macro selector. If the provided expression fails the caller returns either the failure value or a specified value.
#define THROW_ON_FAIL(...) \
EXPAND( SELECT_3RD(__VA_ARGS__ , THROW_ON_FAIL_2, THROW_ON_FAIL_1)(__VA_ARGS__ ) )

/// \brief RAII guard for COM interfaces.
/// The macro will create an anonymous COMPtr holding the interface.
/// The Release method is called because COMPtr will increase the reference count of the interface.
#define COM_GUARD(com) \
COMPtr<IUnknown> ANONYMOUS(com); \
com->Release();

namespace gi_lib{

	namespace windows{
		
		/// \brief Smart pointer to a COM interface.
		/// The pointer will add a reference during initialization and remove one during destruction.
		/// This class is designed to be a light-weight wrapper around an IUnknown pointer.
		/// Do not add any member variables to this class!!!
		/// \author Raffaele D. Facendola
		template <typename TCOM>
		class COMPtr{

		public:

			/// \brief Create an empty pointer.
			COMPtr();

			/// \brief Create an empty pointer.
			COMPtr(nullptr_t);

			/// \brief Defines a pointer to an object.
			/// \param object Object that will be pointed by this pointer.
			COMPtr(TCOM* object);

			/// \brief Defines a pointer to an object.
			/// \param object Object that will be pointed by this pointer.
			template <typename TOther>
			COMPtr(TOther* object);
			
			/// \brief Copy constructor.
			/// \param other Other pointer to copy.
			COMPtr(const COMPtr<TCOM>& other);

			/// \brief Copy constructor.
			/// \param other Other pointer to copy.
			template <typename TOther>
			COMPtr(const COMPtr<TOther>& other);

			/// \brief Move constructor.
			/// \param other Instance to move.
			COMPtr(COMPtr<TCOM>&& other);

			/// \brief Move constructor.
			/// \param other Instance to move.
			template <typename TOther>
			COMPtr(COMPtr<TOther>&& other);

			/// \brief Destructor.
			/// Decreases by one the reference count of the pointed object, if any.
			~COMPtr();

			/// \brief Copy assignment.
			COMPtr<TCOM>& operator=(const COMPtr<TCOM>& other);

			/// \brief Copy assignment.
			template <typename TOther>
			COMPtr<TCOM>& operator=(const COMPtr<TOther>& other);

			/// \brief Move assignment.
			COMPtr<TCOM>& operator=(COMPtr<TCOM>&& other);

			/// \brief Move assignment.
			template <typename TOther>
			COMPtr<TCOM>& operator=(COMPtr<TOther>&& other);

			/// \brief Equality operator.
			/// \return Returns true if both this instance and the specified one points to the same object, returns false otherwise.
			bool operator==(const COMPtr<TCOM>& other) const;

			/// \brief Inequality operator.
			/// \return Returns true if this instance and the specified one points to different objects, returns false otherwise.
			bool operator!=(const COMPtr<TCOM>& other) const;

			/// \brief Transfer the ownership of an existing COM interface to this pointer.
			/// The method won't add any reference to the passed object.
			/// \param object Object to acquire.
			template <typename TOther>
			COMPtr<TCOM>& operator<<(TOther** object);

			/// \brief Release the ownership of the COM interface managed by this pointer.
			/// \param object Object to acquire.
			template <typename TOther>
			COMPtr<TCOM>& operator>>(TOther** object);

			/// \brief Used to validate the pointed object.
			/// \return Returns true if the pointed object is not null, returns false otherwise.
			operator bool() const;

			/// \brief Arrow operator.
			/// Access the managed object.
			TCOM* operator->() const;

			/// \brief Dereferencing operator.
			/// Access the managed object.
			TCOM& operator*() const;

			/// \brief Get a pointer to the managed object.
			TCOM* Get() const;

			/// \brief Return a pointer to the internal
			TCOM** Setter();

			/// \brief Release the pointed object.
			void Release();

		private:

			/// \brief Add a reference to the pointed object.
			void AddRef();

			TCOM* object_ptr_;			/// \brief Pointer to the object.

		};

		/// \brief Move the ownership of a COM interface to a new COM pointer.
		/// \param object Object whose ownership will be moved.
		template <typename TCOM>
		COMPtr<TCOM> COMMove(TCOM** object);

		///////////////////////////// COM PTR //////////////////////////////////

		template <typename TCOM>
		inline COMPtr<TCOM>::COMPtr() :
			object_ptr_(nullptr){}

		template <typename TCOM>
		inline COMPtr<TCOM>::COMPtr(nullptr_t) :
			COMPtr(){}

		template <typename TCOM>
		inline COMPtr<TCOM>::COMPtr(TCOM* object) :
			object_ptr_(object){

			AddRef();

		}

		template <typename TCOM>
		template <typename TOther>
		inline COMPtr<TCOM>::COMPtr(TOther* object) :
			object_ptr_(static_cast<TCOM*>(object)){

			AddRef();

		}

		template <typename TCOM>
		inline COMPtr<TCOM>::COMPtr(const COMPtr<TCOM>& other) :
			COMPtr(other.Get()){}

		template <typename TCOM>
		template <typename TOther>
		inline COMPtr<TCOM>::COMPtr(const COMPtr<TOther>& other) :
			COMPtr(static_cast<TCOM*>(other.Get())){}

		template <typename TCOM>
		inline COMPtr<TCOM>::COMPtr(COMPtr<TCOM>&& other) :
			object_ptr_(other.Get()){

			other.object_ptr_ = nullptr;

		}

		template <typename TCOM>
		template <typename TOther>
		inline COMPtr<TCOM>::COMPtr(COMPtr<TOther>&& other) :
			object_ptr_(static_cast<TCOM*>(other.Get())){

			other.object_ptr_ = nullptr;

		}

		template <typename TCOM>
		inline COMPtr<TCOM>::~COMPtr(){

			Release();

		}

		template <typename TCOM>
		COMPtr<TCOM>& COMPtr<TCOM>::operator=(const COMPtr<TCOM>& other){

			Release();

			object_ptr_ = static_cast<TCOM*>(other.object_ptr_);

			AddRef();

			return *this;

		}

		template <typename TCOM>
		template <typename TOther>
		COMPtr<TCOM>& COMPtr<TCOM>::operator=(const COMPtr<TOther>& other){

			Release();

			object_ptr_ = static_cast<TCOM*>(other.object_ptr_);

			AddRef();

			return *this;

		}

		template <typename TCOM>
		COMPtr<TCOM>& COMPtr<TCOM>::operator=(COMPtr<TCOM>&& other){

			Release();

			object_ptr_ = other.object_ptr_;

			other.object_ptr_ = nullptr;

			return *this;

		}

		template <typename TCOM>
		template <typename TOther>
		COMPtr<TCOM>& COMPtr<TCOM>::operator=(COMPtr<TOther>&& other){

			Release();

			object_ptr_ = static_cast<TCOM*>(other.object_ptr_);

			other.object_ptr_ = nullptr;

			return *this;

		}

		template <typename TCOM>
		inline bool COMPtr<TCOM>::operator==(const COMPtr<TCOM>& other) const{

			return object_ptr_ == other.object_ptr_;

		}

		template <typename TCOM>
		inline bool COMPtr<TCOM>::operator!=(const COMPtr<TCOM>& other) const{

			return object_ptr_ != other.object_ptr_;

		}

		template <typename TCOM>
		template <typename TOther>
		COMPtr<TCOM>& COMPtr<TCOM>::operator<<(TOther** object){

			Release();

			if (object){

				object_ptr_ = static_cast<TCOM*>(*object);	// Acquire without adding any reference.

				*object = nullptr;							// Clear the source.

			}
			else{

				object_ptr_ = nullptr;

			}

			return *this;

		}

		template <typename TCOM>
		template <typename TOther>
		COMPtr<TCOM>& COMPtr<TCOM>::operator>>(TOther** object){

			if (object){

				if (*object){

					(*object)->Release();		// Release the destination object, if present.

				}

				*object = object_ptr_;			// Transfer ownership without removing any reference.

				object_ptr_ = nullptr;			// Clear the source.

			}

		}

		template <typename TCOM>
		inline COMPtr<TCOM>::operator bool() const{

			return object_ptr_ != nullptr;

		}

		template <typename TCOM>
		inline TCOM* COMPtr<TCOM>::operator->() const{

			return object_ptr_;

		}

		template <typename TCOM>
		inline TCOM& COMPtr<TCOM>::operator*() const{

			return *object_ptr_;

		}

		template <typename TCOM>
		TCOM* COMPtr<TCOM>::Get() const{

			return object_ptr_;

		}

		template <typename TCOM>
		inline void COMPtr<TCOM>::Release(){

			if (object_ptr_){

				object_ptr_->Release();

				object_ptr_ = nullptr;

			}

		}

		template <typename TCOM>
		inline void COMPtr<TCOM>::AddRef(){

			if (object_ptr_){

				object_ptr_->AddRef();

			}

		}

		/////////////////////// COM MOVE //////////////////////////////

		template <typename TCOM>
		inline COMPtr<TCOM> COMMove(TCOM** object){

			COMPtr<TCOM> pointer;

			pointer << object;

			return pointer;

		}

	}
	
}


#endif