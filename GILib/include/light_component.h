/// \file light_component.h
///
/// \brief Components used to describe lights.
///
/// \author Raffaele D. Facendola

#pragma once

#include "component.h"

#include "scene.h"
#include "graphics.h"
#include "gimath.h"

namespace gi_lib {

	/// \brief Base class for each light.
	/// \author Raffaele D. Facendola
	class BaseLightComponent : public VolumeComponent {

	public:

		/// \brief Create a new white light component.
		BaseLightComponent();

		/// \brief Create a new light component.
		/// \param color The light's color.
		BaseLightComponent(const Color& color);

		virtual TypeSet GetTypes() const override;

		/// \brief Get the light's color.
		Color GetColor() const;

		/// \brief Set the light's color.
		/// \param color The light's color.
		void SetColor(const Color& color);

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

		/// \brief Access the transform component of the object.
		const TransformComponent& GetTransformComponent() const;
		
		/// \brief Compute the light bounds.
		/// \param notify Whether the VolumeComponent::OnChanged event should be triggered
		void ComputeBounds(bool notify);

	private:
		
		/// \brief Compute the light bounds.
		virtual void ComputeBounds() = 0;

		Color color_;											///< \brief The light color.

		TransformComponent* transform_;							///< \brief Pointer to the transform component for faster access.

		unique_ptr<Listener> on_transform_changed_lister_;		///< \brief Listener for the transform changed event.

	};

	/// \brief Represents a single point light.
	/// A point light is a light that has a position and irradiates light in all directions.
	/// The position of the light is defined by a separate transform component.
	/// The attenuation of the point lights is defined as Att(d) = (Kc + Kl*d + Kq*d*d)^-1 where
	/// Kc is a constant factor
	/// Kl is a linear factor
	/// Kq is a quadratic factor
	/// d is the distance of the surface from the light.
	/// \author Raffaele D. Facendola.
	class PointLightComponent : public BaseLightComponent {

	public:

		/// \brief Create a new point light component.
		/// \param color The light's color.
		/// \param radius Radius of the light sphere.
		PointLightComponent(const Color& color, float radius);

		/// \brief Create a new point light component.
		/// \param color The light's color
		/// \param constant_factor Constant attenuation factor.
		/// \param linear_factor Linear attenuation factor.
		/// \param quadratic_factor Quadratic attenuation factor.
		PointLightComponent(const Color& color, float constant_factor, float linear_factor, float quadratic_factor);

		virtual TypeSet GetTypes() const override;

		virtual IntersectionType TestAgainst(const Frustum& frustum) const override;

		virtual IntersectionType TestAgainst(const AABB& box) const override;

		virtual IntersectionType TestAgainst(const Sphere& sphere) const override;

		/// \brief Get the constant factor Kc of the point light.
		float GetConstantFactor() const;

		/// \brief Set the constant factor Kc of the point light.
		/// \param constant_factor The new constant factor.
		void SetConstantFactor(float constant_factor);

		/// \brief Get the linear factor Kl of the point light.
		float GetLinearFactor() const;

		/// \brief Set the linear factor Kl of the point light.
		/// \param linear_factor The new linear factor.
		void SetLinearFactor(float linear_factor);

		/// \brief Get the quadratic factor Kq of the point light.
		float GetQuadraticFactor() const;

		/// \brief Set the quadratic factor Kq of the point light.
		/// \param quadratic The new quadratic factor.
		void SetQuadraticFactor(float quadratic_factor);

		/// \brief Set the virtual point light's radius.
		/// This method will affect both the constant, the linear and the quadratic factor in order to approximate a point light with an actual radius greater than 0.
		/// \param radius Radius of the sphere.
		void SetRadius(float radius);

		/// \brief Get the light's position.
		/// \return Returns the light's position.
		Vector3f GetPosition() const;

	private:

		virtual void ComputeBounds() override;

		float constant_factor_;							///< \brief The constant attenuation factor.

		float linear_factor_;							///< \brief The linear attenuation factor.

		float quadratic_factor_;						///< \brief The quadratic attenuation factor.

		Sphere bounds_;									///< \brief Bounds of the light.

	};

	/// \brief Represents a single directional light.
	/// A directional light is a light that has no position and irradiates light along one direction.
	/// The direction of the light is defined by the forward direction of the object given by a transform component.
	/// The light has no attenuation.
	/// \author Raffaele D. Facendola.
	class DirectionalLightComponent : public BaseLightComponent {

	public:

		/// \brief Initializes a directional light component using default values for the color.
		DirectionalLightComponent(const Color& color);
		
		virtual IntersectionType TestAgainst(const Frustum& frustum) const override;

		virtual IntersectionType TestAgainst(const AABB& box) const override;

		virtual IntersectionType TestAgainst(const Sphere& sphere) const override;

		virtual TypeSet GetTypes() const override;

		/// \brief Get the light's direction.
		/// \return Returns the light's direction.
		Vector3f GetDirection() const;

	private:

		virtual void ComputeBounds() override;

	};

