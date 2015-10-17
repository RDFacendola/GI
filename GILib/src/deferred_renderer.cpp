#include "deferred_renderer.h"

#include "mesh.h"

using namespace std;
using namespace gi_lib;
	
/////////////////////////////// DEFERRED RENDERER COMPONENT //////////////////////////

DeferredRendererComponent::DeferredRendererComponent(MeshComponent& mesh_component) :
mesh_component_(mesh_component){

	// If the mesh component gets removed, this component is removed as well.

	on_mesh_removed_listener_ = mesh_component.OnRemoved().Subscribe([this](Listener& listener, Component::OnRemovedEventArgs&){

		listener.Unsubscribe();

		RemoveComponent();

	});
	
	// One material per mesh subset.

	materials_.resize(mesh_component_.GetMesh()->GetSubsetCount());

}

DeferredRendererComponent::~DeferredRendererComponent(){}

ObjectPtr<IStaticMesh> DeferredRendererComponent::GetMesh(){

	return ObjectPtr<IStaticMesh>(mesh_component_.GetMesh());

}

ObjectPtr<const IStaticMesh> DeferredRendererComponent::GetMesh() const{

	return ObjectPtr<const IStaticMesh>(mesh_component_.GetMesh());

}

unsigned int DeferredRendererComponent::GetMaterialCount() const{

	return static_cast<unsigned int>(mesh_component_.GetMesh()->GetSubsetCount());

}

ObjectPtr<DeferredRendererMaterial> DeferredRendererComponent::GetMaterial(unsigned int material_index){

	return materials_[material_index];

}

ObjectPtr<const DeferredRendererMaterial> DeferredRendererComponent::GetMaterial(unsigned int material_index) const{

	return ObjectPtr<const DeferredRendererMaterial>(materials_[material_index]);

}

void DeferredRendererComponent::SetMaterial(unsigned int material_index, ObjectPtr<DeferredRendererMaterial> material){

	materials_[material_index] = material;

}

DeferredRendererComponent::TypeSet DeferredRendererComponent::GetTypes() const{

	auto types = Component::GetTypes();

	types.insert(type_index(typeid(DeferredRendererComponent)));

	return types;

}

void DeferredRendererComponent::Initialize(){

	transform_component_ = GetComponent<TransformComponent>();

}

void DeferredRendererComponent::Finalize(){

	transform_component_ = nullptr;

}

/////////////////////////////// TILED DEFERRED RENDERER ///////////////////////////////

TiledDeferredRenderer::TiledDeferredRenderer(Scene& scene) :
scene_(scene){}

TiledDeferredRenderer::~TiledDeferredRenderer(){}

Scene& TiledDeferredRenderer::GetScene(){

	return scene_;

}

const Scene& TiledDeferredRenderer::GetScene() const{

	return scene_;

}