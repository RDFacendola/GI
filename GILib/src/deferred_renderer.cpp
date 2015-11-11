#include "deferred_renderer.h"

#include "mesh.h"

using namespace std;
using namespace gi_lib;

/////////////////////////////// TILED DEFERRED RENDERER ///////////////////////////////

DeferredRenderer::DeferredRenderer(Scene& scene) :
scene_(scene){}

DeferredRenderer::~DeferredRenderer(){}

Scene& DeferredRenderer::GetScene(){

	return scene_;

}

const Scene& DeferredRenderer::GetScene() const{

	return scene_;

}