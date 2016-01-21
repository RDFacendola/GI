/// \file octree.h
/// \brief Octree component
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "gimath.h"
#include "volume_hierarchy.h"

namespace gi_lib{
	
	/// \brief Represents an uniform tree.
	/// The tree subdivides its domain in equally sized cells recursively.
	/// This solution works best for applications where the volumes are distributed uniformly throughout the domain, however it has a large memory footprint.
	/// \author Raffaele D. Facendola
	class UniformTree : public IVolumeHierarchy
	{

	public:

		/// \brief Create a new octree.
		/// \param domain Region of space to subdivide.
		/// \param splits Number of times to split on each axis.
		UniformTree(const AABB& domain, const Vector3i& splits);

		/// \brief Destructor.
		virtual ~UniformTree();

		virtual void AddVolume(VolumeComponent* volume) override;

		virtual void RemoveVolume(VolumeComponent* volume) override;

		virtual vector<VolumeComponent*> GetIntersections(const Frustum& frustum) const override;

		virtual vector<VolumeComponent*> GetIntersections(const Sphere& sphere) const override;

		virtual vector<VolumeComponent*> GetIntersections(const AABB& aabb) const override;

	private:

		struct Impl;

		struct Node;

		/// \brief Create a new uniform octree.
		/// \param parent Parent space containing this tree.
		/// \param domain Region of space to subdivide.
		/// \param splits Number of times to split on each axis.
		UniformTree(UniformTree* parent, const AABB& domain, const Vector3i& splits);

		/// \brief Split the current space at most once on each axis.
		/// \param splits Number of splits left on each axis.
		void Split(const Vector3i& splits);

		/// \brief Check whether a particular volume is fully enclosed in this subspace.
		/// \param volume Volume to check.
		/// \return Returns true if the volume is fully enclosed in this subspace, returns false otherwise.
		bool Encloses(const VolumeComponent& volume);

		UniformTree* parent_;								///< \brief Parent space.

		vector<UniformTree*> children_;						///< \brief Subspaces.

		vector<Node*> nodes_;								///< \brief Volumes contained in this subspace.

		AABB bounding_box_;									///< \brief Bounds of the octree node.

		unsigned int volume_count_;							///< \brief Cumulative volume count.

	};

}