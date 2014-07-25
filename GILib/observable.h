#pragma once

#include <memory>
#include <functional>

using std::shared_ptr;
using std::function;

///Interface implemented by observable objects
template <class... TArguments>
class Observable{

public:

	typedef function<void(TArguments...)> TFunctor;

	typedef shared_ptr<TFunctor> TListener;

	///Add a listener to this object
	virtual void AddListener(TListener & listener) = 0;

	///Remove a listener from this object
	virtual void RemoveListener(TListener & listener) = 0;

	template <class Functor>
	static TListener MakeListener(Functor functor) {

		return TListener(new TFunctor(functor));

	}

};