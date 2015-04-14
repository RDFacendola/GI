/// \file interface.h
/// \brief Defines the base classes used to manage the scene.
///
/// \author Raffaele D. Facendola

#pragma once

#include <typeinfo>
#include <typeindex>
#include <memory>
#include <unordered_map>
#include <set>
#include <iterator>

#include "macros.h"
#include "range.h"

using std::type_index;
using std::unordered_multimap;
using std::set;

namespace gi_lib{

	class Object;

	class Interface;

	/// \brief Multi-interface object where interfaces can be plugged into.
	/// \author Raffaele D. Facendola
	class Object{

		friend class Interface;

	public:

		using InterfaceSetType = set < Interface* > ;

		using InterfaceMapType = unordered_multimap < type_index, Interface* > ;

		template <typename TInterface>
		struct IteratorMapper{

			TInterface& operator()(InterfaceMapType::iterator& iterator);

		};

		template <typename TInterface>
		using iterator = IteratorWrapper < InterfaceMapType::iterator, TInterface, IteratorMapper<TInterface> >;

		template <typename TInterface>
		using const_iterator = IteratorWrapper < InterfaceMapType::iterator, const TInterface, IteratorMapper<TInterface> >;

		template <typename TInterface>
		using range = Range < iterator < TInterface > > ;

		template <typename TInterface>
		using const_range = Range < const_iterator< TInterface > > ;

		/// \brief Default constructor.
		Object();
		
		/// \brief No copy constructor.
		Object(const Object&) = delete;

		/// \brief Destructor. Causes the destruction of inner interfaces.
		~Object();

		/// \brief No assignment operator.
		Object& operator=(const Object&) = delete;

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

		/// \brief Destroy the object and all its interfaces.
		/// This method is equivalent to "delete this".
		void Destroy();

	private:

		/// \brief Plug a new interface to the object.
		void AddInterface(Interface* ptr);

		/// \brief Unplug an existing interface from the object.
		/// \param ptr Pointer to the interface to remove.
		void RemoveInterface(Interface* ptr);

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
		Range < InterfaceMapType::iterator > GetInterfaces(type_index interface_type);

		InterfaceSetType interface_set_;	///< \brief Set of the interfaces.

		InterfaceMapType interface_map_;	///< \brief Associate the interface types with the actual interfaces to achieve constant lookup time.

		bool alive_;						///< \brief Is the object alive?

	};

	/// \brief Base interface.
	/// \auhtor Raffaele D. Facendola.
	class Interface{

	public:

		/// \brief Type of the range used to iterate trough object's interfaces.
		template <typename TInterface>
		using range = typename Object::range < TInterface > ;

		/// \brief Type of the range used to iterate trough object's constant interfaces.
		template <typename TInterface>
		using const_range = typename Object::const_range < TInterface > ;

		/// \brief No copy constructor.
		Interface(const Interface&) = delete;

		/// \brief Destructor.
		virtual ~Interface();

		/// \brief No assignment operator.
		Interface& operator=(const Interface&) = delete;

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

		/// \brief Get the composite object.
		/// \return Returns the composite object.
		Object& GetComposite();

		/// \brief Get the composite object.
		/// \return Returns the composite object.
		const Object& GetComposite() const;

		/// \brief Get the set of all the types this interface can be safely casted to.
		/// \return Return the set of all the types this interface can be safely casted to.
		set<type_index> GetTypes() const;

	protected:

		/// \brief Create a new interface.
		/// \param object Composite object this interface is plugged into.
		Interface(Object& object);

		/// \brief Add new types to a type vector.
		/// Used to determine the types this interface can be casted to.
		virtual void GetTypes(set<type_index>& types) const;

	private:

		Object& object_;		///\brief Composite object.
		
	};

	///////////////////////// OBJECT ////////////////////////////

	template <typename TInterface>
	TInterface& Object::IteratorMapper<TInterface>::operator()(Object::InterfaceMapType::iterator& iterator){

		return *static_cast<TInterface*>(iterator->second);

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

		return object_.GetInterface<TInterface>();

	}

	template <typename TInterface>
	const TInterface* Interface::GetInterface() const{

		return object_.GetInterface<TInterface>();

	}

	template <typename TInterface>
	Interface::range<TInterface> Interface::GetInterfaces(){

		return object_.GetInterfaces<TInterface>();

	}

	template <typename TInterface>
	Interface::const_range<TInterface> Interface::GetInterfaces() const{

		return object_.GetInterfaces<TInterface>();

	}

}