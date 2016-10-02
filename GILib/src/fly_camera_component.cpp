#include "fly_camera_component.h"

#include "scene.h"
#include "gimath.h"

using namespace std;
using namespace gi_lib;

namespace{

    /// \brief Get the strafe direction based on the WASD keys.
    /// \param keyboard Keyboard where the keys will be read from.
    /// \param camera_transform Used to determine the direction in local space.
    /// \return Returns the 
    Vector3f GetStrafeDirection(const IKeyboard& keyboard, const TransformComponent& camera_transform){
    
        Vector3f direction = Vector3f::Zero();

        if (keyboard.IsDown(KeyCode::KEY_W)){

            direction += camera_transform.GetForward();     // Forward

        }

        if (keyboard.IsDown(KeyCode::KEY_S)){

            direction -= camera_transform.GetForward();     // Backward

        }

        if (keyboard.IsDown(KeyCode::KEY_D)){

            direction += camera_transform.GetRight();       // Right

        }

        if (keyboard.IsDown(KeyCode::KEY_A)){

            direction -= camera_transform.GetRight();       // Left

        }

        if (keyboard.IsDown(KeyCode::KEY_E)) {

            direction += Vector3f(0.0f, 1.0f, 0.0f);        // Up (world)

        }

        if (keyboard.IsDown(KeyCode::KEY_Q)) {

            direction -= Vector3f(0.0f, 1.0f, 0.0f);        // Down (world)

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

    // Camera rotation

    auto& mb = input_.GetMouseStatus();

    if (mb.IsDown(ButtonCode::RIGHT_BUTTON)){

        auto movement = mb.GetMovement();

        auto speed = Vector2f(movement(0) * time.GetDeltaSeconds(),
                              movement(1) * time.GetDeltaSeconds());

        auto up = Vector3f(0.0f, 1.0f, 0.0f);

        auto right = camera_transform_->GetRight();

        auto vrotation = Quaternionf(AngleAxisf(speed(1), right));

        auto hrotation = Quaternionf(AngleAxisf(speed(0) * 3.14159f, up));

        camera_transform_->SetRotation(hrotation * vrotation * camera_transform_->GetRotation());

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
