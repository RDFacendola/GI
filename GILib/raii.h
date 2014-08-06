#pragma once

///Guard that executes a functor upon destruction if not dismissed beforehand
class ScopeGuard{

public:

	template <class TFunctor>
	ScopeGuard(TFunctor functor) :
		functor_(functor),
		dismissed_(false){}

	ScopeGuard(ScopeGuard & other) :
		functor_(other.functor_),
		dismissed_(other.dismissed_){

		other.Dismiss();

	}

	~ScopeGuard(){

		if (!dismissed_){

			///Call the functor if not dismissed
			functor_();

		}

	}

	void Dismiss(){

		dismissed_ = true;

	}

private:

	///Called upon destruction, if not dismissed
	std::function<void(void)> functor_;

	bool dismissed_;

};