/// \file interface.h
/// \brief Defines the base classes used to manage the scene.
///
/// \author Raffaele D. Facendola

#include <typeinfo>
#include <typeindex>
#include <memory>
#include <unordered_map>
#include <vector>
#include <iterator>

#include "macros.h"
#include "range.h"

using std::type_index;
using std::unique_ptr;
using std::unordered_multimap;
using std::vector;
using std::pair;

namespace gi_lib{

	class Interface;
	class InterfaceArbiter;

	/// \brief Manages the communication between different interfaces.
	/// An arbiter is alive as long as it holds at least one interface: whenever the last one is deleted, the arbiter is destroyed.
	/// \author Raffaele D. Facendola.
	class InterfaceArbiter{

	public:

		/// \brief Type of the multimap used to associate a type index to the interfaces.
		using InterfaceMapType = unordered_multimap < type_index, Interface* > ;

		/// \brief Type of the iterator used to iterate trough interfaces.
		using iterator = InterfaceMapType::iterator;
		
		/// \brief Type of the interfaces range.
		using range = Range < iterator > ;

		/// \brief Create a new arbiter and assign a new interface to it.
		InterfaceArbiter(unique_ptr<Interface> interface_ptr);

		/// \brief No copy constructor.
		InterfaceArbiter(const InterfaceArbiter&) = delete;

		/// \brief Add a new interface.
		/// \param interface_ptr Pointer to the interface to add and manage via the arbiter.
		/// \return Returns a pointer to the interface.
		Interface* AddInterface(unique_ptr<Interface> interface_ptr);

		/// \brief Remove an interface and delete it.
		/// \param interface_ptr Pointer to the interface to delete.
		/// \param suppress_destructor Whether to delete the interface or simply remove it. This parameter is useful to remove partially-destroyed interfaces.
		void RemoveInterface(Interface* interface_ptr);

		/// \brief Destroy the arbiter and every assigned interface.
		/// \param instigator The interface which caused the arbiter destruction. This interface is no longer managed by the arbiter and may be reused.
		/// \remark This method actually calls "delete" on the instance!
		void Destroy(Interface* instigator);

		/// \brief Remove all the interfaces matching the specified type and delete them.
		/// \param interface_type Type of the interfaces to remove.
		void RemoveInterfaces(type_index interface_type);

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
		range GetInterfaces(type_index interface_type);

	protected:

		/// \brief Destroy the arbiter and every interface associated to it.
		~InterfaceArbiter();

	private:

		vector<unique_ptr<Interface>> interfaces_;		///< \brief List of the interfaces managed by the arbiter.

		InterfaceMapType interface_map_;				///< \brief Associate the interface types with the actual interfaces to achieve constant lookup time.

	};

	/// \brief Class for every interface.
	/// An interface exposes functionalities that can be added and removed at runtime.
	/// \auhtor Raffaele D. Facendola.
	class Interface{

		friend class InterfaceArbiter;

	public:

		template <typename TInterface>
		struct IteratorMapper{
			
			TInterface& operator()(InterfaceArbiter::iterator& iterator);

		};
		
		template <typename TInterface>
		using iterator = IteratorWrapper < InterfaceArbiter::iterator, TInterface, IteratorMapper<TInterface> >;

		template <typename TInterface>
		using const_iterator = IteratorWrapper < InterfaceArbiter::iterator, const TInterface, IteratorMapper<TInterface> >;

		template <typename TInterface>
		using range = Range < iterator < TInterface > > ;

		template <typename TInterface>
		using const_range = Range < const_iterator< TInterface > > ;

		/// \brief Destroy the interface.
		/// If an interface is deleted explicitly, this method causes the destruction of every other interface associated to the entity in order to prevent memory leaks.
		/// If the real intention was to remove the functionalities associated to this particular interface, consider using the RemoveInterface method instead.
		virtual ~Interface();

		/// \brief Create a new interface and adds it to the current object.
		/// \tparam TInterface Type of the interface to add.
		/// \tparam TArgs Type of the arguments to pass to the interface's constructor.
		/// \param Arguments Arguments to pass to the interface's constructor.
		/// \return Returns a pointer to the newely-created interface.
		template <typename TInterface, typename... TArgs>
		DERIVES_FROM_T(TInterface, Interface, TInterface*) AddInterface(TArgs&&... arguments);

