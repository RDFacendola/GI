/// \file fly_camera_component.h
///
/// \author Raffaele D. Facendola

#pragma once

#include "component.h"
#include "timer.h"
#include "input.h"

namespace gi_lib{

	class TransformComponent;

	/// \brief Component used to move an aerial camera.
	/// \author Raffaele D. Facendola
	class FlyCameraComponent : public Component{

	public:

		FlyCameraComponent(const IInput& input);

		/// \brief No copy constructor.
		FlyCameraComponent(const FlyCameraComponent&) = delete;

		/// \brief Destructor.
		virtual ~FlyCameraComponent();

		/// \brief Update the component.
		/// \param time Application time.
		void Update(const Time& time);

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		const IInput& input_;						///< \brief Input reader.

		TransformComponent* camera_transform_;		///< \brief Transform component associated to the camera.

	};


}