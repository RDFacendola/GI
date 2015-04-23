/// \file volume_hierarchy.h
/// \brief Base file for bounding volume hierarchies.
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "..\component.h"

using std::vector;

namespace gi_lib{

	class Frustum;

	class VolumeComponent;
	class VolumeHierarchyComponent;
	
	/// \brief Represents a volume hierarchy.
	/// \author Raffaele D. Facendola
	class VolumeHierarchyComponent : public Component{

	public:
		
		/// \brief Precision of the intersection test.
		enum class PrecisionLevel{

			Coarse,				///< \brief A coarse test may produce false positives.
			Medium,				///< \brief A medium-grained test that could produce some false positive.
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

}