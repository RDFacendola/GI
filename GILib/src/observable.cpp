#include "observable.h"

using namespace std;
using namespace gi_lib;

///////////////////////////////////// LISTENER ///////////////////////////////////////////

Listener::Listener() :
id_(Unique<ListenerTag>::kNull),
subject_(nullptr){}

Listener::Listener(BaseObservable* observable, Unique<ListenerTag> id) :
id_(id),
subject_(observable){}

Listener::~Listener(){

	Unsubscribe();

}

Unique<ListenerTag> Listener::GetId(){

	return id_;

}

void Listener::Unsubscribe(){

	if (subject_){

		subject_->Unsubscribe(*this);

		Invalidate();

	}
	
}

void Listener::Invalidate(){

	id_ = Unique<ListenerTag>::kNull;
	subject_ = nullptr;

}

///////////////////////////////////// BASE OBSERVABLE ///////////////////////////////////////////

BaseObservable::BaseObservable(){}

BaseObservable::~BaseObservable(){}

void BaseObservable::Invalidate(Listener& listener){

	listener.Invalidate();

}

unique_ptr<Listener> BaseObservable::GenerateListener(){

	return make_unique<Listener>(this, Unique < ListenerTag >::MakeUnique());

}

void BaseObservable::Unsubscribe(Listener& listener){

	Unsubscribe(listener.id_);

	listener.Invalidate();

}

////////////////////// EVENT ARGS ///////////////////////////

const EventArgs EventArgs::kEmpty = EventArgs();