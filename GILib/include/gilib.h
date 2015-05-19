/// \file gilib.h
/// \brief Base classes and methods
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <locale>
#include <codecvt>

#include "macros.h"
#include "exceptions.h"

namespace gi_lib{

	template <typename TObject>
	class ObjectPtr;

	template <typename TObject>
	class ObjectWeakPtr;

	class Object;

	/// \brief Don't care.
	/// Use this structure when you don't need a parameter in a lambda expression
	struct _{

		template <typename... TArguments>
		_(TArguments&&...){}

	};
	
	/// \brief Used to count strong and weak references of a shared object.
	/// Whenever the strong reference count drops to 0, the object is deleted.
	/// Whenever the weak reference count drops to 0, this object is deleted.
	/// The weak reference count is increased by 1 if the strong reference count is greater than 0.
	/// \author Raffaele D. Facendola.
	class RefCountObject{

	public:

		/// \brief Create a new reference count object with an actual pointer.
		/// \param object The managed object pointer.
		RefCountObject(Object* object);

		/// \brief Get the object.
		Object* Get() const;

		/// \brief Adds a strong reference.
		void AddRef();

		/// \brief Remove a strong reference.
		void Release();

		/// \brief Add a weak reference.
		void AddWeakRef();

		/// \brief Remove a weak reference.
		void WeakRelease();

	private:

		size_t ref_count_;		///< \brief Strong reference count.

		size_t weak_count_;		///< \brief Weak reference count.

		Object* object_;		///< \brief Actual object pointer.

	};

	/// \brief Base interface for every object whose life cycle is determined by a reference counter.
	/// \author Raffaele D. Facendola
	class Object{
		
		template <typename TObject>
		friend class ObjectPtr;

		template <typename TObject>
		friend class ObjectWeakPtr;

	public:

		/// \brief Default destructor.
		Object();

		/// \brief Virtual class.
		virtual ~Object();

		/// \brief No assignment operator.
		/// An assignment operator would break the ref counter object.
		Object& operator=(const Object&) = delete;
		
	private:

		RefCountObject* ref_count_object_;			///< \brief Reference counter.

	};
	
	/// \brief Strong reference to an object.
	/// The pointer will add a reference during initialization and remove one during destruction.
	/// \remarks This class is not thread safe.
	/// \todo Make this class thread safe.
	template <typename TObject>
	class ObjectPtr{

		template <typename TFriend>
		friend class ObjectPtr;

	public:

		/// \brief Create an empty pointer.
		ObjectPtr();

		/// \brief Create an empty pointer.
		ObjectPtr(nullptr_t);

		/// \brief Defines a pointer to an object.
		/// \param object Object that will be pointed by this pointer.
		template <typename TOther>
		ObjectPtr(TOther* object);

		/// \brief Copy constructor.
		/// \param other Other pointer to copy.
		template <typename TOther>
		ObjectPtr(const ObjectPtr<TOther>& other);

		/// \brief Move constructor.
		/// \param other Instance to move.
		template <typename TOther>
		ObjectPtr(ObjectPtr<TOther>&& other);

		/// \brief Destructor.
		/// Decreases by one the reference count of the pointed object, if any.
		~ObjectPtr();

		/// \brief Unified assignment operator.
		ObjectPtr<TObject>& operator=(ObjectPtr<TObject> other);

		/// \brief Equality operator.
		/// \return Returns true if both this instance and the specified one points to the same object, returns false otherwise.
		bool operator==(const ObjectPtr<TObject>& other) const;

		/// \brief Inequality operator.
		/// \return Returns true if this instance and the specified one points to different objects, returns false otherwise.
		bool operator!=(const ObjectPtr<TObject>& other) const;

		/// \brief Used to validate the pointed object.
		/// \return Returns true if the pointed object is not null, returns false otherwise.
		operator bool() const;

		/// \brief Arrow operator.
		/// Access the managed object.
		TObject* operator->() const;

		/// \brief Dereferencing operator.
		/// Access the managed object.
		TObject& operator*() const;
				
		/// \brief Get a pointer to the managed object.
		TObject* Get() const;

		/// \brief Release the pointed object.
		void Release();

	private:

		/// \brief Add a reference to the pointed object.
		void AddRef();

		/// \brief Swaps this instance with another one.
		void Swap(ObjectPtr<TObject>& other);

		/// \brief Get the reference count object.
		RefCountObject& GetRefCountObject();

		TObject* object_ptr_;			/// \brief Pointer to the object.

	};

	/// \brief Weak reference to an object.
	/// The pointer will add a weak reference during initialization and remove one during destruction.
	/// \remarks This class is not thread safe.
	/// \todo Make this class thread safe.
	template <typename TObject>
	class ObjectWeakPtr{

		template <typename TFriend>
		friend class ObjectWeakPtr;

	public:

		/// \brief Create an empty pointer.
		ObjectWeakPtr();

		/// \brief Create an empty pointer.
		ObjectWeakPtr(nullptr_t);

