/// \file bvh.h
/// \brief Interfaces to manage bounding volume hierarchies.
///
/// \author Raffaele D. Facendola

#include <vector>

#include "gimath.h"

using std::vector;

namespace gi_lib{

	class Boundable;

	/// \brief Interface for every bounding volume hierarchy
	/// \author Raffaele D. Facendola
	class BVH{

	public:

		/// \brief Rebuild the bounding volume hierarchy.
		virtual void Rebuild() = 0;

		/// \brief Add a new volume to the hierarchy.
		/// \param volume The volume to add to the hierarchy.
		virtual void AddVolume(Boundable & volume) = 0;

		/// \brief Remove an existing volume from the hierarchy.
		/// \param volume The volume to remove from the hierarchy.
		virtual void RemoveVolume(Boundable & volume) = 0;

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

		virtual void AddVolume(Boundable & volume) override;

		virtual void RemoveVolume(Boundable & volume) override;

	private:

		Octree(Octree * parent, const Bounds & bounds);

		void Join(vector<Boundable *> & objects);

		void Split(const Vector3f & min_extents, unsigned int max_objects);

		void RecomputeBounds();

		Octree * parent_;

		vector<Octree *> children_;

		vector<Boundable *> objects_;

		Bounds bounds_;
		
	};

}