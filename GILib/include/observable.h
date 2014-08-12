/// \file observable.h
/// \brief Classes used to implement and manage the observer pattern.
///
/// \author Raffaele D. Facendola

#pragma once

#include <functional>
#include <vector>
#include <algorithm>

#include "unique.h"

namespace gi_lib{

	/// \brief Tag associated to obserable objects's handle.
	struct ListenerTag{};

	/// \brief Key associated to each listener.
	using ListenerKey = Unique<ListenerTag>;

	/// \brief Observable object.
	/// \tparam TArguments Type of arguments this observable delivers to the listeners during notification.
	/// \author Raffaele D. Facendola
	template <typename... TArguments>
	class Observable{

	public:

		/// \brief Add a new listener to this observable.
		/// \tparam TFunctor Type of listener object.
		/// \param functor Actual listener object.
		/// \return Returns an handle that associate the listener object to this observable.
		template<typename TFunctor>
		ListenerKey AddListener(TFunctor&& functor){

			ListenerKey handle = ListenerKey::MakeUnique();

			listeners_.push_back(std::make_pair(handle, std::forward<TFunctor>(functor)));

			return handle;

		}

		/// \brief Remove a listener object from this observable.
		/// \param handle Handle associated to the listener object.
		void RemoveListener(ListenerKey & key){

			listeners_.erase(std::remove_if(listeners_.begin(),
											listeners_.end(),
											[&handle](const Listener & listener){

												return listener.first == key;

											}),
							 listeners_.end());

		}

	protected:

		/// \brief Type of the listener used by this observable.
		using Listener = std::pair < ListenerKey, std::function<void(TArguments...)> >;

		/// \brief Notify all the listener objects.
		/// \param subject Subject who have requested the notification.
		/// \param args Arguments that will be delivered to each listener object.
		void Notify(TArguments&&... args){

			for (auto it : listeners_){

				it.second(std::forward<TArguments>(args)...);

			}

		}

		std::vector<Listener> listeners_;

	};

	/// \brief Observable event.
	/// \tparam TArguments Type of arguments this observable delivers to the listeners during notification.
	/// \author Raffaele D. Facendola
	template <typename... TArguments>
	class Event: public Observable<TArguments...>{

	public:

		/// \brief Notify all the listener objects.
		/// \param subject Subject who have requested the notification.
		/// \param args Arguments that will be delivered to each listener object.
		void Notify(TArguments&&... args){

			::Observable<TArguments...>::Notify(std::forward<TArguments>(args)...);

		}

	};


}
