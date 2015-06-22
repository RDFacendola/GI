#include "dx11deferred_renderer.h"

#include "..\dx11.h"
#include "..\dx11resources.h"
#include "..\..\..\include\gimath.h"
#include "..\..\..\include\windows\win_os.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;
using namespace ::gi_lib::windows;
using namespace ::Eigen;

namespace{

	struct Light{

		float position[4];

	};

	void DrawIndexedSubset(ID3D11DeviceContext& context, const MeshSubset& subset){

		context.DrawIndexed(static_cast<unsigned int>(subset.count),
							static_cast<unsigned int>(subset.start_index),
							0);

	}

}

///////////////////////////////// DX11 DEFERRED RENDERER MATERIAL ///////////////////////////////

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const CompileFromFile& args) :
material_(new DX11Material(args))
{

	Setup();

}

DX11DeferredRendererMaterial::DX11DeferredRendererMaterial(const Instantiate& args) :
material_(new DX11Material(Material::Instantiate{ args.base->GetMaterial() })){

	Setup();

}

void DX11DeferredRendererMaterial::SetMatrix(const Affine3f& world, const Affine3f& view, const Matrix4f& projection){

	if (world_view_proj_){

		world_view_proj_->Set((projection * view * world).matrix());

	}

	if (world_){

		world_->Set(world.matrix());

	}

}

void DX11DeferredRendererMaterial::Setup(){

	world_view_proj_ = material_->GetVariable("gWorldViewProj");
	world_ = material_->GetVariable("gWorld");

}

///////////////////////////////// DX11 TILED DEFERRED RENDERER //////////////////////////////////

