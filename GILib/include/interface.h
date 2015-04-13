/// \file interface.h
/// \brief Defines the base classes used to manage the scene.
///
/// \author Raffaele D. Facendola

#pragma once

#include <typeinfo>
#include <typeindex>
#include <memory>
#include <unordered_map>
#include <vector>
#include <iterator>

#include "macros.h"
#include "range.h"
#include "debug.h"
#include "observable.h"

using std::type_index;
using std::unique_ptr;
using std::unordered_multimap;
using std::vector;
using std::pair;

namespace gi_lib{

	class Object;

	class Interface;

	/// \brief Multi-interface object where interfaces can be plugged into.
	/// \author Raffaele D. Facendola
	class Object{

	public:

		using InterfaceMapType = unordered_multimap < type_index, Interface* > ;

		using InterfaceMapIterator = InterfaceMapType::iterator;

		using InterfaceMapRange = Range < InterfaceMapIterator > ;

		template <typename TInterface>
		struct IteratorMapper{

			TInterface& operator()(InterfaceMapIterator& iterator);

		};

		template <typename TInterface>
		using iterator = IteratorWrapper < InterfaceMapIterator, TInterface, IteratorMapper<TInterface> > ;

		template <typename TInterface>
		using const_iterator = IteratorWrapper < InterfaceMapIterator, const TInterface, IteratorMapper<TInterface> > ;

		template <typename TInterface>
		using range = Range < iterator < TInterface > > ;

		template <typename TInterface>
		using const_range = Range < const_iterator< TInterface > > ;

		/// \brief Create a new interface and adds it to the current object.
		/// \tparam TInterface Type of the interface to add.
		/// \tparam TArgs Type of the arguments to pass to the interface's constructor.
		/// \param Arguments Arguments to pass to the interface's constructor.
		/// \return Returns a pointer to the newely-created interface.
		template < typename TInterface, typename... TArgs >
		TInterface* AddInterface(TArgs&&... arguments);

		/// \brief Remove an interface from this object.
		/// \param ptr Pointer to the interface to remove.
		void RemoveInterface(Interface* ptr);

		/// \brief Get the first interface that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a pointer to the first interface that can be casted to TInterface or null if no such interface exists.
		template < typename TInterface >
		TInterface* GetInterface();

		/// \brief Get the first interface that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a pointer to the first interface that can be casted to TInterface or null if no such interface exists.
		template < typename TInterface >
		const TInterface* GetInterface() const;

		/// \brief Get a range containing all the interfaces that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a range containing all the interfaces that can be casted to TInterface.
		template < typename TInterface >
		range<TInterface> GetInterfaces();

		/// \brief Get a range containing all the interfaces that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a range containing all the interfaces that can be casted to TInterface.
		template < typename TInterface >
		const_range<TInterface> GetInterfaces() const;

	private:

		/// \brief Add a new interface.
		/// \param ptr Pointer to the interface to add and manage via the arbiter.
		/// \return Returns a pointer to the interface.
		Interface* AddInterface(unique_ptr<Interface> ptr);

		/// \brief Get the first interface matching the specified type.
		/// The returned pointer is guaranteed to be an instance of the specified type.
		/// \param interface_type Type of the interface to find.
		/// \return Returns a pointer to the first interface matching the specified type if any. Returns nullptr otherwise.
		/// \remarks The method is guaranteed to run in constant time.
		Interface* GetInterface(type_index interface_type);

		/// \brief Get the interfaces matching the specified type.
		/// The returned pointer is guaranteed to be an instance of the specified type.
		/// \param interface_type Type of the interfaces to get.
		/// \return Returns a pair of iterators. The first element points to the begin of the range, while the second points to one element past the end of the range.
		///			If no interface could be found, the range is empty and both the elements point at the same (invalid) location.
		/// \remarks The method is guaranteed to run in constant time.
		InterfaceMapRange GetInterfaces(type_index interface_type);

		vector<unique_ptr<Interface>> interfaces_;		///< \brief List of the interfaces.

		InterfaceMapType interface_map_;				///< \brief Associate the interface types with the actual interfaces to achieve constant lookup time.

	};

