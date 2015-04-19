/// \file observable.h
/// \brief Classes used to implement and manage the observer pattern.
///
/// \author Raffaele D. Facendola

#pragma once

#include <map>
#include <functional>
#include <memory>
#include <tuple>

#include "unique.h"

using ::std::map;
using ::std::function;
using ::std::unique_ptr;
using ::std::tuple;

namespace gi_lib{

	class BaseObservable;
	class Listener;

	/// \brief Tag associated to listener's identifiers.
	struct ListenerTag{};
	
	/// \brief Represents a listener-to-subject relationship.
	/// \author Raffaele D. Facendola.
	class Listener{

		friend class BaseObservable;

	public:

		/// \brief Create a new listener.
		/// \param observable Subject.
		/// \param id Unique id.
		Listener(BaseObservable* observable, Unique<ListenerTag> id);

		/// \brief Default constructor.
		Listener();

		/// \brief No copy constructor.
		Listener(const Listener&) = delete;

		/// \brief No assignment operator.
		Listener& operator=(Listener) = delete;

		/// \brief Destructor.
		~Listener();

		/// \brief Unsubscribe from the current event.
		void Unsubscribe();
		
		/// \brief Get the listener's id.
		/// \return Returns the listener's id.
		Unique<ListenerTag> GetId();

	private:

		/// \brief Invalidate the state of the listener.
		void Invalidate();

		Unique<ListenerTag> id_;	///< \brief Unique identifier.

		BaseObservable* subject_;	///< \brief Observed object.

	};

	/// \brief Base class for observable objects.
	/// \author Raffaele D. Facendola.
	class BaseObservable{

		friend class Listener;

	public:

		/// \brief Default constructor.
		BaseObservable();

		/// \brief No copy constructor.
		BaseObservable(const BaseObservable&) = delete;

		/// \brief Destructor.
		virtual ~BaseObservable();

		/// \brief No assignment operator.
		BaseObservable& operator=(const BaseObservable&) = delete;
		
	protected:

		/// \brief Unsubscribe a listener by id.
		virtual void Unsubscribe(Unique<ListenerTag> listener_id) = 0;

		/// \brief Invalidate a listener.
		virtual void Invalidate(Listener& listener);

		/// \brief Generate a new listener.
		unique_ptr<Listener> GenerateListener();

	private:

		/// \brief Unsubscribe a listener from the observable object.
		void Unsubscribe(Listener& listener);

	};

	/// \brief Dummy structure used as arguments for events that do not have any arguments to relay.
	struct EventArgs{
	
		/// \brief Empty event arguments.
		static const EventArgs kEmpty;
	
	};

	/// \brief Observable object.
	/// \tparam TArgument Type of the argument that is sent during notification.
	/// \author Raffaele D. Facendola.
	template <typename TArgument = EventArgs>
	class Observable : public BaseObservable{

	public:

		/// \brief Default constructor.
		Observable();

		/// \brief Destructor.
		virtual ~Observable();

		/// \brief Subscribe a listener to the current instance.
		/// The provided listener function must have signature (Listener&, TArgument&) -> void.
		/// \tparam TListener Type of the listener function.
		template <typename TListener>
		unique_ptr<Listener> Subscribe(TListener listener);

	protected:

		struct ListenerEntry{

			Listener* listener;

			function<void(Listener&, TArgument&)> callback;

		};

		using ListenerMapType = map < Unique<ListenerTag>, ListenerEntry >;

		virtual void Unsubscribe(Unique<ListenerTag> listener_id) override;

		ListenerMapType listeners_;		///< \brief Map of the listeners.

	};

	/// \brief Observable event.
	/// \tparam TArgument Type of the argument that is sent during notification.
	/// \author Raffaele D. Facendola.
	template <typename TArgument>
	class Event : public Observable < TArgument > {

	public:

		/// \brief Destructor.
		virtual ~Event();

		/// \brief Notify all the listeners.
		void Notify(TArgument& argument);

	};

	////////////////////// OBSERVABLE ///////////////////////////
	
	template <typename TArgument>
	Observable<TArgument>::Observable(){}

	template <typename TArgument>
	Observable<TArgument>::~Observable(){

		// Invalidate all the listeners

		for (auto& it : listeners_){

			Invalidate(*(it.second.listener));

		}

	}

	template <typename TArgument>
	template <typename TListener>
	unique_ptr<Listener> Observable<TArgument>::Subscribe(TListener listener){

		auto listener_ptr = GenerateListener();
				
		listeners_.insert(ListenerMapType::value_type(listener_ptr->GetId(),
													  ListenerEntry{ listener_ptr.get(),
																	 std::move(listener) }));

		return listener_ptr;

	}

	template <typename TArgument>
	void Observable<TArgument>::Unsubscribe(Unique<ListenerTag> listener_id){

		auto it = listeners_.find(listener_id);

		if (it != listeners_.end()){

			listeners_.erase(it);

		}

	}

	////////////////////// EVENT ///////////////////////////

	template <typename TArgument>
	Event<TArgument>::~Event(){}

	template <typename TArgument>
	void Event<TArgument>::Notify(TArgument& argument){

		ListenerMapType listeners = ListenerMapType(listeners_);

		for (auto& it : listeners){

			// The callback may modify the listener list...

			it.second.callback(*(it.second.listener), argument);

		}

	}

}