#pragma once

#include <set>
#include <algorithm>

#include "observable.h"

using std::set;

///An event that can be observed and notify all its listeners
template <class... TArguments>
class Event : public Observable<TArguments...>
{

public:

	///Add a listener to this object
	__forceinline virtual void AddListener(TListener & listener){

		listeners_.insert(&listener);

	}

	///Remove a listener from this object
	__forceinline virtual void RemoveListener(TListener & listener){

		listeners_.erase(std::remove(std::begin(listeners_), std::end(listeners_), &listener), std::end(listeners_));

	}

	///Notify all the listeners
	__forceinline void Notify(TArguments... arguments){

		for (auto & listener : listeners_){

			(**listener)(arguments...);

		}
		
	}

private:

	///List of the listeners
	set<TListener *> listeners_;

};