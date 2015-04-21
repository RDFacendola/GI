/// \file octree.h
/// \brief Octree component
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "..\gimath.h"
#include "volume_hierarchy.h"

namespace gi_lib{
	
	/// \brief Represents an uniform octree.
	/// The octree subdivides the specified region as much as possible.
	/// This solution works best for applications where the volumes are distribuited uniformly throughout the domain.
	/// For other applications it may cause an excessive consumption of memory.
	/// \author Raffaele D. Facendola
	class UniformOctreeComponent : public VolumeHierarchyComponent
	{

	public:

		/// \brief Create a new octree.
		/// \param domain Region of space to subdivide.
		/// \param min_size Minimum size per octree node.
		UniformOctreeComponent(const AABB& domain, const Vector3f& min_size);

		/// \brief Destructor.
		virtual ~UniformOctreeComponent();

		virtual void AddVolume(VolumeComponent* volume) override;

		virtual void RemoveVolume(VolumeComponent* volume) override;

		virtual vector<VolumeComponent*> GetIntersections(const Frustum& frustum, PrecisionLevel precision) const;
		
		virtual TypeSet GetTypes() const override;

	protected:

		/// \brief Create a new uniform octree.
		/// \param parent Parent space containing this octree.
		/// \param domain Region of space to subdivide.
		/// \param min_size Minimum size per octree node.
		UniformOctreeComponent(UniformOctreeComponent* parent, const AABB& domain, const Vector3f& min_size);

		virtual void Initialize() override;

		virtual void Finalize() override;

		/// \brief Split the current space in at most 8 subspaces.
		/// \param Minimum size per octree node.
		/// \return Returns true if the node was successfully splitted, returns false otherwise.
		bool Split(const Vector3f& min_size);

		/// \brief Get all the volume component who intersects with the given frustum.
		/// \param frustum Frustum.
		/// \param precision Precision level of the test.
		/// \param The result vector.
		/// \remarks Coarse tests are faster but may lead to false positives.
		///          Fine tests, however, achieve lesser performances and produce no false positive.
		void GetIntersections(const Frustum& frustum, PrecisionLevel precision, vector<VolumeComponent*>& intersection) const;

		UniformOctreeComponent* parent_;					///< \brief Parent space.

		vector<UniformOctreeComponent*> children_;			///< \brief Subspaces.

		vector<VolumeComponent*> volumes_;					///< \brief Volumes contained in this node.

		AABB bounds_;										///< \brief Bounds of the octree node.

		unsigned int volume_count_;							///< \brief Cumulative volume count.

	};

}