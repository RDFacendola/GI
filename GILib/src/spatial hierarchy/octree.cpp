#include "..\..\include\spatial hierarchy\octree.h"

#include <algorithm>

#include <Eigen\Core>

using namespace gi_lib;
using namespace Eigen;

namespace{

	// Unitary offsets of each children from the center of the parent (this)
	static vector<Vector3f> OctreeNodeOffset = { Vector3f(1.0f, 1.0f, 1.0f),
												 Vector3f(1.0f, 1.0f, -1.0f),
												 Vector3f(1.0f, -1.0f, 1.0f),
												 Vector3f(1.0f, -1.0f, -1.0f),
												 Vector3f(-1.0f, 1.0f, 1.0f),
												 Vector3f(-1.0f, 1.0f, -1.0f),
												 Vector3f(-1.0f, -1.0f, 1.0f),
												 Vector3f(-1.0f, -1.0f, -1.0f) };

}

///////////////////////////////////// OCTREE ////////////////////////////////////

UniformOctreeComponent::UniformOctreeComponent(const AABB& domain, const Vector3f& min_size) :
UniformOctreeComponent(nullptr, domain, min_size){}

UniformOctreeComponent::UniformOctreeComponent(UniformOctreeComponent* parent, const AABB& domain, const Vector3f& min_size) :
parent_(parent),
bounds_(domain),
volume_count_(0u){

	Split(min_size);

}

UniformOctreeComponent::~UniformOctreeComponent(){

	for (auto child : children_){

		delete child;

	}

}

void UniformOctreeComponent::AddVolume(VolumeComponent* volume){

	++volume_count_;

}

void UniformOctreeComponent::RemoveVolume(VolumeComponent* volume){

	--volume_count_;

}

vector<VolumeComponent*> UniformOctreeComponent::GetIntersections(const Frustum& frustum, PrecisionLevel precision) const{

	vector<VolumeComponent*> intersections;

	intersections.reserve(volume_count_);	// Theoretical maximum number of volumes

	GetIntersections(frustum, precision, intersections);

	intersections.shrink_to_fit();			// Shrink to the actual value

	return intersections;

}

UniformOctreeComponent::TypeSet UniformOctreeComponent::GetTypes() const{

	auto types = VolumeHierarchyComponent::GetTypes();

	types.insert(type_index(typeid(UniformOctreeComponent)));

	return types;

}

void UniformOctreeComponent::Initialize(){}

void UniformOctreeComponent::Finalize(){}

bool UniformOctreeComponent::Split(const Vector3f& min_size){

	// A node has either 8 children or none.

	// The node must be equal or greater than twice the minimum extents (so halving the node won't produce nodes that are smaller than the allowed minimum)

	if (bounds_.half_extents(0) >= min_size(0) * 2.0f &&
		bounds_.half_extents(1) >= min_size(1) * 2.0f &&
		bounds_.half_extents(2) >= min_size(2) * 2.0f){

		Vector3f quarter = bounds_.half_extents * 0.5f;				// Offset of each child from the center of the parent.

		for (auto & offset : OctreeNodeOffset){

			children_.push_back(new UniformOctreeComponent(this,
														   AABB{ bounds_.center + offset.cwiseProduct(quarter),
																 quarter },
														   min_size));

		}

		return true;

	}
	else{

		return false;	// No split

	}

}

void UniformOctreeComponent::GetIntersections(const Frustum& frustum, PrecisionLevel precision, vector<VolumeComponent*>& intersections) const{

	auto intersection_result = frustum.Intersect(bounds_);

	// Stop the recursion if this space doesn't intersect

	if (intersection_result && IntersectionType::kInside ||
		intersection_result && IntersectionType::kOverlapping){

		auto size = intersections.size();				// current size
		
		intersections.resize(intersections.size() +
							 volumes_.size());			// will shrink if necessary

		// Test against the volumes in this node

		if (precision == PrecisionLevel::Coarse){

			// Copy every volume without testing. This may lead to some false positive but is reasonably quick.
			
			std::copy(volumes_.begin(),
					  volumes_.end(),
					  intersections.begin() + size);

		}
		else{

			// Test each volume. Necessary to ensure no false positive, will affect performances.

			auto end = std::copy_if(volumes_.begin(),
									volumes_.end(),
									intersections.begin() + size,
									[&frustum](const VolumeComponent* volume){

										auto r = intersector(volume);

										return r && IntersectionType::kInside ||
											   r && IntersectionType::kOverlapping;

									});

			intersections.erase(end,
								intersections.end());	// discard unused positions


		}

		// Recursion

		for (auto& child : children_){

			child->GetIntersections(frustum, precision, intersections);

		}

	}

	

}
