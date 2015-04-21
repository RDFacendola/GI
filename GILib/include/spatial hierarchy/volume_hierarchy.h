/// \file volume_hierarchy.h
/// \brief Base file for bounding volume hierarchies.
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include <functional>

#include "..\gimath.h"
#include "..\component.h"
#include "..\observable.h"
#include "..\scene.h"

using std::vector;
using std::function;

namespace gi_lib{

	class VolumeHierarchyComponent;
	class VolumeComponent;

	/// \brief Represents a volume hierarchy.
	/// \author Raffaele D. Facendola
	class VolumeHierarchyComponent : public Component{

	public:
		
		/// \brief Precision of the intersection test.
		enum class PrecisionLevel{

			Coarse,				///< \brief A coarse test may produce false positives.
			Fine,				///< \brief A fine-grained test never produce false positives.
			
		};

		/// \brief Virtual destructor.
		virtual ~VolumeHierarchyComponent();

		/// \brief Add a new volume to the hierarchy.
		/// \param volume The volume to add to the hierarchy.
		virtual void AddVolume(VolumeComponent* volume) = 0;

		/// \brief Remove an existing volume from the hierarchy.
		/// \param volume The volume to remove from the hierarchy.
		virtual void RemoveVolume(VolumeComponent* volume) = 0;

		/// \brief Get all the volume component who intersects with the given frustum.
		/// \param frustum Frustum.
		/// \param precision Precision level of the test.
		/// \remarks Coarse tests are faster but may lead to false positives.
		///          Fine tests, however, achieve lesser performances and produce no false positive.
		virtual vector<VolumeComponent*> GetIntersections(const Frustum& frustum, PrecisionLevel precision) const = 0;

		virtual TypeSet GetTypes() const override;

	};

	/// \brief Contains informations about an axis-aligned bounding box surrounding the node.
	/// \author Raffaele D. Facendola
	class VolumeComponent : public Component{

	public:
		
		/// \brief Arguments relative to OnBoundsChanged event.
		struct OnBoundsChangedEventArgs{

			VolumeComponent* volume;	///< \brief Volume whose bounds have changed.

		};

		/// \brief Create a new volume component.
		VolumeComponent();

		/// \brief Create a new volume component.
		/// \param bounds The initial bounds of the component.
		VolumeComponent(const AABB& bounds);

		/// \brief Virtual destructor.
		virtual ~VolumeComponent();

		/// \brief Get the transformed bounding box.
		/// \return Returns the transformed bouding box.
		const AABB& GetBoundingBox() const;

		/// \brief Get the transformed bounding sphere with squared radius.
		/// \return Returns the transformed bounding sphere with squared radius.
		const Sphere& GetBoundingSphereSquared() const;

		/// \brief Event that is signaled whenever the bounds change.
		/// \return Returns the event that is signaled whenever the bounds change.
		Observable<OnBoundsChangedEventArgs>& OnBoundsChanged();

		virtual TypeSet GetTypes() const override;

	protected:

		virtual void Initialize() override;

		virtual void Finalize() override;

		/// \brief Set new bounds for this component.
		/// \param bounds New bounds.
		void SetBoundingBox(const AABB& bounds);

	private:

		void SetDirty();										///< \brief Mark this component as dirty and notify everybody.

		AABB bounding_box_;										///< \brief Bounding box.

		TransformComponent* transform_;							///< \brief Transform component needed to computed the transformed bounds.

		VolumeHierarchyComponent* hierarchy_;					///< \brief Volume hierarchy component needed to fast volume rejection.

		Event<OnBoundsChangedEventArgs> on_bounds_changed_;		///< \brief Event signaled whenever the bounds change.

		unique_ptr<Listener> on_transform_changed_lister_;		///< \brief Listener for the transform changed event.

		mutable AABB transformed_bounds_;						///< \brief Transformed bounds.

		mutable bool is_box_dirty_;								///< \brief Whether the bounds needs to be recalculated.

		mutable Sphere bounding_sphere_;						///< \brief Bounding sphere with squared radius. Calculated by need.

		mutable bool is_sphere_dirty_;							///< \brief Is the bounding sphere dirty?

	};

}