#include "dx11deferred_renderer.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;

DX11TiledDeferredRenderer::DX11TiledDeferredRenderer(const RendererConstructionArgs& arguments) :
TiledDeferredRenderer(arguments.scene){}

DX11TiledDeferredRenderer::~DX11TiledDeferredRenderer(){}

void DX11TiledDeferredRenderer::Draw(IOutput& output){

	// Scene to draw
	auto& scene = GetScene();

	// Main camera
	auto& camera = *scene.GetMainCamera();

	// Nodes within the frustum culling
	auto nodes = GetScene().GetVolumeHierarchy()
						   .GetIntersections(camera.GetViewFrustum(),
											 IVolumeHierarchy::PrecisionLevel::Medium);			// Avoids extreme false positive while keeping reasonably high performances.



}