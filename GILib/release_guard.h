#pragma once

#include <d3d11.h>

///RAII release guard
template <class TType>
class ReleaseGuard{

public:

	ReleaseGuard(TType *& object){

		this->object = &object;

	}

	~ReleaseGuard(){

		if (object != nullptr){

			(*object)->Release();
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