		/// \brief Defines a pointer to an object.
		/// \param object Object that will be pointed by this pointer.
		template <typename TOther>
		ObjectWeakPtr(TOther* object);

		/// \brief Copy constructor.
		/// \param other Other pointer to copy.
		template <typename TOther>
		ObjectWeakPtr(const ObjectWeakPtr<TOther>& other);

		/// \brief Create a weak pointer from a strong reference.
		/// \param other Other pointer to copy.
		template <typename TOther>
		ObjectWeakPtr(const ObjectPtr<TOther>& other);

		/// \brief Move constructor.
		/// \param other Instance to move.
		template <typename TOther>
		ObjectWeakPtr(ObjectWeakPtr<TOther>&& other);

		/// \brief Destructor.
		/// Decreases by one the weak reference count of the pointed object, if any.
		~ObjectWeakPtr();

		/// \brief Unified assignment operator.
		ObjectWeakPtr<TObject>& operator=(ObjectWeakPtr<TObject> other);

		/// \brief Equality operator.
		/// \return Returns true if both this instance and the specified one points to the same object, returns false otherwise.
		bool operator==(const ObjectWeakPtr<TObject>& other) const;

		/// \brief Inequality operator.
		/// \return Returns true if this instance and the specified one points to different objects, returns false otherwise.
		bool operator!=(const ObjectWeakPtr<TObject>& other) const;

		/// \brief Used to validate the pointed object.
		/// \return Returns true if the pointed object is not null, returns false otherwise.
		operator bool() const;

		/// \brief Used to validate the pointed object.
		/// \return Returns true if the pointed object is not null, returns false otherwise.
		bool IsValid() const;

		/// \brief Locks and create a strong reference to the pointed object.
		/// \return Returns a strong reference to the pointed object if it was still valid, returns a pointer to null otherwise.
		ObjectPtr<TObject> Lock() const;

		/// \brief Release a weak reference from the pointed object.
		void Release();

	private:

		/// \brief Add a weak reference to the pointed object.
		void AddRef();

		/// \brief Swaps this instance with another one.
		void Swap(ObjectWeakPtr<TObject>& other);

		RefCountObject* ref_count_object_;			///< \brief Weak reference to the pointed object.

	};

	/// \brief Converts a string to a wstring.
	inline std::wstring to_wstring(const std::string& string)
	{

		return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(string);

	}

	/// \brief Converts a wstring to a string.
	inline std::string to_string(const std::wstring& wstring)
	{

		return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(wstring);

	}

	///////////////////////////////// REF COUNT OBJECT ////////////////////////

	inline RefCountObject::RefCountObject(Object* object) :
	object_(object),
	ref_count_(0),
	weak_count_(0){}

	inline Object* RefCountObject::Get() const{

		return object_;

	}

	inline void RefCountObject::AddRef(){

		if (ref_count_ == 0){

			++weak_count_;

		}

		++ref_count_;

	}

	inline void RefCountObject::Release(){

		--ref_count_;

		if (ref_count_ == 0){

			delete object_;

			object_ = nullptr;

			WeakRelease();

		}

	}

	inline void RefCountObject::AddWeakRef(){

		++weak_count_;

	}

	inline void RefCountObject::WeakRelease(){

		--weak_count_;

		if (weak_count_ == 0){

			delete this;

		}

	}

	///////////////////////////////// OBJECT //////////////////////////////////

	inline Object::Object() :
		ref_count_object_(new RefCountObject(this)){}

	inline Object::~Object(){}

	///////////////////////////////// OBJECT PTR //////////////////////////////////

	template <typename TObject>
	inline ObjectPtr<TObject>::ObjectPtr() :
		object_ptr_(nullptr){}

	template <typename TObject>
	inline ObjectPtr<TObject>::ObjectPtr(nullptr_t) :
		ObjectPtr(){}

	template <typename TObject>
	template <typename TOther>
	inline ObjectPtr<TObject>::ObjectPtr(TOther* object) :
		object_ptr_(static_cast<TObject*>(object)){
		
		AddRef();

	}

	template <typename TObject>
	template <typename TOther>
	inline ObjectPtr<TObject>::ObjectPtr(const ObjectPtr<TOther>& other) :
		ObjectPtr(static_cast<TObject*>(other.Get())){}

	template <typename TObject>
	template <typename TOther>
	inline ObjectPtr<TObject>::ObjectPtr(ObjectPtr<TOther>&& other) :
		object_ptr_(static_cast<TObject*>(other.Get())){

		other.object_ptr_ = nullptr;

	}

	template <typename TObject>
	inline ObjectPtr<TObject>::~ObjectPtr(){

		Release();

	}

	template <typename TObject>
	inline ObjectPtr<TObject>& ObjectPtr<TObject>::operator=(ObjectPtr<TObject> other){

		other.Swap(*this);

		return *this;

	}

	template <typename TObject>
	inline bool ObjectPtr<TObject>::operator==(const ObjectPtr<TObject>& other) const{

		return object_ptr_ == other.object_ptr_;

	}

