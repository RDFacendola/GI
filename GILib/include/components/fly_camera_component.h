/// \file fly_camera_component.h
///
/// \author Raffaele D. Facendola

#pragma once

#include "..\component.h"

namespace gi_lib{

	class TransformComponent;

	/// \brief Component used to move an aerial camera.
	/// \author Raffaele D. Facendola
	class FlyCameraComponent : public Component{

	public:

		/// \brief No copy constructor.
		FlyCameraComponent(const FlyCameraComponent&) = delete;

		/// \brief Destructor.
		virtual ~FlyCameraComponent();

		/// \brief Get the scene this node is associated to.
		/// \return Returns the scene this node is associated to.
		Scene& GetScene();

		/// \brief Get the scene this node is associated to.
		/// \return Returns the scene this node is associated to.
		const Scene& GetScene() const;

		/// \brief Get the node name.
		/// The name may not be univocal.
		/// \return Returns the node name.
		const wstring& GetName() const;

		/// \brief Get the node unique identifier.
		/// \return Returns the node unique identifier.
		const Unique<NodeComponent> GetUid() const;

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		TransformComponent* transform_;

	};


}