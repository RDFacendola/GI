/// \file observable.h
/// \brief Classes used to implement and manage the observer pattern and its variations.
///
/// \author Raffaele D. Facendola

#pragma once

#include <set>
#include <functional>

using std::set;
using std::function;

namespace gi_lib{

	/// \brief Interface that should be inherited by any observable object.
	/// \tparam TArguments Types of the arguments passed to the observers when the observable object notifies them.
	template <typename... TArguments>
	class Observable{

	public:

		/// \brief Type of the observers.
		typedef function<void(TArguments...)> TListener;

		/// \brief Add a new observer to this instace.
		/// \param listener Reference to the observer instace that should be notified.
		/// \return Returns a reference to this instance.
		virtual Observable & operator<<(TListener & listener) = 0;

		/// \brief Remove a new observer from this instace.
		/// \param listener Reference to the observer instace that should be removed.
		/// \return Returns a reference to this instance.
		virtual Observable & operator>>(TListener & listener) = 0;

	};

	/// \brief Observable event with the ability to notify all its observers.
	/// \tparam TArguments Types of the arguments passed to the observers when the observable object notifies them.
	template <typename... TArguments>
	class Event : public Observable<TArguments...>
	{

	public:

		inline virtual Observable & operator<<(TListener & listener){

			listeners_.insert(&listener);

			return *this;

		}

		inline virtual Observable & operator>>(TListener & listener){

			listeners_.erase(&listener);

			return *this;

		}

		/// \brief Notify all the observers.
		/// \param arguments List of the arguments that will be passed to the observers.
		inline void Notify(TArguments... arguments){

			for (auto & listener : listeners_){

				(*listener)(arguments...);

			}

		}

	private:

		set<TListener *> listeners_;

	};

}
