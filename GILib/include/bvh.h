/// \file bvh.h
/// \brief Interfaces to manage bounding volume hierarchies.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "gimath.h"

using std::vector;

namespace gi_lib{

	class Boundable;
	class SceneNode;

	/// \brief Interface for every bounding volume hierarchy
	/// \author Raffaele D. Facendola
	class BVH{

	public:

		/// \brief Virtual destructor.
		virtual ~BVH(){}

		/// \brief Rebuild the bounding volume hierarchy.
		virtual void Rebuild() = 0;

		/// \brief Add a new volume to the hierarchy.
		/// \param volume The volume to add to the hierarchy.
		virtual void AddBoundable(Boundable & volume) = 0;

		/// \brief Remove an existing volume from the hierarchy.

		/// If the specified volume couldn't be found the method does nothing.
		/// \param volume The volume to remove from the hierarchy.
		virtual void RemoveBoundable(Boundable & volume) = 0;

		/// \brief Get the set of nodes which overlaps or are contained inside the specified frustum.
		/// \param frustum The frustum used to test the volumes against.
		/// \return Returns the set of the nodes whose bounding boxes are contained or overlap the specified frustum.
		virtual vector<SceneNode *> GetIntersections(const Frustum & frustum) = 0;

	};

	/// \brief Represents an octree.
	/// \author Raffaele D. Facendola
	class Octree: public BVH{

	public:

		/// \brief Create a new unstructured octree.
		Octree();

		/// \brief Destroy this octree.
		~Octree();

		virtual void Rebuild() override;

		virtual void AddBoundable(Boundable & volume) override;

		virtual void RemoveBoundable(Boundable & volume) override;

		virtual vector<SceneNode *> GetIntersections(const Frustum & frustum) override;

	private:

		Octree(Octree * parent, const Bounds & bounds);

		void Join(vector<Boundable *> & objects);

		void Split(const Vector3f & min_extents, unsigned int max_objects);

		void RecomputeBounds();

		void GetIntersections(const Frustum & frustum, vector<Boundable *> & objects);

		Octree * parent_;

		vector<Octree *> children_;

		vector<Boundable *> objects_;

		Bounds bounds_;
		
	};

}