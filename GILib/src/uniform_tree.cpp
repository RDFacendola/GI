#include "uniform_tree.h"

#include <algorithm>

#include "gilib.h"
#include "gimath.h"
#include "observable.h"
#include "scene.h"
#include "exceptions.h"

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

		static vector<Vector3i> diff = { Vector3i(0, 0, 0),			// 0: None
										 Vector3i(1, 0, 0),			// 1: X
										 Vector3i(0, 1, 0),			// 2: Y
										 Vector3i(1, 1, 0),			// 3: XY
										 Vector3i(0, 0, 1),			// 4: Z
										 Vector3i(1, 0, 1),			// 5: XZ
										 Vector3i(0, 1, 1),			// 6: YZ
										 Vector3i(1, 1, 1) };		// 7: XYZ

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

////////////////////////////////// UNIFORMTREECOMPONENT :: NODE /////////////////////////////////////

struct UniformTree::Node{

	Node(UniformTree* parent, VolumeComponent* volume);

	/// \brief Push this node down the hierarchy.
	void PushDown();										

	/// \brief Pull this node up the hierarchy.
	void PullUp();	

	void SetParent(UniformTree* new_parent);

	UniformTree* parent_;									///< \brief Space containing this node

	VolumeComponent* volume_;								///< \brief Volume component inside this node

	unique_ptr<Listener> on_bounds_changed_listener_;		///< \brief Volume.OnBoundsChanged listener

};

UniformTree::Node::Node(UniformTree* parent, VolumeComponent* volume) :
parent_(parent),
volume_(volume){

	++(parent_->volume_count_);

	parent_->nodes_.push_back(this);

	on_bounds_changed_listener_ = volume->OnBoundsChanged().Subscribe([this](_, _){

		this->PullUp();	// Relocate the moving node

	});

}

void UniformTree::Node::PushDown(){
	
	auto new_parent = parent_;

	bool repeat;

	do{

		repeat = false;

		for (auto child : new_parent->children_){

			// Strictly enclosure ensures that at most one child may accept the volume.

			if (child->Encloses(*volume_)){

				++(child->volume_count_);

				new_parent = child;
				
				repeat = true;

				break;

			}

		}

	} while (repeat);	// Stop when every child rejected the volume

	// Set the new parent

	SetParent(new_parent);

}

void UniformTree::Node::PullUp(){

	auto new_parent = parent_;

	while (!new_parent->Encloses(*volume_) &&
			new_parent->parent_ != nullptr){

		--(new_parent->volume_count_);

		new_parent = new_parent->parent_;

	}

	SetParent(new_parent);

	// Find a more suitable subspace from the current parent
	PushDown();

}

void UniformTree::Node::SetParent(UniformTree* new_parent){

	if (parent_ != new_parent){

		auto& old_nodes = parent_->nodes_;

		auto it = std::find(old_nodes.begin(),
							old_nodes.end(),
							this);

		if (it != old_nodes.end()){

			old_nodes.erase(it);

		}
		else{

			// If this happens it means that we have a bug...
			THROW(L"Bad programmer exception.");

		}

		parent_ = new_parent;								// Update parent reference

		parent_->nodes_.push_back(this);					// Add the node to the new parent

	}

}

////////////////////////////////// UNIFORMTREECOMPONENT :: IMPL /////////////////////////////////////

struct UniformTree::Impl{

	template <IVolumeHierarchy::PrecisionLevel precision>
	static void GetIntersections(const UniformTree* tree, const Frustum& frustum, vector<VolumeComponent*>& intersections);

private:

	/// \brief Simple wrapper around the Node struct.
	struct VolumeMapper{

		VolumeComponent** operator()(UniformTree::Node* node) const;

	};

	using iterator = vector<UniformTree::Node*>::const_iterator;

	using iterator_wrapper = IteratorWrapper<vector<UniformTree::Node*>::const_iterator,
											 VolumeComponent*,
											 VolumeMapper>;
	
	template <IVolumeHierarchy::PrecisionLevel precision>
	static void GetIntersections(const vector<UniformTree::Node*>& nodes, const Frustum& frustum, vector<VolumeComponent*>& intersections);

};

VolumeComponent** UniformTree::Impl::VolumeMapper::operator()(UniformTree::Node* node) const{

	return &(node->volume_);

}

template <IVolumeHierarchy::PrecisionLevel precision>
void UniformTree::Impl::GetIntersections(const UniformTree* tree, const Frustum& frustum, vector<VolumeComponent*>& intersections){

	// Stop the recursion if this space doesn't intersect or if the subspace has no volumes inside.

	if (tree->volume_count_ > 0 &&
		(frustum.Intersect(tree->bounding_box_) && IntersectionType::kIntersect)){

		// Test against volumes

		GetIntersections<precision>(tree->nodes_, 
									frustum, 
									intersections);

		// Recursion

		for (auto child : tree->children_){

			GetIntersections<precision>(child,
										frustum, 
										intersections);

		}

	}

}