		/// \brief Remove this interface.
		/// \remarks If this interface was the last one, the object gets destroyed. In this case the method has the same effect of "delete this".
		void RemoveInterface();

		/// \brief Remove all the interfaces that can be casted to TInterface (base or derived).
		/// \tparam TInterface Type of interface to test against.
		template <typename TInterface, DERIVES_FROM(TInterface, Interface)>
		void RemoveInterfaces();

		/// \brief Get the first interface that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a pointer to the first interface that can be casted to TInterface or null if no such interface exists.
		template <typename TInterface, DERIVES_FROM(TInterface, Interface)>
		TInterface* GetInterface();

		/// \brief Get the first interface that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a pointer to the first interface that can be casted to TInterface or null if no such interface exists.
		template <typename TInterface, DERIVES_FROM(TInterface, Interface)>
		const TInterface* GetInterface() const;

		/// \brief Get a range containing all the interfaces that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a range containing all the interfaces that can be casted to TInterface.
		template <typename TInterface, DERIVES_FROM(TInterface, Interface)>
		range<TInterface> GetInterfaces();

		/// \brief Get a range containing all the interfaces that can be casted to TInterface.
		/// \tparam TInterface Type of interface to test against.
		/// \return Returns a range containing all the interfaces that can be casted to TInterface.
		template <typename TInterface, DERIVES_FROM(TInterface, Interface)>
		const_range<TInterface> GetInterfaces() const;

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

		/// \brief Get the arbiter reference.
		/// \return Returns a reference to the arbiter.
		InterfaceArbiter& GetArbiter();

		/// \brief Get the arbiter reference.
		/// \return Returns a reference to the arbiter.
		const InterfaceArbiter& GetArbiter() const;
		
		mutable InterfaceArbiter* arbiter_;			///< \brief Used to communicate with other interfaces.
		
	};

	///////////////////////// INTERFACE ////////////////////////////

	template <typename TInterface>
	TInterface& Interface::IteratorMapper<TInterface>::operator()(InterfaceArbiter::iterator& iterator){

		return *static_cast<TInterface*>(iterator->second);
		
	}

	template <typename TInterface, typename... TArgs>
	DERIVES_FROM_T(TInterface, Interface, TInterface*) Interface::AddInterface(TArgs&&... arguments){

		return static_cast<TInterface*>(GetArbiter().AddInterface(make_unique<TInterface>(std::forward(arguments)...)));

	}

	inline void Interface::RemoveInterface(){

		GetArbiter().RemoveInterface(this);

	}

	template <typename TInterface, DERIVES_FROM_DEF(TInterface, Interface)>
	void Interface::RemoveInterfaces(){

		GetArbiter().RemoveInterfaces(type_index(typeid(TInterface)));

	}

	template <typename TInterface, DERIVES_FROM_DEF(TInterface, Interface)>
	TInterface* Interface::GetInterface(){

		return static_cast<TInterface*>(GetArbiter().GetInterface(type_index(typeid(TInterface))));

	}

	template <typename TInterface, DERIVES_FROM_DEF(TInterface, Interface)>
	const TInterface* Interface::GetInterface() const{

		return static_cast<TInterface*>(GetArbiter().GetInterface(type_index(typeid(TInterface))));

	}

	template <typename TInterface, DERIVES_FROM_DEF(TInterface, Interface)>
	Interface::range<TInterface> Interface::GetInterfaces(){

		auto range = GetArbiter().GetInterfaces(type_index(typeid(TInterface)));

		IteratorMapper<TInterface> mapper;

		return Interface::range<TInterface>(iterator<TInterface>(range.begin(), mapper),
										    iterator<TInterface>(range.end(), mapper));

	}

	template <typename TInterface, DERIVES_FROM_DEF(TInterface, Interface)>
	Interface::const_range<TInterface> Interface::GetInterfaces() const{

		auto range = GetArbiter().GetInterfaces(type_index(typeid(TInterface)));

		return const_range<TInterface>(const_iterator<TInterface>(range.begin(), mapper),
									   const_iterator<TInterface>(range.end(), mapper));
	}


}