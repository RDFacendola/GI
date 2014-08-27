#include "scene/scene.h"

using namespace gi_lib;

void SceneObject::Update(const Time & time){

	for (auto & it : components_){

		if (it.second->IsEnabled()){

			it.second->Update(time);

		}

	}

}