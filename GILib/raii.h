#pragma once

///RAII - Acquire an object and delete it upon destruction
template <class TType>
class DeleteGuard{

public:

	///Acquire the object
	DeleteGuard(TType *& object){

		this->object = &object;

	}

	///Delete the object
	~DeleteGuard(){

		if (object != nullptr){

			delete (*object);
			(*object) = nullptr;

		}

	}

	///Free the object without deleting it
	void Free(){

		object = nullptr;

	}

private:

	TType ** object;

};

///RAII - Acquire an object and release it upon destruction
template <class TType>
class ReleaseGuard{

public:

	///Acquire the object
	ReleaseGuard(TType *& object){

		this->object = &object;

	}

	///Release the object
	~ReleaseGuard(){

		if (object != nullptr &&
			(*object) != nullptr){

			(*object)->Release();
			(*object) = nullptr;

		}

	}

	//Free the object without releasing it
	void Free(){
				
		object = nullptr;

	}

private:

	TType ** object;

};