	/// \brief Represents a single spot light.
	/// A spotlight is a light that irradiates from one point towards a direction spreading with a fixed angle.
	/// The direction and position of the light are defined by the forward direction and position of the object given by a transform component.	
	/// The attenuation of the point lights is defined as Att(d) = (Kc + Kl*d + Kq*d*d)^-1 where
	/// Kc is a constant factor
	/// Kl is a linear factor
	/// Kq is a quadratic factor
	/// d is the distance of the surface from the light.
	/// The point light also defines two angles used to determine the angle of the light cone and penumbra cone and a falloff factor used to determine how sharp the light's edge is.
	/// The falloff attenuation is defined as Falloff(rho) = saturate((rho - cos(phi*0.5)) / (cos(theta*0.5) - cos(phi*0.5)) ^ f where
	/// rho is the cosine of the angle between light's direction and the surface.
	/// phi is the penumbra angle.
	/// theta is the light angle.
	/// \author Raffaele D. Facendola.
	class SpotLightComponent : public BaseLightComponent {

	public:

		/// \brief Create a new default spotlight component.
		SpotLightComponent();

		/// \brief Initializes a directional light component using default values for the color.
		SpotLightComponent(const Color& color, float light_angle, float penumbra_angle, float falloff, float constant_factor, float linear_factor, float quadratic_factor);

		virtual TypeSet GetTypes() const override;

		virtual IntersectionType TestAgainst(const Frustum& frustum) const override;

		virtual IntersectionType TestAgainst(const AABB& box) const override;

		virtual IntersectionType TestAgainst(const Sphere& sphere) const override;

		/// \brief Get the light cone angle in radians.
		float GetLightConeAngle() const;

		/// \brief Set the light cone angle.
		/// \param light_cone_angle The new light cone angle in radians.
		void SetLightConeAngle(float light_angle);

		/// \brief Get the penumbra cone angle in radians.
		float GetPenumbraConeAngle() const;

		/// \brief Set the penumbra cone angle.
		/// \param penumbra_cone_angle The new penumbra cone angle in radians.
		void SetPenumbraConeAngle(float penumbra_angle);

		/// \brief Get the falloff factor.
		float GetFalloff() const;

		/// \brief Set the falloff factor.
		/// \param falloff The new falloff value.
		void SetFalloff(float falloff);

		/// \brief Get the constant factor Kc of the point light.
		float GetConstantFactor() const;

		/// \brief Set the constant factor Kc of the point light.
		/// \param constant_factor The new constant factor.
		void SetConstantFactor(float constant_factor);

		/// \brief Get the linear factor Kl of the point light.
		float GetLinearFactor() const;

		/// \brief Set the linear factor Kl of the point light.
		/// \param linear_factor The new linear factor.
		void SetLinearFactor(float linear_factor);

		/// \brief Get the quadratic factor Kq of the point light.
		float GetQuadraticFactor() const;

		/// \brief Set the quadratic factor Kq of the point light.
		/// \param quadratic The new quadratic factor.
		void SetQuadraticFactor(float quadratic_factor);

		/// \brief Get the light's position.
		/// \return Returns the light's position.
		Vector3f GetPosition() const;

		/// \brief Get the light's direction.
		/// \return Returns the light's direction.
		Vector3f GetDirection() const;

	private:

		virtual void ComputeBounds() override;

		float light_angle_;									///< \brief Maximum angle at which a surface point is inside the light cone.

		float penumbra_angle_;								///< \brief Maximum angle at which a surface point is inside the penumbra cone.

		float falloff_;										///< \brief Exponential factor used to determine the sharpness of the light's cone.

		float constant_factor_;								///< \brief The constant attenuation factor.

		float linear_factor_;								///< \brief The linear attenuation factor.

		float quadratic_factor_;							///< \brief The quadratic attenuation factor.

	};

	//////////////////////////// BASE LIGHT COMPONENT ///////////////////////////////

	inline BaseLightComponent::BaseLightComponent() :
		BaseLightComponent(kOpaqueWhite) {}

	inline BaseLightComponent::BaseLightComponent(const Color& color) : color_(color) {}

	inline Color BaseLightComponent::GetColor() const {

		return color_;

	}

	inline void BaseLightComponent::SetColor(const Color& color) {

		color_ = color;

	}

	/// \brief Access the transform component of the object.
	inline const TransformComponent& BaseLightComponent::GetTransformComponent() const {

		return *transform_;

	}

	/////////////////////////// POINT LIGHT COMPONENT ///////////////////////////////

	inline PointLightComponent::PointLightComponent(const Color& color, float constant_factor, float linear_factor, float quadratic_factor) :
		BaseLightComponent(color),
		constant_factor_(constant_factor),
		linear_factor_(linear_factor),
		quadratic_factor_(quadratic_factor) {}

	inline IntersectionType PointLightComponent::TestAgainst(const Frustum& frustum) const {

		return frustum.Intersect(bounds_);

	}

	inline IntersectionType PointLightComponent::TestAgainst(const AABB& box) const {

		return bounds_.Intersect(box);

	}