template <>
void UniformTree::Impl::GetIntersections<IVolumeHierarchy::PrecisionLevel::Coarse>(const vector<UniformTree::Node*>& nodes, const Frustum&, vector<VolumeComponent*>& intersections){

	// Copy every volume without testing. This may lead to some false positive (even far away) but requires no further test.

	auto size = intersections.size();

	intersections.resize(size + nodes.size());

	std::copy(iterator_wrapper(nodes.begin()),
			  iterator_wrapper(nodes.end()),
			  intersections.begin() + size);

}

template <>
void UniformTree::Impl::GetIntersections<IVolumeHierarchy::PrecisionLevel::Medium>(const vector<UniformTree::Node*>& nodes, const Frustum& frustum, vector<VolumeComponent*>& intersections){

	// Test each volume using the bounding sphere. May lead to some false positive near the frustum and is reasonably quick.

	auto size = intersections.size();

	intersections.resize(size + nodes.size());	// Overshoot

	auto end = std::copy_if(iterator_wrapper(nodes.begin()),
							iterator_wrapper(nodes.end()),
							intersections.begin() + size,
							[&frustum](const iterator_wrapper::reference volume){

								return frustum.Intersect(volume->GetBoundingSphere()) && IntersectionType::kIntersect;

							});

	intersections.erase(end,
						intersections.end());	// Shrink

}

template <>
void UniformTree::Impl::GetIntersections<IVolumeHierarchy::PrecisionLevel::Fine>(const vector<UniformTree::Node*>& nodes, const Frustum& frustum, vector<VolumeComponent*>& intersections){

	// Test each volume using maximum precision. No false positive is reported, however the performances may be affected.

	auto size = intersections.size();

	intersections.resize(size + nodes.size());	// Overshoot

	auto end = std::copy_if(iterator_wrapper(nodes.begin()),
							iterator_wrapper(nodes.end()),
							intersections.begin() + size,
							[&frustum](const iterator_wrapper::reference volume){

								// Hypothesis: rejected volumes are always more than accepted volumes.
								// Optimize the rejection case using a bounding sphere test first.

								return (frustum.Intersect(volume->GetBoundingSphere()) && IntersectionType::kIntersect) &&
									   (frustum.Intersect(volume->GetBoundingBox()) && IntersectionType::kIntersect);

							});

	intersections.erase(end,
						intersections.end());	// Shrink

}

///////////////////////////////////// UNIFORM TREE COMPONENT ////////////////////////////////////

UniformTree::UniformTree(const AABB& domain, const Vector3i& splits) :
UniformTree(nullptr, domain, splits){}

UniformTree::UniformTree(UniformTree* parent, const AABB& domain, const Vector3i& splits) :
parent_(parent),
bounding_box_(domain),
volume_count_(0u){

	Split(splits);

}

UniformTree::~UniformTree(){

	for (auto child : children_){

		delete child;

	}

}

void UniformTree::AddVolume(VolumeComponent* volume){

	auto node = new Node(this, volume);

	node->PushDown();
	
}

void UniformTree::RemoveVolume(VolumeComponent* volume){

	auto tree = this;

	bool repeat;

	do{

		repeat = false;

		// Strictly enclosure ensures that at most one child may accept the volume.

		for (auto& child : tree->children_){

			if (child->Encloses(*volume)){

				tree = child;

				repeat = true;

				break;

			}

		}

	} while (repeat);			// Stop when every child rejected the volume.

	// Remove the volume from the subspace

	auto& nodes = tree->nodes_;

	auto it = std::find_if(nodes.begin(),
						   nodes.end(),
						   [&volume](const Node* node){

								return node->volume_ == volume;

						   });

	delete *it;

	nodes.erase(it);	// We are sure the node exists, right?
	
}

vector<VolumeComponent*> UniformTree::GetIntersections(const Frustum& frustum, PrecisionLevel precision) const{

	vector<VolumeComponent*> intersections;

	intersections.reserve(volume_count_);	// Theoretical maximum number of volumes

	GetIntersections(frustum, precision, intersections);

	intersections.shrink_to_fit();			// Shrink to the actual value

	return intersections;

}

void UniformTree::Split(const Vector3i& splits){

	auto sub_splits = splits;
	Vector3f sub_extents = bounding_box_.half_extents;

	// Each child is equal

	for (auto& offset : GetSplitOffsets(sub_splits, sub_extents)){

		children_.push_back(new UniformTree(this,
											AABB{ bounding_box_.center + offset.cwiseProduct(sub_extents),
												  sub_extents },
											sub_splits));

	}

}

void UniformTree::GetIntersections(const Frustum& frustum, PrecisionLevel precision, vector<VolumeComponent*>& intersections) const{

	switch (precision){

	case PrecisionLevel::Coarse:

		Impl::GetIntersections<PrecisionLevel::Coarse>(this, frustum, intersections);
		break;

	case PrecisionLevel::Medium:

		Impl::GetIntersections<PrecisionLevel::Medium>(this, frustum, intersections);
		break;

	case PrecisionLevel::Fine:

		Impl::GetIntersections<PrecisionLevel::Fine>(this, frustum, intersections);
		break;

	}

}

bool UniformTree::Encloses(const VolumeComponent& volume){

	// False positive here are not acceptable here

	// Volumes must be strictly contained inside the cell, otherwise volumes touching the bounds will never be tested against touching object on neightbour subspaces.

	return bounding_box_.Intersect(volume.GetBoundingBox()) && IntersectionType::kInside;

}