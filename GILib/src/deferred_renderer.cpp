#include "deferred_renderer.h"

#include "mesh.h"

using namespace std;
using namespace gi_lib;

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