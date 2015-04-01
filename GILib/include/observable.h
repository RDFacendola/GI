/// \file observable.h
/// \brief Classes used to implement and manage the observer pattern.
///
/// \author Raffaele D. Facendola

#pragma once

#include <functional>
#include <vector>
#include <tuple>
#include <memory>

#include "unique.h"

using ::std::tuple;
using ::std::function;
using ::std::unique_ptr;

namespace gi_lib{

	/// \brief Tag associated to listener objects.
	struct ListenerTag{};

	/// \brief Listener id type.
	using ListenerId = Unique < ListenerTag > ;

	/// \brief Observable object.
	/// \tparam TArguments Type of arguments this observable delivers to the listeners during notification.
	/// \author Raffaele D. Facendola
	template <typename... TArguments>
	class Observable{

	public:

		/// \brief Add a new listener to this observable.
		/// \tparam TFunctor Type of listener object.
		/// \param functor Actual listener object.
		/// \return Returns the listener id.
		template<typename TFunctor>
		ListenerId AddListener(TFunctor functor);

		/// \brief Remove a listener object from this observable.
		/// \param id Id of the listener to remove.
		void RemoveListener(const ListenerId& id);

	protected:

		using ListenerTableEntry = tuple < ListenerId, function<void(TArguments...)> > ;		///< \brief <id; handler> pair

		std::vector< ListenerTableEntry > listener_table_;										///< \brief List of listeners.
		
	};

	/// \brief Event object.
	/// \tparam TArguments Type of arguments this observable delivers to the listeners during notification.
	/// \author Raffaele D. Facendola
	template <typename... TArguments>
	class Event: public Observable<TArguments...>{

	public:

		/// \brief Notify all the listener objects.
		/// \param args Arguments that will be delivered to each listener object.
		void Notify(TArguments&&... args);

	};
		
	/// \brief Base interface for listener guards.
	class BaseListener{

	public:

		/// \brief Virtual destructor.
		virtual ~BaseListener(){}

	protected:

		/// \brief Protected constructor, prevent instantiation.
		BaseListener(){}

	};

	/// \brief RAII guard for listener objects.
	/// \author Raffaele D. Facendola
	template <typename... TArguments>
	class ListenerGuard : public BaseListener{

	public:

		/// \brief Create a new listener guard.
		/// \param subject The object being observed.
		/// \param listener_id Id associated to the listener.
		ListenerGuard(Observable<TArguments...>& subject, ListenerId listener_id);

		/// \brief No copy constructor.
		ListenerGuard(const ListenerGuard<TArguments...>&) = delete;

		/// \brief Move constructor.
		/// \param other Instance to move.
		ListenerGuard(ListenerGuard<TArguments...>&& other);

		/// \brief Unified assignment operator.
		/// \param other Instance to assign.
		ListenerGuard<TArguments...> operator=(ListenerGuard<TArguments...> other);

		/// \brief Virtual destructor.
		virtual ~ListenerGuard();

	private:

		/// \brief Remove the listener from the subject.
		void RemoveListener();

		Observable<TArguments...>* subject_;	///< \brief Object being observed.

		ListenerId listener_id_;				///< \brief Listener id.

	};

	/////////////////////////////// OBSERVABLE /////////////////////////////// 

	template <typename... TArguments>
	template<typename TFunctor>
	ListenerId Observable<TArguments...>::AddListener(TFunctor functor){

		ListenerId id = ListenerId::MakeUnique();

		listener_table_.push_back(std::make_tuple(id, 
											      std::move(functor)));

		return id;

	}

	template <typename... TArguments>
	void Observable<TArguments...>::RemoveListener(const ListenerId & id){

		listener_table_.erase(std::remove_if(listener_table_.begin(),
											 listener_table_.end(),
											 [&id](const ListenerTableEntry& entry){

												return std::get<0>(entry) == id;

											 }),
							  listener_table_.end());

	}

	/////////////////////////////// EVENT /////////////////////////////// 

	template <typename... TArguments>
	void Event<TArguments...>::Notify(TArguments&&... args){

		for (auto& entry : listener_table_){

			std::get<1>(entry)(std::forward<TArguments>(args)...);

		}

	}
		
	/////////////////////////////// LISTENER GUARD /////////////////////////////// 

	template <typename... TArguments>
	ListenerGuard<TArguments...>::ListenerGuard(Observable<TArguments...>& subject, ListenerId listener_id) :
		subject_(std::addressof(subject)),
		listener_id_(listener_id)
	{}

	template <typename... TArguments>
	ListenerGuard<TArguments...>::ListenerGuard(ListenerGuard<TArguments...>&& other) :
		subject_(other.subject_),
		listener_id_(other.listener_id_)
	{
	
		other.subject_ = nullptr;

	}

	template <typename... TArguments>
	ListenerGuard<TArguments...> ListenerGuard<TArguments...>::operator=(ListenerGuard<TArguments...> other){

		std::swap(subject_, other.subject_);
		std::swap(listener_id_, other.listener_id_);

	}

	template <typename... TArguments>
	ListenerGuard<TArguments...>::~ListenerGuard(){

		RemoveListener();

	}

	template <typename... TArguments>
	void ListenerGuard<TArguments...>::RemoveListener(){

		if (subject_ != nullptr){

			subject_->RemoveListener(listener_id_);
			subject_ = nullptr;

		}

	}

	/////////////////////////////// MISC /////////////////////////////// 

	/// \brief Global type for listener guards.
	using Listener = unique_ptr < BaseListener > ;

	/// \brief Create a new listener guard.
	/// \tparam TFunctor Type of functor called during notifications.
	/// \tparam TArguments Type of the arguments passed during notifications.
	/// \param subject Object being observed.
	/// \param listener_func Functor called during notifications.
	template <typename TFunctor, typename... TArguments>
	Listener make_listener(Observable<TArguments...>& subject, TFunctor listener_func){

		return make_unique<ListenerGuard<TArguments...>>(subject,
														 subject.AddListener(std::move(listener_func)));

	}

}
