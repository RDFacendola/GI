#include "dx11deferred_renderer.h"

using namespace ::std;
using namespace ::gi_lib;

DX11TiledDeferredRenderer::~DX11TiledDeferredRenderer(){

}

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