#include "..\..\include\components\fly_camera_component.h"

#include "..\..\include\scene.h"

#include "..\..\include\gimath.h"

using namespace std;
using namespace gi_lib;

namespace{

	/// \brief Get the strafe direction based on the WASD keys.
	/// \param keyboard Keyboard where the keys will be read from.
	/// \param camera_transform Used to determine the direction in local space.
	/// \return Returns the 
	Vector3f GetStrafeDirection(const IKeyboard& keyboard, const TransformComponent& camera_transform){
	
		Vector3f direction = Vector3f::Zero();

		if (keyboard.IsDown(17)){

			direction += camera_transform.GetForward();		// Up

		}

		if (keyboard.IsDown(31)){

			direction -= camera_transform.GetForward();		// Down

		}

		if (keyboard.IsDown(32)){

			direction += camera_transform.GetRight();		// Right

		}

		if (keyboard.IsDown(30)){

			direction -= camera_transform.GetRight();		// Left

		}

		return direction;

	}

}

FlyCameraComponent::FlyCameraComponent(const IInput& input) :
input_(input){}

FlyCameraComponent::~FlyCameraComponent(){}

void FlyCameraComponent::Update(const Time& time){

	// Camera strafe

	auto strafe_direction = GetStrafeDirection(input_.GetKeyboardStatus(),
											   *camera_transform_);

	if (strafe_direction.squaredNorm() > 0){

		strafe_direction.normalize();

		Vector3f translation = camera_transform_->GetTranslation().vector();

		translation += strafe_direction * time.GetDeltaSeconds() * 500.0f;

		camera_transform_->SetTranslation(Translation3f(translation));

	}

	// Rotation

	auto& mb = input_.GetMouseStatus();

	if (mb.IsDown(0)){

		auto movement = mb.GetMovement();

		auto speed = Vector2f(movement(0) * time.GetDeltaSeconds(),
							  movement(1) * time.GetDeltaSeconds());

		auto up = Vector3f(0.0f, 1.0f, 0.0f);

		auto hrotation = Quaternionf(AngleAxisf(speed(0) * 3.14159f, up));

		camera_transform_->SetRotation(hrotation * camera_transform_->GetRotation());

		auto right = camera_transform_->GetRight();

		auto vrotation = Quaternionf(AngleAxisf(speed(1), right));

		camera_transform_->SetRotation(vrotation * camera_transform_->GetRotation());

	}

}

FlyCameraComponent::TypeSet FlyCameraComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(FlyCameraComponent)));

	return types;

}

void FlyCameraComponent::Initialize(){

	camera_transform_ = GetComponent<TransformComponent>();

}

void FlyCameraComponent::Finalize(){

	camera_transform_ = nullptr;

}
