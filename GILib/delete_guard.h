#pragma once

///RAII delete guard
template <class TType>
class DeleteGuard{

public:

	DeleteGuard(TType *& object){

		this->object = &object;

	}

	~DeleteGuard(){

		if (object != nullptr){

			delete (*object);
			(*object) = nullptr;

		}

	}

	void Free(){

		//Frees the object without releasing it
		object = nullptr;

	}

private:

	TType ** object;

};