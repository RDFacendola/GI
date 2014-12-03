#include "..\include\bvh.h"

#include <algorithm>

#include <Eigen\Core>

#include "..\include\components.h"
#include "..\include\scene.h"

using namespace gi_lib;
using namespace Eigen;

///////////////////////////////////// OCTREE ////////////////////////////////////

Octree::Octree():
parent_(nullptr),
bounds_({ Vector3f::Zero(), Vector3f::Zero() }){

}

Octree::~Octree(){

	objects_.clear();

	for (auto child : children_){

		delete child;

	}

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

	// Searching inside an octree will
	// - check against every object inside every node considered. If the objects are spread uniformly across the space and we ignore object overlapping two or more nodes
	//   we can safely assume that all the objects are stored inside the 8^depth leaves. This means that the average number of objects per leaf is objects / 8 ^ depth.
	// - check against the bounds of all 8 children of every node found during tree traversal. Each traversal from root to leaf cost "depth", thus the total cost is 8 times that amount.
	// - the cost of a single traversal must be payed for each node overlapping the camera.
	// Cost function = overlaps * ( avg_objects_per_node + traversal_cost )
	//				 = overlaps * ( objects_count / 8 ^ depth + 8 depth )
	// Minimize the above to find the following approximation:
	
	const float kOverlap = 8;	// 8 is just a guess here...

	if (objects_.size() > 0){

		auto depth = std::lround(0.4809f * std::log(0.25993f * kOverlap * objects_.size()));

		max_objects = static_cast<unsigned int>(std::lround(objects_.size() / std::pow(8, depth)));
		min_extents = bounds_.half_extents * (1.0f / std::powf(2.0f, static_cast<float>(depth)));

	}
	else{

		max_objects = 1;
		min_extents = bounds_.half_extents;

	}
	
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

	parent_objects.resize(parent->objects_.size());
	objects_.resize(parent->objects_.size());

	auto new_ends = std::partition_copy(parent->objects_.begin(),
		parent->objects_.end(),
		objects_.begin(),
		parent_objects.begin(),
		[bounds](Boundable * object){

		return object->GetBounds().Inside(bounds);

	});

	parent_objects.erase(new_ends.second,
		parent_objects.end());

	objects_.erase(new_ends.first,
		objects_.end());

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
		bounds_.half_extents(0) >= min_extents(0) * 2.0f &&
		bounds_.half_extents(1) >= min_extents(1) * 2.0f &&
		bounds_.half_extents(2) >= min_extents(2) * 2.0f){
		
		Vector3f quarter = bounds_.half_extents * 0.5f;				// Offset of each children from the center of the parent.

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
				quarter }));

			// Split recursively
			children_.back()->Split(min_extents, max_objects);

		}

	}

}

void Octree::RecomputeBounds(){

	bounds_.center = Vector3f::Zero();
	bounds_.half_extents = Vector3f::Zero();

	if (objects_.size() > 0){

		bounds_ = *(objects_.back());

		Vector3f min = bounds_.center - bounds_.half_extents;
		Vector3f max = bounds_.center + bounds_.half_extents;

		for (auto object : objects_){

			bounds_ = *object;

			min = Math::Min(min, bounds_.center + bounds_.half_extents);
			max = Math::Max(max, bounds_.center - bounds_.half_extents);

		}

		bounds_.center = (max + min) * 0.5f;
		bounds_.half_extents = (max - min) * 0.5f;

	}

}

void Octree::AddBoundable(Boundable & volume){

	objects_.push_back(&volume);

	// TODO listen to the volume

}

void Octree::RemoveBoundable(Boundable & volume){

	// TODO Remove the actual volume

}

vector<SceneNode *> Octree::GetIntersections(const Frustum & frustum){

	vector<Boundable *> objects;

	// Internal recursive method.

	GetIntersections(frustum, objects);

	vector<SceneNode *> nodes;

	nodes.resize(objects.size());

	// Returns the node list from the boundable list.

	std::transform(objects.begin(), objects.end(), nodes.begin(), [](Boundable * boundable){

		return &(boundable->GetNode());

	});

	return nodes;

}

void Octree::GetIntersections(const Frustum & frustum, vector<Boundable *> & objects){

	if (!frustum.Intersect(bounds_)){

		// The rest of the hierarchy won't intersect the frustum either...

		return;

	}
	else{

		// Reserve the entire capacity

		objects.resize(objects.size() + objects_.size());

		auto end_it = std::copy_if(objects_.begin(), 
								   objects_.end(), 
								   objects.end(), 
								   [&frustum](Boundable * b){ 
			
									return frustum.Intersect(*b); 
		
								    });

		// Shrink it down
		objects_.erase(end_it, objects_.end());

		// Recursion...

		for (auto child : children_){

			child->GetIntersections(frustum, objects);

		}

	}

}