	inline IntersectionType PointLightComponent::TestAgainst(const Sphere& sphere) const {

		return bounds_.Intersect(sphere);

	}
	
	inline float PointLightComponent::GetConstantFactor() const {

		return constant_factor_;

	}

	inline void PointLightComponent::SetConstantFactor(float constant_factor) {

		constant_factor_ = constant_factor;
		
		BaseLightComponent::ComputeBounds(true);

	}

	inline float PointLightComponent::GetLinearFactor() const {

		return linear_factor_;

	}

	inline void PointLightComponent::SetLinearFactor(float linear_factor) {

		linear_factor_ = linear_factor;

		BaseLightComponent::ComputeBounds(true);

	}

	inline float PointLightComponent::GetQuadraticFactor() const {

		return quadratic_factor_;

	}

	inline void PointLightComponent::SetQuadraticFactor(float quadratic_factor) {

		quadratic_factor_ = quadratic_factor;
		
		BaseLightComponent::ComputeBounds(true);

	}

	inline Vector3f PointLightComponent::GetPosition() const {

		return Math::ToVector3(GetTransformComponent().GetWorldTransform().matrix().col(3));

	}

	/////////////////////////// DIRECTIONAL LIGHT COMPONENT ///////////////////////////////

	inline DirectionalLightComponent::DirectionalLightComponent(const Color& color) :
		BaseLightComponent(color) {}

	inline IntersectionType DirectionalLightComponent::TestAgainst(const Frustum&) const{

		// A directional light is infinite in size and range
		return IntersectionType::kIntersect;

	}

	inline IntersectionType DirectionalLightComponent::TestAgainst(const AABB&) const {
		
		// A directional light is infinite in size and range
		return IntersectionType::kIntersect;

	}

	inline IntersectionType DirectionalLightComponent::TestAgainst(const Sphere&) const {

		// A directional light is infinite in size and range
		return IntersectionType::kIntersect;

	}

	inline Vector3f DirectionalLightComponent::GetDirection() const {

		return GetTransformComponent().GetForward();

	}

	inline void DirectionalLightComponent::ComputeBounds() {

		// Do nothing, whatever

	}

	/////////////////////////// SPOT LIGHT COMPONENT //////////////////////////////////////

	inline SpotLightComponent::SpotLightComponent() :
		SpotLightComponent(kOpaqueWhite, Math::kPi / 3, Math::kPi / 2, 1.0f, 1.0f, 0.0f, 0.0f) {}

	inline SpotLightComponent::SpotLightComponent(const Color& color, float light_angle, float penumbra_angle, float falloff, float constant_factor, float linear_factor, float quadratic_factor) :
		BaseLightComponent(color),
		light_angle_(light_angle),
		penumbra_angle_(penumbra_angle),
		falloff_(falloff),
		constant_factor_(constant_factor),
		linear_factor_(linear_factor),
		quadratic_factor_(quadratic_factor) {}
		
	inline float SpotLightComponent::GetLightConeAngle() const {

		return light_angle_;

	}

	inline void SpotLightComponent::SetLightConeAngle(float light_angle) {

		light_angle_ = light_angle;

		BaseLightComponent::ComputeBounds(true);

	}

	inline float SpotLightComponent::GetPenumbraConeAngle() const {

		return penumbra_angle_;

	}
	
	inline void SpotLightComponent::SetPenumbraConeAngle(float penumbra_angle) {

		penumbra_angle_ = penumbra_angle;

		//BaseLightComponent::ComputeBounds(true);		Not needed

	}

	inline float SpotLightComponent::GetFalloff() const {

		return falloff_;

	}

	inline void SpotLightComponent::SetFalloff(float falloff) {

		falloff_ = falloff;

		// BaseLightComponent::ComputeBounds(true);		// Not needed

	}
	
	inline float SpotLightComponent::GetConstantFactor() const {

		return constant_factor_;

	}

	inline void SpotLightComponent::SetConstantFactor(float constant_factor) {

		constant_factor_ = constant_factor;

		BaseLightComponent::ComputeBounds(true);

	}

	inline float SpotLightComponent::GetLinearFactor() const {

		return linear_factor_;

	}

	inline void SpotLightComponent::SetLinearFactor(float linear_factor) {

		linear_factor_ = linear_factor;

		BaseLightComponent::ComputeBounds(true);

	}

	inline float SpotLightComponent::GetQuadraticFactor() const {

		return quadratic_factor_;

	}

	inline void SpotLightComponent::SetQuadraticFactor(float quadratic_factor) {

		quadratic_factor_ = quadratic_factor;

		BaseLightComponent::ComputeBounds(true);

	}

	inline Vector3f SpotLightComponent::GetPosition() const {

		return Math::ToVector3(GetTransformComponent().GetWorldTransform().matrix().col(3));

	}

	inline Vector3f SpotLightComponent::GetDirection() const {

		return Math::ToVector3(GetTransformComponent().GetWorldTransform().matrix().col(2));

	}

}