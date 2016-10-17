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

        if (direction.squaredNorm() > 0) {

            direction.normalize();

        }

        return direction;

    }

}

FlyCameraComponent::FlyCameraComponent(const IInput& input)
    : input_(input)
    , speed_(Vector3f::Zero())
    , rotation_speed_(Vector2f::Zero()){}

FlyCameraComponent::~FlyCameraComponent(){}

void FlyCameraComponent::Update(const Time& time){

    static const float kMaxSpeed = 750.0f;

    // Camera strafe
    
    auto delta_time = time.GetDeltaSeconds();

    Vector3f target_speed = GetStrafeDirection(input_.GetKeyboardStatus(),
                                               *camera_transform_) * kMaxSpeed;

    speed_ = Math::Lerp(speed_, target_speed, delta_time * 5.0f);

    Vector3f translation = camera_transform_->GetTranslation().vector() + speed_ * delta_time;

    camera_transform_->SetTranslation(Translation3f(translation));

    // Camera rotation

    auto& mb = input_.GetMouseStatus();

    Vector2i movement = mb.IsDown(ButtonCode::RIGHT_BUTTON) ? mb.GetMovement() : Vector2i::Zero();

    rotation_speed_ = Math::Lerp(rotation_speed_, Vector2f(movement(0), movement(1)), delta_time * 25.0f);

    auto up = Vector3f(0.0f, 1.0f, 0.0f);

    auto right = camera_transform_->GetRight();

    auto vrotation = Quaternionf(AngleAxisf(rotation_speed_(1) * delta_time, right));

    auto hrotation = Quaternionf(AngleAxisf(rotation_speed_(0) * delta_time, up));

    camera_transform_->SetRotation(hrotation * vrotation * camera_transform_->GetRotation());

    // Camera FOV

//     camera_component_->SetFieldOfView(Math::Lerp(Math::DegToRad(90.0f), 
//                                                  Math::DegToRad(92.5f),
//                                                  speed_.norm() / kMaxSpeed));

}

FlyCameraComponent::TypeSet FlyCameraComponent::GetTypes() const{

    auto types = Component::GetTypes();

    types.insert(type_index(typeid(FlyCameraComponent)));

    return types;

}

void FlyCameraComponent::Initialize(){

    camera_transform_ = GetComponent<TransformComponent>();
    camera_component_ = GetComponent<CameraComponent>();

}

void FlyCameraComponent::Finalize(){

    camera_transform_ = nullptr;
    camera_component_ = nullptr;

}