	/// \brief Base class for other interfaces.
	/// \tparam TObject Sepecialized object type. Must derive from Object.
	/// \auhtor Raffaele D. Facendola.
	class Interface{

		friend class Object;

	public:

		/// \brief Type of the range used to iterate trough object's interfaces.
		template <typename TInterface>
		using range = typename Object::range < TInterface > ;

		/// \brief Type of the range used to iterate trough object's constant interfaces.
		template <typename TInterface>
		using const_range = typename Object::const_range < TInterface > ;

		/// \brief Destroy the interface.
		virtual ~Interface();

		/// \brief Get the first interface that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a pointer to the first interface that can be casted to TInterface or null if no such interface exists.
		template <typename TInterface>
		TInterface* GetInterface();

		/// \brief Get the first interface that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a pointer to the first interface that can be casted to TInterface or null if no such interface exists.
		template <typename TInterface>
		const TInterface* GetInterface() const;

		/// \brief Get a range containing all the interfaces that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a range containing all the interfaces that can be casted to TInterface.
		template <typename TInterface>
		range<TInterface> GetInterfaces();

		/// \brief Get a range containing all the interfaces that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a range containing all the interfaces that can be casted to TInterface.
		template <typename TInterface>
		const_range<TInterface> GetInterfaces() const;

		/// \brief Get the object this interface refers to.
		/// \return Returns the object this interface refers to.
		Object* GetObject();

		/// \brief Get the object this interface refers to.
		/// \return Returns the object this interface refers to.
		const Object* GetObject() const;

		/// \brief Get the set of all the types this interface can be safely casted to.
		/// \return Return the set of all the types this interface can be safely casted to.
		vector<type_index> GetTypes() const;

	protected:

		/// \brief Add new types to a type vector.
		/// Used to determine the types this interface can be casted to.
		virtual void GetTypes(vector<type_index>& types) const;

		/// \brief Protected ctor to prevent instantiation.
		Interface();

	private:

		Object* object_;					///< \brief Object this interface refers to.

	};

	///////////////////////// OBJECT ////////////////////////////

	template <typename TInterface>
	TInterface& Object::IteratorMapper<TInterface>::operator()(InterfaceMapIterator& iterator){

		return *static_cast<TInterface*>(iterator->second);

	}

	template < typename TInterface, typename... TArgs >
	TInterface* Object::AddInterface(TArgs&&... arguments){

		return static_cast<TInterface*>(AddInterface(make_unique<TInterface>(std::forward(arguments)...)));

	}

	template <typename TInterface>
	TInterface* Object::GetInterface(){

		return static_cast<TInterface*>(GetInterface(type_index(typeid(TInterface))));

	}

	template <typename TInterface>
	const TInterface* Object::GetInterface() const{

		return static_cast<TInterface*>(GetInterface(type_index(typeid(TInterface))));

	}
	
	template <typename TInterface>
	Object::range<TInterface> Object::GetInterfaces(){

		auto r = GetInterfaces(type_index(typeid(TInterface)));

		IteratorMapper<TInterface> mapper;

		return range<TInterface>(iterator<TInterface>(r.begin(), mapper),
								 iterator<TInterface>(r.end(), mapper));

	}

	template <typename TInterface>
	Object::const_range<TInterface> Object::GetInterfaces() const{
		
		auto r = GetInterfaces(type_index(typeid(TInterface)));

		IteratorMapper<TInterface> mapper;

		return const_range<TInterface>(const_iterator<TInterface>(r.begin(), mapper),
									   const_iterator<TInterface>(r.end(), mapper));

	}

	///////////////////////// INTERFACE ////////////////////////////
	
	template <typename TInterface>
	TInterface* Interface::GetInterface(){

		return object_->GetInterface<TInterface>();

	}

	template <typename TInterface>
	const TInterface* Interface::GetInterface() const{

		return object_->GetInterface<TInterface>();

	}

	template <typename TInterface>
	Interface::range<TInterface> Interface::GetInterfaces(){

		return object_->GetInterfaces<TInterface>();

	}

	template <typename TInterface>
	Interface::const_range<TInterface> Interface::GetInterfaces() const{

		return object_->GetInterfaces<TInterface>();

	}

}