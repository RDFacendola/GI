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

namespace gi_lib{

	/// \brief Represents a single point light.
	/// A point light is a light that has a position and irradiates light in all directions.
	/// The position of the light is defined by a separate transform component.
	/// \author Raffaele D. Facendola.
	class PointLightComponent : public Component{

	public:

		static const Color kDefaultLightColor;				///< \brief Default light color: white.

		static const float kDefaultLinearDecay;				///< \brief Default linear decay of the light: none.

		static const float kDefaultSquareDecay;				///< \brief Default square decay of the light: 1/4pi (reverse area of a sphere).

		static const float kDefaultIntensity;				///< \brief Default intensity of the light: 1.

		/// \brief Initializes a point light component using default values for both the color and the decay.
		PointLightComponent();

		virtual TypeSet GetTypes() const override;

		/// \brief Get the light's color.
		/// \return Returns the light's color.
		Color GetColor() const;

		/// \brief Set the light's color.
		/// \param color The new light's color.
		void SetColor(Color color);

		/// \brief Get the linear decay of the light, relative to the distance between the light and the surface.
		/// \return Returns the linear decay of the light.
		float GetLinearDecay() const;

		/// \brief Set the linear decay of the light, relative to the distance between the light and the surface.
		/// \param linear_decay Linear decay of the light.
		void SetLinearDecay(float linear_decay);

		/// \brief Get the square decay of the light, relative to the squared distance between the light and the surface.
		/// \return Returns the squared decay of the light.
		float GetSquareDecay() const;

		/// \brief Set the squared decay of the light, relative to the squared distance between the light and the surface.
		/// \param squared_decay Squared decay of the light.
		void SetSquareDecay(float squared_decay);

		/// \brief Get the light intensity.
		/// \return Returns the light intensity.
		float GetIntensity() const;

		/// \brief Set the intensity of the light.
		/// \param intensity Intensity of the light
		void SetIntensity(float intensity);

		/// \brief Get the light's position.
		/// \return Returns the light's position.
		Vector3f GetPosition() const;
		
	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		Color color_;										///< \brief Light color.

		float linear_decay_;								///< \brief Linear decay of the light, relative to the distance between the light and the surface.

		float squared_decay_;								///< \brief Squared decay of the light, relative to the squared distance between the light to the surface.
			
		float intensity_;									///< \brief Intensity of the light, as multiplicative factor over the light color.

		TransformComponent* transform_component_;			///< \brief Used to determine the position of the light.

	};

	/// \brief Represents a single directional light.
	/// A directional light is a light that has no position and irradiates light along one direction.
	/// The direction of the light is defined by the forward direction of the object given by a transform component.
	/// \author Raffaele D. Facendola.
	class DirectionalLightComponent : public Component{

	public:

		static const Color kDefaultLightColor;				///< \brief Default light color: white.

		static const float kDefaultIntensity;				///< \brief Default intensity of the light: 1.

		/// \brief Initializes a directional light component using default values for the color.
		DirectionalLightComponent();

		virtual TypeSet GetTypes() const override;

		/// \brief Get the light's color.
		/// \return Returns the light's color.
		Color GetColor() const;

		/// \brief Set the light's color.
		/// \param color The new light's color.
		void SetColor(Color color);

		/// \brief Get the light intensity.
		/// \return Returns the light intensity.
		float GetIntensity() const;

		/// \brief Set the intensity of the light.
		/// \param intensity Intensity of the light
		void SetIntensity(float intensity);

		/// \brief Get the light's direction.
		/// \return Returns the light's direction.
		Vector3f GetDirection() const;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		Color color_;										///< \brief Light color.

		float intensity_;									///< \brief Intensity of the light, as multiplicative factor over the light color.

		TransformComponent* transform_component_;			///< \brief Used to determine the position of the light.

	};

	/////////////////////////// POINT LIGHT COMPONENT ///////////////////////////////

	inline PointLightComponent::PointLightComponent() :
		color_(kDefaultLightColor),
		linear_decay_(kDefaultLinearDecay),
		squared_decay_(kDefaultSquareDecay),
		intensity_(kDefaultIntensity){}

	inline Color PointLightComponent::GetColor() const{

		return color_;

	}

	inline void PointLightComponent::SetColor(Color color){

		color_ = color;

	}

	inline float PointLightComponent::GetLinearDecay() const{

		return linear_decay_;

	}

	inline void PointLightComponent::SetLinearDecay(float linear_decay){

		linear_decay_ = linear_decay;

	}

	inline float PointLightComponent::GetSquareDecay() const{

		return squared_decay_;

	}

	inline void PointLightComponent::SetSquareDecay(float squared_decay){

		squared_decay_ = squared_decay;

	}

	inline float PointLightComponent::GetIntensity() const{

		return intensity_;

	}

	inline void PointLightComponent::SetIntensity(float intensity){

		intensity_ = intensity;

	}

	inline Vector3f PointLightComponent::GetPosition() const{

		return Math::ToVector3(transform_component_->GetWorldTransform().matrix().col(3));

	}

	/////////////////////////// DIRECTIONAL LIGHT COMPONENT ///////////////////////////////

	inline DirectionalLightComponent::DirectionalLightComponent() :
		color_(kDefaultLightColor),
		intensity_(kDefaultIntensity){}

	inline Color DirectionalLightComponent::GetColor() const{

		return color_;

	}

	inline void DirectionalLightComponent::SetColor(Color color){

		color_ = color;

	}
	
	inline float DirectionalLightComponent::GetIntensity() const{

		return intensity_;

	}

	inline void DirectionalLightComponent::SetIntensity(float intensity){

		intensity_ = intensity;

	}

	inline Vector3f DirectionalLightComponent::GetDirection() const{

		return Math::ToVector3(transform_component_->GetWorldTransform().matrix().col(2));

	}


}