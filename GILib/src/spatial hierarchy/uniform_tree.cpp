#include "..\..\include\spatial hierarchy\uniform_tree.h"

#include <algorithm>

#include <Eigen\Core>

using namespace gi_lib;
using namespace Eigen;

namespace{
	
	vector<Vector3f> GetSplitOffsets(Vector3i& splits, Vector3f& extents){

		static vector<vector<Vector3f>> offsets = { {},																// 0: None

													{ Vector3f(1.f, 0.f, 0.f), Vector3f(-1.f, 0.f, 0.f) },			// 1: X

													{ Vector3f(0.f, 1.f, 0.f), Vector3f(0.f, -1.f, 0.f) },			// 2: Y
													
													{ Vector3f(1.f, 1.f, 0.f), Vector3f(-1.f, 1.f, 0.f),
													  Vector3f(1.f, -1.f, 0.f), Vector3f(-1.f, -1.f, 0.f) },		// 3: XY

													{ Vector3f(0.f, 0.f, 1.f), Vector3f(0.f, 0.f, -1.f) },			// 4: Z

													{ Vector3f(1.f, 0.f, 1.f), Vector3f(-1.f, 0.f, 1.f), 
													  Vector3f(1.f, 0.f, -1.f), Vector3f(-1.f, 0.f, -1.f) },		// 5: XZ

													{ Vector3f(0.f, 1.f, 1.f), Vector3f(0.f, -1.f, 1.f), 
													  Vector3f(0.f, 1.f, -1.f), Vector3f(0.f, -1.f, -1.f) },		// 6: YZ

													{ Vector3f(1.f, 1.f, 1.f), Vector3f(-1.f, 1.f, 1.f), 
													  Vector3f(1.f, -1.f, 1.f), Vector3f(-1.f, -1.f, 1.f), 
													  Vector3f(1.f, 1.f, -1.f), Vector3f(-1.f, 1.f, -1.f), 
													  Vector3f(1.f, -1.f, -1.f), Vector3f(-1.f, -1.f, -1.f) } };	// 7: XYZ

		static vector<Vector3f> diff = { Vector3f(0.f, 0.f, 0.f),		// 0: None
										 Vector3f(1.f, 0.f, 0.f),		// 1: X
										 Vector3f(0.f, 1.f, 0.f),		// 2: Y
										 Vector3f(1.f, 1.f, 0.f),		// 3: XY
										 Vector3f(0.f, 0.f, 1.f),		// 4: Z
										 Vector3f(1.f, 0.f, 1.f),		// 5: XZ
										 Vector3f(0.f, 1.f, 1.f),		// 6: YZ
										 Vector3f(1.f, 1.f, 1.f) };		// 7: XYZ

		static vector<Vector3f> half = { Vector3f(1.f, 1.f, 1.f),		// 0: None
										 Vector3f(.5f, 1.f, 1.f),		// 1: X
										 Vector3f(1.f, .5f, 1.f),		// 2: Y
										 Vector3f(.5f, .5f, 1.f),		// 3: XY
										 Vector3f(1.f, 1.f, .5f),		// 4: Z
										 Vector3f(.5f, 1.f, .5f),		// 5: XZ
										 Vector3f(1.f, .5f, .5f),		// 6: YZ
										 Vector3f(.5f, .5f, .5f) };		// 7: XYZ

		size_t mask = (splits(0) > 0 ? 1 : 0) |		// x split -> 1, 3, 5, 7
					  (splits(1) > 0 ? 2 : 0) |		// y split -> 2, 3, 6, 7
					  (splits(2) > 0 ? 4 : 0);		// z split -> 4, 5, 6, 7

		splits -= diff[mask];

		extents = extents.cwiseProduct(half[mask]);

		return offsets[mask];

	}

}

///////////////////////////////////// OCTREE ////////////////////////////////////

UniformTreeComponent::UniformTreeComponent(const AABB& domain, const Vector3i& splits) :
UniformTreeComponent(nullptr, domain, splits){}

