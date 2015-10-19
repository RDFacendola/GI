/// \file volume_hierarchy.h
/// \brief Base file for bounding volume hierarchies.
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "component.h"

using std::vector;

namespace gi_lib{

	class Frustum;

	class VolumeComponent;
	class VolumeHierarchyComponent;
	
	/// \brief Base interface for volume hierarchy.
	/// Volumes added to the hierarchy must be manually removed upon destruction of the component.
	/// The hierarchy will relocate automatically the node whenever its bounds change.
	/// \author Raffaele D. Facendola
	class IVolumeHierarchy{

	public:

		/// \brief Virtual destructor.
		virtual ~IVolumeHierarchy(){};

		/// \brief Add a new volume to the hierarchy.
		/// \param volume The volume to add to the hierarchy.
		virtual void AddVolume(VolumeComponent* volume) = 0;

		/// \brief Remove an existing volume from the hierarchy.
		/// \param volume The volume to remove from the hierarchy.
		virtual void RemoveVolume(VolumeComponent* volume) = 0;

		/// \brief Get all the volume component who intersects with the given frustum.
		/// \param frustum Frustum to test against.
		/// \return Returns the list of all the volumes who intersect the specified frustum.
		virtual vector<VolumeComponent*> GetIntersections(const Frustum& frustum) const = 0;

	};

}