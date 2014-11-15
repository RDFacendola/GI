#include "..\include\bvh.h"

#include <algorithm>

#include <Eigen\Core>

#include "..\include\components.h"

using namespace gi_lib;
using namespace Eigen;

///////////////////////////////////// OCTREE ////////////////////////////////////

Octree::Octree():
parent_(nullptr),
bounds_{ Vector3f::Zero(), Vector3f::Zero() }{

}

Octree::~Octree(){

}

void Octree::Rebuild(){

	// Join the existing hierarchy

	vector<Boundable *> objects;

	Join(objects);

	objects_ = std::move(objects);

	// Recompute the bounds

	RecomputeBounds();
	
	// Evaluate optimal minimum extents and maximum objects per node
	Vector3f min_extents;
	unsigned int max_objects;

	// 3. ???????????
	// 4. Profit!

	// Split the hierarchy again
	Split(min_extents, max_objects);

}

Octree::Octree(Octree * parent, const Bounds & bounds) :
parent_(parent),
bounds_(bounds){

	// Take all those objects whose bounds fall exactly within this node's bounds.
	vector<Boundable *> parent_objects;

	std::partition_copy(parent->objects_.begin(),
		parent->objects_.end(),
		objects_.begin(),
		parent_objects.begin(),
		[bounds](Boundable * object){

		return object->GetBounds().Inside(bounds);

	});

	parent->objects_ = std::move(parent_objects);
	
}

void Octree::Join(vector<Boundable *> & objects){

	// Join parent's objects with this node's ones.

	objects.reserve(objects.size() + objects_.size());

	objects.insert(objects.end(), objects_.begin(), objects_.end());

	objects_.clear();

	// Join recursively, delete children afterwards.

	for (auto child : children_){

		child->Join(objects);

		delete child;

	}

	children_.clear();

}

void Octree::Split(const Vector3f & min_extents, unsigned int max_objects){

	// The node must be equal or greater than twice the minimum extents (so halving the node won't produce nodes that are smaller than the allowed minimum)

	if (objects_.size() > max_objects &&
		bounds_.extents(0) >= min_extents(0) * 2.0f &&
		bounds_.extents(1) >= min_extents(1) * 2.0f &&
		bounds_.extents(2) >= min_extents(2) * 2.0f ){
		
		Vector3f half = bounds_.extents * 0.5f;		// Extents of each children.
		Vector3f quarter = half * 0.5f;				// Offset of each children from the center of the parent.

		// Unitary offsets of each children from the center of the parent (this)
		static vector<Vector3f> offsets = { Vector3f(1.0f, 1.0f, 1.0f),
											Vector3f(1.0f, 1.0f, -1.0f),
											Vector3f(1.0f, -1.0f, 1.0f),
											Vector3f(1.0f, -1.0f, -1.0f),
											Vector3f(-1.0f, 1.0f, 1.0f),
											Vector3f(-1.0f, 1.0f, -1.0f),
											Vector3f(-1.0f, -1.0f, 1.0f),
											Vector3f(-1.0f, -1.0f, -1.0f) };

		for (auto & offset : offsets){

			children_.push_back(new Octree(this,
				Bounds{ bounds_.center + offset.cwiseProduct(quarter),
				half }));

			// Split recursively
			children_.back()->Split(min_extents, max_objects);

		}

	}

}

void Octree::RecomputeBounds(){

	bounds_.center = Vector3f::Zero();
	bounds_.extents = Vector3f::Zero();

	if (objects_.size() > 0){

		bounds_ = *(objects_.back());

		Vector3f min = bounds_.center - bounds_.extents * 0.5f;
		Vector3f max = min + bounds_.extents;

		for (auto object : objects_){

			bounds_ = *object;

			min = Math::Min(min, bounds_.center + bounds_.extents * 0.5f);
			max = Math::Max(max, bounds_.center - bounds_.extents * 0.5f);

		}

		bounds_.center = (min + max) * 0.5f;
		bounds_.extents = max - min;

	}

}

void Octree::AddVolume(Boundable & volume){

	objects_.push_back(&volume);

	// TODO listen to the volume

}

void Octree::RemoveVolume(Boundable & volume){

}

/*
Octree::Octree(const Bounds & bounds, int split){

	bounds_ = bounds;

	if (split > 0){


		
	}

}

Octree::Octree(Octree & parent, const Bounds & bounds, int split) :
Octree(bounds, split){

	parent_ = &parent;

}

Octree::~Octree(){

	for (auto child : children_){

		delete child;

	}

	parent_ = nullptr;

}

*/