UniformTreeComponent::UniformTreeComponent(UniformTreeComponent* parent, const AABB& domain, const Vector3i& splits) :
parent_(parent),
bounding_box_(domain),
bounding_sphere_(Sphere::FromAABB(domain)),
volume_count_(0u){

	Split(splits);

}

UniformTreeComponent::~UniformTreeComponent(){

	for (auto child : children_){

		delete child;

	}

}

void UniformTreeComponent::AddVolume(VolumeComponent* volume){

	++volume_count_;



}

void UniformTreeComponent::RemoveVolume(VolumeComponent* volume){

	--volume_count_;

}

vector<VolumeComponent*> UniformTreeComponent::GetIntersections(const Frustum& frustum, PrecisionLevel precision) const{

	vector<VolumeComponent*> intersections;

	intersections.reserve(volume_count_);	// Theoretical maximum number of volumes

	GetIntersections(frustum, precision, intersections);

	intersections.shrink_to_fit();			// Shrink to the actual value

	return intersections;

}

UniformTreeComponent::TypeSet UniformTreeComponent::GetTypes() const{

	auto types = VolumeHierarchyComponent::GetTypes();

	types.insert(type_index(typeid(UniformTreeComponent)));

	return types;

}

void UniformTreeComponent::Initialize(){}

void UniformTreeComponent::Finalize(){}

void UniformTreeComponent::Split(const Vector3i& splits){

	auto sub_splits = splits;
	Vector3f sub_extents = bounding_box_.half_extents;

	// Each child is equal

	for (auto& offset : GetSplitOffsets(sub_splits, sub_extents)){

		children_.push_back(new UniformTreeComponent(this,
													 AABB{ bounding_box_.center + offset.cwiseProduct(sub_extents),
														   sub_extents },
													 sub_splits));

	}

}

void UniformTreeComponent::GetIntersections(const Frustum& frustum, PrecisionLevel precision, vector<VolumeComponent*>& intersections) const{

	// Stop the recursion if this space doesn't intersect or if the subspace has no volumes inside.

	// TODO: Resort to a sphere test if the bounds are "cubic" enough, it may save 4 mul and 4 add in case of failed test (the most common-case scenario)

	if (volume_count_ > 0 &&
		(frustum.Intersect(bounding_box_) && IntersectionType::kIntersect)){

		auto size = intersections.size();				// current size
		
		intersections.resize(intersections.size() +
							 volumes_.size());			// will shrink if necessary

		// Test against the volumes in this node

		if (precision == PrecisionLevel::Coarse){

			// Copy every volume without testing. This may lead to some false positive (even far away) but requires no further test.
			
			std::copy(volumes_.begin(),
					  volumes_.end(),
					  intersections.begin() + size);

		}
		else if (precision == PrecisionLevel::Medium){

			// Test each volume using the bounding sphere. May lead to some false positive near the frustum and is reasonably quick.

			auto end = std::copy_if(volumes_.begin(),
									volumes_.end(),
									intersections.begin() + size,
									[&frustum](const VolumeComponent* volume){

										return frustum.Intersect(volume->GetBoundingSphere()) && IntersectionType::kIntersect;

									});

			intersections.erase(end,
								intersections.end());	// discard unused positions

		} else /* if( precision == PrecisionLevel::Fine ) */ {

			// Test each volume using maximum precision. No false positive is reported, however the performances may be affected.

			auto end = std::copy_if(volumes_.begin(),
									volumes_.end(),
									intersections.begin() + size,
									[&frustum](const VolumeComponent* volume){

										// Hypothesis: rejected volumes are always more than accepted volumes.
										// Optimize the rejection case using a bounding sphere test first.

										return (frustum.Intersect(volume->GetBoundingSphere()) && IntersectionType::kIntersect) &&
											   (frustum.Intersect(volume->GetBoundingBox()) && IntersectionType::kIntersect);

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