	template <typename TObject>
	inline bool ObjectPtr<TObject>::operator!=(const ObjectPtr<TObject>& other) const{

		return object_ptr_ != other.object_ptr_;

	}

	template <typename TObject>
	inline ObjectPtr<TObject>::operator bool() const{

		return object_ptr_ != nullptr;

	}

	template <typename TObject>
	inline TObject* ObjectPtr<TObject>::operator->() const{

		return object_ptr_;

	}

	template <typename TObject>
	inline TObject& ObjectPtr<TObject>::operator*() const{

		return *object_ptr_;

	}

	template <typename TObject>
	TObject* ObjectPtr<TObject>::Get() const{

		return object_ptr_;

	}

	template <typename TObject>
	inline void ObjectPtr<TObject>::Swap(ObjectPtr<TObject>& other){

		std::swap(object_ptr_, 
				  other.object_ptr_);

	}

	template <typename TObject>
	inline void ObjectPtr<TObject>::Release(){

		if (object_ptr_){

			GetRefCountObject().Release();

			object_ptr_ = nullptr;

		}

	}

	template <typename TObject>
	inline void ObjectPtr<TObject>::AddRef(){

		if (object_ptr_){

			GetRefCountObject().AddRef();

		}

	}

	template <typename TObject>
	inline RefCountObject& ObjectPtr<TObject>::GetRefCountObject(){

		return *static_cast<const Object*>(object_ptr_)->ref_count_object_;

	}

	///////////////////////////////// WEAK OBJECT PTR ////////////////////////////////

	template <typename TObject>
	inline ObjectWeakPtr<TObject>::ObjectWeakPtr() :
		ref_count_object_(nullptr){}

	template <typename TObject>
	inline ObjectWeakPtr<TObject>::ObjectWeakPtr(nullptr_t) :
		ObjectWeakPtr(){}

	template <typename TObject>
	template <typename TOther>
	inline ObjectWeakPtr<TObject>::ObjectWeakPtr(TOther* object) :
		ref_count_object_(static_cast<const Object*>(object)->ref_count_object_){
		
		static_assert(std::is_base_of<TObject, TOther>::value, "TOther must derive from TObject.");

		AddRef();

	}

	template <typename TObject>
	template <typename TOther>
	inline ObjectWeakPtr<TObject>::ObjectWeakPtr(const ObjectWeakPtr<TOther>& other) :
		ObjectWeakPtr(other.Lock()){}

	template <typename TObject>
	template <typename TOther>
	inline ObjectWeakPtr<TObject>::ObjectWeakPtr(const ObjectPtr<TOther>& other) :
		ObjectWeakPtr(other.Get()){}

	template <typename TObject>
	template <typename TOther>
	inline ObjectWeakPtr<TObject>::ObjectWeakPtr(ObjectWeakPtr<TOther>&& other) :
		ref_count_object_(other.ref_count_object_){

		static_assert(std::is_base_of<TObject, TOther>::value, "TOther must derive from TObject.");

		other.ref_count_object_ = nullptr;

	}

	template <typename TObject>
	inline ObjectWeakPtr<TObject>::~ObjectWeakPtr(){

		Release();

	}

	template <typename TObject>
	inline ObjectWeakPtr<TObject>& ObjectWeakPtr<TObject>::operator=(ObjectWeakPtr<TObject> other){

		other.Swap(*this);

		return *this;

	}
	
	template <typename TObject>
	inline bool ObjectWeakPtr<TObject>::operator==(const ObjectWeakPtr<TObject>& other) const{

		return object_ptr_ == other.object_ptr_;

	}

	template <typename TObject>
	inline bool ObjectWeakPtr<TObject>::operator!=(const ObjectWeakPtr<TObject>& other) const{

		return object_ptr_ != other.object_ptr_;

	}

	template <typename TObject>
	inline ObjectWeakPtr<TObject>::operator bool() const{

		return IsValid();

	}

	template <typename TObject>
	inline bool ObjectWeakPtr<TObject>::IsValid() const{

		return ref_count_object_ != nullptr &&
			   ref_count_object_->Get() != nullptr;

	}

	template <typename TObject>
	inline ObjectPtr<TObject> ObjectWeakPtr<TObject>::Lock() const{

		return IsValid() ?
			   static_cast<TObject*>(ref_count_object_->Get()) :
			   nullptr;
						
	}

	template <typename TObject>
	inline void ObjectWeakPtr<TObject>::Swap(ObjectWeakPtr<TObject>& other){

		std::swap(ref_count_object_,
				  other.ref_count_object_);

	}

	template <typename TObject>
	inline void ObjectWeakPtr<TObject>::Release(){

		if (ref_count_object_){

			ref_count_object_->WeakRelease();

			ref_count_object_ = nullptr;

		}

	}

	template <typename TObject>
	inline void ObjectWeakPtr<TObject>::AddRef(){

		if (ref_count_object_){

			ref_count_object_->AddWeakRef();

		}

	}

}