DX11TiledDeferredRenderer::DX11TiledDeferredRenderer(const RendererConstructionArgs& arguments) :
TiledDeferredRenderer(arguments.scene){

	auto& device = DX11Graphics::GetInstance().GetDevice();

	// Get the immediate rendering context.

	ID3D11DeviceContext* context;

	device.GetImmediateContext(&context);

	immediate_context_ = std::move(unique_com(context));

	// Create the depth stencil state

	D3D11_DEPTH_STENCIL_DESC depth_state_desc;

	ID3D11DepthStencilState* depth_state;
	
	depth_state_desc.DepthEnable = true;
	depth_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_state_desc.DepthFunc = D3D11_COMPARISON_LESS;
	depth_state_desc.StencilEnable = false;
	depth_state_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depth_state_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	
	depth_state_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_state_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_state_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_state_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	depth_state_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depth_state_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_state_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_state_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	device.CreateDepthStencilState(&depth_state_desc,
								   &depth_state);

	depth_state_.reset(depth_state);

	ZeroMemory(&depth_state_desc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	device.CreateDepthStencilState(&depth_state_desc,
								   &depth_state);

	disable_depth_test_.reset(depth_state);

	// Create the blend state

	D3D11_BLEND_DESC blend_state_desc;

	ID3D11BlendState* blend_state;
	
	blend_state_desc.AlphaToCoverageEnable = true;
	blend_state_desc.IndependentBlendEnable = false;

	blend_state_desc.RenderTarget[0].BlendEnable = false;
	blend_state_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend_state_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blend_state_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_state_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_state_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_state_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device.CreateBlendState(&blend_state_desc,
							&blend_state);

	blend_state_.reset(blend_state);

	// Create the raster state.

	D3D11_RASTERIZER_DESC rasterizer_state_desc;

	ID3D11RasterizerState* rasterizer_state;

	rasterizer_state_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_state_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_state_desc.FrontCounterClockwise = false;
	rasterizer_state_desc.DepthBias = 0;
	rasterizer_state_desc.SlopeScaledDepthBias = 0.0f;
	rasterizer_state_desc.DepthBiasClamp = 0.0f;
	rasterizer_state_desc.DepthClipEnable = true;
	rasterizer_state_desc.ScissorEnable = false;
	rasterizer_state_desc.MultisampleEnable = false;
	rasterizer_state_desc.AntialiasedLineEnable = false;

	device.CreateRasterizerState(&rasterizer_state_desc,
								 &rasterizer_state);

	rasterizer_state_.reset(rasterizer_state);
	
	//

	light_array_ = new DX11StructuredVector(StructuredVector::FromDescription{ 32, sizeof(Light) });

	InitializeToneMap();

	// TODO: Remove this

	auto& file_system = FileSystem::GetInstance();

	auto file_name = Application::GetInstance().GetDirectory() + L"Data\\cstest.hlsl";

	string code = to_string(file_system.Read(file_name));

	ShaderReflection reflection;

	reflection.shaders = ShaderType::NONE;

	ID3D11ComputeShader* cs;

	wstring errors;

	THROW_ON_FAIL(MakeShader<ID3D11ComputeShader>(device, 
												  code, 
												  to_string(file_name), 
												  &cs, 
												  &reflection, 
												  &errors));

	light_cs_.reset(cs);

}

DX11TiledDeferredRenderer::~DX11TiledDeferredRenderer(){}

void DX11TiledDeferredRenderer::Draw(ObjectPtr<RenderTarget> render_target){
	
	// Draws only if there's a camera

	if (GetScene().GetMainCamera()){
		
		auto dx11_render_target = resource_cast(render_target);
	
		auto width = dx11_render_target->GetWidth();
		auto height = dx11_render_target->GetHeight();

		DrawGBuffer(width, height);									// Scene -> GBuffer
		
		ComputeLighting(width, height);								// Scene, GBuffer, DepthBuffer -> LightBuffer

		StartPostProcess();

		ToneMap((*light_buffer_)[0]->GetView(),			// LightBuffer -> Output
				*dx11_render_target);

	}

	// Cleanup
	immediate_context_->OMSetRenderTargets(0, nullptr, nullptr);

	immediate_context_->ClearState();
	
}

void DX11TiledDeferredRenderer::DrawGBuffer(unsigned int width, unsigned int height){

	// Lazy initialization of the GBuffer

	if (!gbuffer_){

		gbuffer_ = new DX11RenderTarget(width,
										height,
										{ DXGI_FORMAT_R16G16B16A16_FLOAT,
										  DXGI_FORMAT_R16G16B16A16_FLOAT });

	}
	else{

		gbuffer_->Resize(width,
						 height);
		
	}

	immediate_context_->OMSetRenderTargets(0, nullptr, nullptr);

	SetViewport(*immediate_context_,
				width,
				height);

	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediate_context_->RSSetState(rasterizer_state_.get());

	immediate_context_->OMSetDepthStencilState(depth_state_.get(),
											   0);

	immediate_context_->OMSetBlendState(blend_state_.get(),
										0,
										0xFFFFFFFF);

	// Set up render GBuffer render targets

	gbuffer_->ClearDepthStencil(*immediate_context_,
								 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
								 1.0f,
								 0);

	Color color;

	color.color.alpha = 0.0f;
	color.color.red = 0.1f;
	color.color.green = 0.1f;
	color.color.blue = 0.1f;

	gbuffer_->ClearTargets(*immediate_context_,
							color);

	gbuffer_->Bind(*immediate_context_);

	// TODO: Clean this crap and minimize state changes
	
	float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

	Affine3f world_matrix;
	Affine3f view_matrix;
	Matrix4f projection_matrix;

	Matrix4f world_view_matrix;

	ObjectPtr<DX11Mesh> mesh;
	ObjectPtr<DX11DeferredRendererMaterial> material;

	auto& scene = GetScene();

	auto& camera = *scene.GetMainCamera();

	view_matrix = camera.GetViewTransform();

	projection_matrix = ComputePerspectiveProjectionLH(camera.GetFieldOfView(),
													   aspect_ratio,
													   camera.GetMinimumDistance(),
													   camera.GetMaximumDistance());

	// Frustum culling
	
	auto camera_frustum = camera.GetViewFrustum(aspect_ratio);

	auto nodes = scene.GetVolumeHierarchy()
					  .GetIntersections(camera_frustum,											// Updates the view frustum according to the output ratio.
										IVolumeHierarchy::PrecisionLevel::Medium);				// Avoids extreme false positive while keeping reasonably high performances.


	for (auto&& node : nodes){

		// Items to draw

		for (auto&& drawable : node->GetComponents<DeferredRendererComponent>()){

			// Bind the mesh

			mesh = resource_cast(drawable.GetMesh());

			mesh->Bind(*immediate_context_);

			// For each subset
			for (unsigned int subset_index = 0; subset_index < mesh->GetSubsetCount(); ++subset_index){

				material = drawable.GetMaterial(subset_index);

				// Fill the constant buffers

				world_matrix = drawable.GetComponent<TransformComponent>()->GetWorldTransform();

				material->SetMatrix(world_matrix,
									view_matrix,
									projection_matrix);
				
				// Bind the material

				material->Commit(*immediate_context_);

				// Draw	the subset
				DrawIndexedSubset(*immediate_context_,
								  mesh->GetSubset(subset_index));

			}

		}

	}

}

void DX11TiledDeferredRenderer::ComputeLighting(unsigned int width, unsigned int height){

	// Lazy initialization of the LightBuffer

	if (!light_buffer_){

		light_buffer_ = new DX11RenderTarget(width,
											 height,
											 { DXGI_FORMAT_R16G16B16A16_FLOAT },
											 true);

	}
	else{

		light_buffer_->Resize(width,
							  height);

	}
	
	immediate_context_->OMSetRenderTargets(0, nullptr, nullptr);

	ID3D11ShaderResourceView* srv[] = { resource_srv((*gbuffer_)[0]),
										resource_srv((*gbuffer_)[1]) };

	ID3D11UnorderedAccessView* uav[] = { resource_uav((*light_buffer_)[0]) };

	immediate_context_->CSSetShader(light_cs_.get(),
									nullptr,
									0);

	immediate_context_->CSSetShaderResources(0, 2, &srv[0]);
	immediate_context_->CSSetUnorderedAccessViews(0, 1, &uav[0], nullptr);


	unsigned int dispatchWidth = (width + 15) / 16;
	unsigned int dispatchHeight = (height + 15) / 16;

	immediate_context_->Dispatch(dispatchWidth, 
								 dispatchHeight, 
								 1);

	uav[0] = nullptr;

	immediate_context_->CSSetUnorderedAccessViews(0, 1, &uav[0], nullptr);

	return;

	// Set up LightBuffer render targets
	
	Color color;

	ZeroMemory(&color, sizeof(color));

	light_buffer_->ClearTargets(*immediate_context_,
								color);

	light_buffer_->Bind(*immediate_context_);

	// 

	// TODO: Perform an actual light setup and remove the fake lights.

	Light* light_ptr = light_array_->Map<Light>(*immediate_context_);

	for (size_t light_index = 0; light_index < light_array_->GetElementCount(); ++light_index){

		light_ptr->position[0] = light_index * 250.0f - light_array_->GetElementCount() * 125.0f;
		light_ptr->position[1] = std::cosf(static_cast<float>(light_index)) * 50.0f + 75.0f;
		light_ptr->position[2] = 0.0f;
		light_ptr->position[3] = 1.0f;

		++light_ptr;

	}

	light_array_->Unmap(*immediate_context_);

}

void DX11TiledDeferredRenderer::StartPostProcess(){

	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediate_context_->RSSetState(nullptr);

	immediate_context_->OMSetDepthStencilState(disable_depth_test_.get(),
											   0);

	immediate_context_->OMSetBlendState(nullptr, 0, 0xFFFFFFFF);

	immediate_context_->IASetVertexBuffers(0, 0, nullptr, 0, 0);

	immediate_context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	immediate_context_->IASetInputLayout(nullptr);
	
}

void DX11TiledDeferredRenderer::InitializeToneMap(){
		
	tonemapper_ = new DX11Material(Material::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\tonemapping.fx" });

	tonemap_source_ = tonemapper_->GetResource("gHDR");
	tonemap_vignette_ = tonemapper_->GetVariable("gVignette");
	tonemap_exposure_ = tonemapper_->GetVariable("gExposure");
	
}

void DX11TiledDeferredRenderer::ToneMap(ObjectPtr<IResourceView> source_view, DX11RenderTarget& destination){

	immediate_context_->OMSetRenderTargets(0, nullptr, nullptr);

	// Update the tonemap resources
	tonemap_source_->Set(source_view);
	tonemap_vignette_->Set(5.0f);
	tonemap_exposure_->Set(0.75f);

	// Bind the surfaces and the tonemapper to the context.

	destination.Bind(*immediate_context_);

	tonemapper_->Commit(*immediate_context_);

	// Draw a full-screen quad

	SetViewport(*immediate_context_, 
				destination.GetWidth(), 
				destination.GetHeight());
	
	immediate_context_->Draw(6, 0);
	
}