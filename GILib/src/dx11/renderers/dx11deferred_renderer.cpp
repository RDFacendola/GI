#include "dx11deferred_renderer.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;

///////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const CompileFromFile& args) :
material_(new DX11Material(args))
{



}

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const Instantiate& args) :
material_(new DX11Material(Material::Instantiate{ args.base->GetMaterial() })){

	

}

ObjectPtr<Material> DX11DeferredRendererMaterial::GetMaterial()
{
	
	return material_;

}

size_t gi_lib::dx11::DX11DeferredRendererMaterial::GetSize() const
{
	
	return material_->GetSize();

}

ObjectPtr<const Material> gi_lib::dx11::DX11DeferredRendererMaterial::GetMaterial() const
{
	
	return material_;

}

///////////////////////////////// DX11 TILED DEFERRED RENDERER //////////////////////////////////

DX11TiledDeferredRenderer::DX11TiledDeferredRenderer(const RendererConstructionArgs& arguments) :
TiledDeferredRenderer(arguments.scene){}

DX11TiledDeferredRenderer::~DX11TiledDeferredRenderer(){}

void DX11TiledDeferredRenderer::Draw(IOutput& output){
	
	// Scene to draw
	auto& scene = GetScene();

	// The cast is safe as long as the client is not mixing different APIs.
	auto& dx11output = static_cast<DX11Output&>(output);

	// Draws only if there's a camera

	if (scene.GetMainCamera()){

		// Main camera
		auto& camera = *scene.GetMainCamera();

		// Render target
		auto render_target = dx11output.GetRenderTarget();
	
		// Nodes within the frustum culling
		auto nodes = GetScene().GetVolumeHierarchy()
							   .GetIntersections(camera.GetViewFrustum(render_target->GetAspectRatio()),		// Updates the view frustum according to the output ratio.
												 IVolumeHierarchy::PrecisionLevel::Medium);						// Avoids extreme false positive while keeping reasonably high performances.

		for (auto&& node : nodes){

			// Items to draw

			auto drawables = node->GetComponents<DeferredRendererComponent>();

		}

	}

	// Present the image
	dx11output.Present();

}

