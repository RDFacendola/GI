/// \file instance_builder.h
/// \brief Classes used to programmatically create class instances from template data.
///
/// \author Raffaele D. Facendola

#pragma once

#include <typeindex>
#include <typeinfo>
#include <functional>
#include <utility>
#include <map>

#include "macros.h"

using ::std::map;
using ::std::type_index;
using ::std::function;
using ::std::pair;

#define INSTANTIABLE_2(ClassName, Arguments) \
	static InstanceRegisterer<ClassName, ClassName, Arguments> ANONYMOUS;

#define INSTANTIABLE_3(ClassName, ClassType, Arguments) \
	static InstanceRegisterer<ClassName, ClassType, Arguments> ANONYMOUS;

/// \brief Macro selector. If the provided expression fails the caller returns either the failure value or a specified value.
#define INSTANTIABLE(...) EXPAND( SELECT_4TH(__VA_ARGS__ , INSTANTIABLE_3, INSTANTIABLE_2)(__VA_ARGS__ ) )

namespace gi_lib{

	/// \brief Class used to build class instances from metadata programmatically.
	/// Only one argument constructor are supported.
	/// \author Raffaele D. Facendola
	class InstanceBuilder{

	public:

		/// \brief Register a new class.
		/// \tparam TClass Type of the class to register.
		/// \tparam TConcrete Concrete class to instantiate. Must derive from TClass.
		/// \tparam TArgs Type of the arguments required by the instance's constructor.
		/// A single class can be registered multiple times with different argument types.
		template <typename TClass, typename TConcrete, typename TArgs/*, DERIVES_FROM(TConcrete, TClass)*/>
		static void Register();

		/// \brief Register a new class.
		/// \tparam TClass Type of the class to register and instantiate.
		/// \tparam TArgs Type of the arguments required by the instance's constructor.
		/// A single class can be registered multiple times with different argument types.
		template <typename TClass, typename TArgs>
		static void Register();

		/// \brief Build a new class instance.
		/// \param class_type Type of the object to instantiate.
		/// \param args_type Type of the arguments required by instance's constructor.
		/// \param args Arguments that will be passed to the instance's constructor.
		/// \return Returns a new instance of the specified class type. Returns nullptr if no match was found for the class-argument type pair.
		/// \remarks If a match is found it is <b>guaranteed<\b> that the returned value can be statically casted to the proper class.
		static void* Build(const type_index& class_type, const type_index& args_type, const void* args);

	private:

		using BuilderMap = map < pair < type_index, type_index >,
								 function<void* (const void*)> >;		// Instance constructor

		static BuilderMap& GetBuilderMap();

	};

	/// \brief Class used to register an instantiable type
	template <typename TClass, typename TConcrete, typename TArgs>
	class InstanceRegisterer{

	public:

		/// \brief Instantiate a new object.
		InstanceRegisterer();

	};

	//////////////////////// INSTANCE BUILDER ///////////////////////

	template <typename TClass, typename TConcrete, typename TArgs/*, DERIVES_FROM_DEF(TConcrete, TClass)*/>
	void InstanceBuilder::Register(){

		// Unique key

		auto builder_key = make_pair(type_index(typeid(TClass)),
									 type_index(typeid(TArgs)));

		auto&& builder_map = GetBuilderMap();

		builder_map[builder_key] = [](const void* args){
		
										return new TConcrete(*static_cast<const TArgs*>(args));

								   };

	}
	
	template <typename TClass, typename TArgs>
	void InstanceBuilder::Register(){

		// The instance will be of the same declared class type.
		Register<TClass, TClass, TArgs>();

	}

	inline void* InstanceBuilder::Build(const type_index& class_type, const type_index& args_type, const void* args){

		auto&& builder_map = GetBuilderMap();

		auto constructor = builder_map.find(make_pair(class_type,
														  args_type));

		return constructor != builder_map.end() ?
			   constructor->second(args) :					// Instantiate a new object.
			   nullptr;										// Not supported
			
	}

	inline InstanceBuilder::BuilderMap& InstanceBuilder::GetBuilderMap(){

		static BuilderMap builder_map;

		return builder_map;

	}

	//////////////////////// INSTANCE REGISTERER ////////////////////////

	template <typename TClass, typename TConcrete, typename TArgs>
	inline InstanceRegisterer<TClass,TConcrete,TArgs>::InstanceRegisterer(){

		InstanceBuilder::Register<TClass, TConcrete, TArgs>();

	}

}