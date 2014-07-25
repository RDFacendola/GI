#pragma once

#include <set>
#include <functional>

using std::set;
using std::function;

///Interface implemented by observable objects
template <class... TArguments>
class Observable{

public:
	
	typedef function<void(TArguments...)> TListener;

	///Add a listener to this object
	virtual Observable & operator<<(TListener & listener) = 0;

	///Remove a listener from this object
	virtual Observable & operator>>(TListener & listener) = 0;

};

///An event that can be observed and notify all its listeners
template <class... TArguments>
class Event : public Observable<TArguments...>
{

public:
	
	///Add a listener to this object
	inline virtual Observable & operator<<(TListener & listener){

		listeners_.insert(&listener);

		return *this;

	}

	///Remove a listener from this object
	inline virtual Observable & operator>>(TListener & listener){
	
		listeners_.erase(&listener);

		return *this;

	}

	///Notify all the listeners
	inline void Notify(TArguments... arguments){

		for (auto & listener : listeners_){

			(*listener)(arguments...);

		}
		
	}

private:

	///List of the listeners
	set<TListener *> listeners_;

};