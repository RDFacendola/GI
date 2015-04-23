/// \file octree.h
/// \brief Octree component
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "..\gimath.h"
#include "volume_hierarchy.h"

namespace gi_lib{
	
	/// \brief Represents an uniform tree.
	/// The tree subdivides its domain in equally sized cells recursively.
	/// This solution works best for applications where the volumes are distribuited uniformly throughout the domain, however it has a large memory footprint.
	/// \author Raffaele D. Facendola
	class UniformTreeComponent : public VolumeHierarchyComponent
	{

	public:

		/// \brief Create a new octree.
		/// \param domain Region of space to subdivide.
		/// \param splits Number of times to split on each axis.
		UniformTreeComponent(const AABB& domain, const Vector3i& splits);

		/// \brief Destructor.
		virtual ~UniformTreeComponent();

		virtual void AddVolume(VolumeComponent* volume) override;

		virtual void RemoveVolume(VolumeComponent* volume) override;

		virtual vector<VolumeComponent*> GetIntersections(const Frustum& frustum, PrecisionLevel precision) const;
		
		virtual TypeSet GetTypes() const override;

	protected:
		
		virtual void Initialize() override;

		virtual void Finalize() override;

	private:

		struct Impl;

		struct Node;

		/// \brief Create a new uniform octree.
		/// \param parent Parent space containing this tree.
		/// \param domain Region of space to subdivide.
		/// \param splits Number of times to split on each axis.
		UniformTreeComponent(UniformTreeComponent* parent, const AABB& domain, const Vector3i& splits);

		/// \brief Split the current space at most once on each axis.
		/// \param splits Number of splits left on each axis.
		void Split(const Vector3i& splits);

		/// \brief Get all the volume component who intersects with the given frustum.
		/// \param frustum Frustum.
		/// \param precision Precision level of the test.
		/// \param The result vector.
		/// \remarks Coarse tests are faster but may lead to false positives.
		///          Fine tests, however, achieve lesser performances and produce no false positive.
		void GetIntersections(const Frustum& frustum, PrecisionLevel precision, vector<VolumeComponent*>& intersection) const;

		/// \brief Check whether a particular volume is fully enclosed in this subspace.
		/// \param volume Volume to check.
		/// \return Returns true if the volume is fully enclosed in this subspace, returns false otherwise.
		bool Encloses(const VolumeComponent& volume);

		UniformTreeComponent* parent_;						///< \brief Parent space.

		vector<UniformTreeComponent*> children_;			///< \brief Subspaces.

		vector<Node*> nodes_;								///< \brief Volumes contained in this subspace.

		AABB bounding_box_;									///< \brief Bounds of the octree node.

		unsigned int volume_count_;							///< \brief Cumulative volume count.

	};

}