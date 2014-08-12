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

	/// \brief Handle associated to each listener.
	using ListenerHandle = Unique<ListenerTag>;

	/// \brief Observable event.
	/// \tparam TSubject Concrete type of the subject this observable refers to.
	/// \tparam TArguments Type of arguments this observable delivers to the listeners during notification.
	/// \author Raffaele D. Facendola
	template <typename TSubject, typename... TArguments>
	class Observable{

		/// The subject this observable refers to is the only one who should be able to notify the observers
		friend typename TSubject;

	public:

		/// \brief Type of the listener used by this observable.
		using Listener = std::pair < ListenerHandle, std::function<void(TSubject &, TArguments...)> >;

		/// \brief Add a new listener to this observable.
		/// \tparam TFunctor Type of listener object.
		/// \param functor Actual listener object.
		/// \return Returns an handle that associate the listener object to this observable.
		template<typename TFunctor>
		ListenerHandle AddListener(TFunctor&& functor){

			ListenerHandle handle;

			listeners_.push_back(std::make_pair(handle, std::forward<TFunctor>(functor)));

			return handle;

		}

		/// \brief Remove a listener object from this observable.
		/// \param handle Handle associated to the listener object.
		void RemoveListener(ListenerHandle & handle){

			listeners_.erase(std::remove_if(listeners_.begin(),
											listeners_.end(),
											[&handle](const Listener & ref){

												return ref.first == handle;

											}),
							 listeners_.end());

		}

	protected:

		/// \brief Notify all the listener objects.
		/// \param subject Subject who have requested the notification.
		/// \param args Arguments that will be delivered to each listener object.
		void Notify(TSubject & subject, TArguments&&... args){

			for (auto it : listeners_){

				it.second(subject, std::forward<TArguments>(args)...);

			}

		}

		std::vector<Listener> listeners_;

	};

}
