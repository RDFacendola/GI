

#include "dx11/dx11.h"

#pragma comment(lib, "dxguid.lib")

#include <sstream>
#include <algorithm>

#include "exceptions.h"
#include "enums.h"
#include "scope_guard.h"
#include "windows\win_os.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::windows;

//////////////////////////////////// GRAPHICS ////////////////////////////////////

DX11Utils & DX11Utils::GetInstance() {

	static DX11Utils utils;

	return utils;

}

DX11Utils::DX11Utils() {}

DX11Utils::~DX11Utils() {}

void DX11Utils::PushDepthStencilState(ID3D11DeviceContext& device_context, ID3D11DepthStencilState& depth_stencil_state) {

	ID3D11DepthStencilState* current_state;

	device_context.OMGetDepthStencilState(&current_state, nullptr);		// Increase the ref count!

	depth_stencil_state_.push_back(current_state);

	device_context.OMSetDepthStencilState(&depth_stencil_state, 0);
	
}

void DX11Utils::PushRasterizerState(ID3D11DeviceContext& device_context, ID3D11RasterizerState& rasterizer_state) {

	ID3D11RasterizerState* current_state;

	device_context.RSGetState(&current_state);							// Increase the ref count!

	rasterizer_state_.push_back(current_state);

	device_context.RSSetState(&rasterizer_state);

}

void DX11Utils::PopDepthStencilState(ID3D11DeviceContext& device_context) {

	ID3D11DepthStencilState* state = depth_stencil_state_.back();

	device_context.OMSetDepthStencilState(state, 0);

	state->Release();													// Decrease the ref count!

}

void DX11Utils::PopRasterizerState(ID3D11DeviceContext& device_context) {

	ID3D11RasterizerState* state = rasterizer_state_.back();

	device_context.RSSetState(state);									
	
	state->Release();													// Decrease the ref count!
	
}	

/////////////////// METHODS ///////////////////////////

HRESULT gi_lib::dx11::MakeDepthStencil(ID3D11Device& device, unsigned int width, unsigned int height, ID3D11ShaderResourceView** shader_resource_view, ID3D11DepthStencilView** depth_stencil_view){

	ID3D11Texture2D* texture = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;

	auto cleanup = make_scope_guard([&dsv, &srv, &texture]{

		if (dsv)		dsv->Release();
		if (srv)		srv->Release();

	});

	D3D11_TEXTURE2D_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	
	RETURN_ON_FAIL(device.CreateTexture2D(&desc, 
										  nullptr, 
										  &texture));

	COM_GUARD(texture);

	if (shader_resource_view){

		// Create the shader resource view.

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;

		ZeroMemory(&srv_desc, sizeof(srv_desc));

		srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;		// The stencil may not be accessed by the shader.
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MostDetailedMip = 0;
		srv_desc.Texture2D.MipLevels = 1;

		THROW_ON_FAIL(device.CreateShaderResourceView(texture,
													  &srv_desc,
													  &srv));

	}

	if (depth_stencil_view){
		
		// Create the depth stencil view.

		RETURN_ON_FAIL(MakeDepthStencilView(device,
											*texture,
											&dsv));

	}

	// Output
	if (shader_resource_view){

		*shader_resource_view = srv;

	}

	if (depth_stencil_view){

		*depth_stencil_view = dsv;

	}

	// Cleanup

	cleanup.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeRenderTarget(ID3D11Device& device, unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11ShaderResourceView** shader_resource_view, ID3D11RenderTargetView** render_target_view, bool mip_chain){

	ID3D11Texture2D* texture = nullptr;
	ID3D11RenderTargetView* rtv = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;

	auto cleanup = make_scope_guard([&rtv, &srv](){

		if (rtv)		rtv->Release();
		if (srv)		srv->Release();

	});

	D3D11_TEXTURE2D_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mip_chain ? 0 : 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = mip_chain ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	RETURN_ON_FAIL(device.CreateTexture2D(&desc,
										  nullptr,
										  &texture));

	COM_GUARD(texture);

	if (render_target_view){
		
		RETURN_ON_FAIL(MakeRenderTargetView(device,
											*texture,
											&rtv));

	}
	
	if (shader_resource_view){

		RETURN_ON_FAIL(device.CreateShaderResourceView(texture,
													   nullptr,
													   &srv));

	}
		
	// Output

	if (render_target_view){

		*render_target_view = rtv;

	}
	
	if (shader_resource_view){

		*shader_resource_view = srv;

	}

	// Cleanup

	cleanup.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeRenderTargetArray(ID3D11Device& device, unsigned int width, unsigned int height, unsigned int count, DXGI_FORMAT format, ID3D11ShaderResourceView** shader_resource_view, std::vector<ID3D11RenderTargetView*>* render_target_view_list, bool mip_chain) {

	ID3D11Texture2D* texture = nullptr;
	vector<ID3D11RenderTargetView*> rtv;
	ID3D11ShaderResourceView* srv = nullptr;

	auto cleanup = make_scope_guard([&rtv, &srv]() {

		for (auto&& rtv_item : rtv) {

			if (rtv_item) rtv_item->Release();

		}
		
		rtv.clear();

		if (srv)		srv->Release();

	});

	D3D11_TEXTURE2D_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	
	desc.ArraySize = count;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mip_chain ? 0 : 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = mip_chain ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	
	RETURN_ON_FAIL(device.CreateTexture2D(&desc,
										  nullptr,
										  &texture));

	COM_GUARD(texture);

	if (render_target_view_list) {

		rtv.resize(count);

		std::fill(rtv.begin(), rtv.end(), nullptr);

		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;

		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtv_desc.Format = format;

		for (unsigned int slice_index = 0; slice_index < count; ++slice_index) {

			rtv_desc.Texture2DArray.ArraySize = 1;						// 1 element
			rtv_desc.Texture2DArray.FirstArraySlice = slice_index;		// i-th element
			rtv_desc.Texture2DArray.MipSlice = 0;						// Most detailed mipmap
			
			RETURN_ON_FAIL(device.CreateRenderTargetView(texture,
														 &rtv_desc,
														 &rtv[slice_index]));

		}

	}

	if (shader_resource_view) {

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;

		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srv_desc.Texture2DArray.ArraySize = count;
		srv_desc.Texture2DArray.FirstArraySlice = 0;
		srv_desc.Texture2DArray.MipLevels = static_cast<unsigned int>(-1);
		srv_desc.Texture2DArray.MostDetailedMip = 0;

		RETURN_ON_FAIL(device.CreateShaderResourceView(texture,
													   &srv_desc,
													   &srv));

	}

	// Output

	if (render_target_view_list) {

		*render_target_view_list = std::move(rtv);

	}

	if (shader_resource_view) {

		*shader_resource_view = srv;

	}

	// Cleanup

	cleanup.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeUnorderedTexture(ID3D11Device& device, unsigned int width, unsigned int height, DXGI_FORMAT format, ID3D11UnorderedAccessView** unordered_access_view, ID3D11ShaderResourceView** shader_resource_view, unsigned int mips){

	ID3D11Texture2D* texture = nullptr;
	ID3D11UnorderedAccessView* uav = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;

	auto cleanup = make_scope_guard([&texture, &uav, &srv](){

		if (uav)		uav->Release();
		if (srv)		srv->Release();

	});

	D3D11_TEXTURE2D_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mips;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;

	RETURN_ON_FAIL(device.CreateTexture2D(&desc,
										  nullptr,
										  &texture));

	COM_GUARD(texture);

	if (unordered_access_view){
		
		RETURN_ON_FAIL(MakeUnorderedAccessView(device,
											   *texture,
											   &uav));

	}
	
	if (shader_resource_view){

		RETURN_ON_FAIL(device.CreateShaderResourceView(texture,
													   nullptr,
													   &srv));

	}
		
	// Output

	if (unordered_access_view){

		*unordered_access_view = uav;

	}
	
	if (shader_resource_view){

		*shader_resource_view = srv;

	}

#ifdef _DEBUG

	const char c_szName[] = "Unordered texture!";

	texture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_szName) - 1, c_szName);

#endif

	// Cleanup

	cleanup.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeUnorderedTexture(ID3D11Device& device, unsigned int width, unsigned int height, unsigned int depth, DXGI_FORMAT format, ID3D11UnorderedAccessView** unordered_access_view, ID3D11ShaderResourceView** shader_resource_view, unsigned int mips){

	ID3D11Texture3D* texture = nullptr;
	ID3D11UnorderedAccessView* uav = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;

	auto cleanup = make_scope_guard([&texture, &uav, &srv](){

		if (uav)		uav->Release();
		if (srv)		srv->Release();

	});

	D3D11_TEXTURE3D_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_TEXTURE3D_DESC));

	desc.Width = width;
	desc.Height = height;
	desc.Depth = depth;
	desc.MipLevels = mips;
	desc.Format = format;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	RETURN_ON_FAIL(device.CreateTexture3D(&desc,
										  nullptr,
										  &texture));

	COM_GUARD(texture);

	if (unordered_access_view){
		
		RETURN_ON_FAIL(MakeUnorderedAccessView(device,
											   *texture,
											   &uav));

	}
	
	if (shader_resource_view){

		RETURN_ON_FAIL(device.CreateShaderResourceView(texture,
													   nullptr,
													   &srv));

	}
		
	// Output

	if (unordered_access_view){

		*unordered_access_view = uav;

	}
	
	if (shader_resource_view){

		*shader_resource_view = srv;

	}

#ifdef _DEBUG

	const char c_szName[] = "Unordered texture!";

	texture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_szName) - 1, c_szName);

#endif

	// Cleanup

	cleanup.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeUnorderedTextureArray(ID3D11Device& device, unsigned int width, unsigned int height, unsigned int count, DXGI_FORMAT format, ID3D11UnorderedAccessView** unordered_access_view, ID3D11ShaderResourceView** shader_resource_view, unsigned int mips){

	ID3D11Texture2D* texture = nullptr;
	ID3D11UnorderedAccessView* uav = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;

	auto cleanup = make_scope_guard([&texture, &uav, &srv](){

		if (uav)		uav->Release();
		if (srv)		srv->Release();

	});

	D3D11_TEXTURE2D_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.ArraySize = count;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = mips;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;

	RETURN_ON_FAIL(device.CreateTexture2D(&desc,
										  nullptr,
										  &texture));

	COM_GUARD(texture);

	if (unordered_access_view){
	
		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;

		uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uav_desc.Format = format;
		uav_desc.Texture2DArray.ArraySize = count;
		uav_desc.Texture2DArray.FirstArraySlice = 0;
		uav_desc.Texture2DArray.MipSlice = 0;
		
		RETURN_ON_FAIL(device.CreateUnorderedAccessView(texture,
														&uav_desc,
														&uav));
		
	}
	
	if (shader_resource_view){

		RETURN_ON_FAIL(device.CreateShaderResourceView(texture,
													   nullptr,
													   &srv));

	}
		
	// Output

	if (unordered_access_view){

		*unordered_access_view = uav;

	}
	
	if (shader_resource_view){

		*shader_resource_view = srv;

	}

#ifdef _DEBUG

	const char c_szName[] = "Unordered texture array";

	texture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_szName) - 1, c_szName);

#endif

	// Cleanup

	cleanup.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeRawVertexBuffer(ID3D11Device& device, unsigned int element_count, ID3D11Buffer** buffer, ID3D11UnorderedAccessView** unordered_access_view) {

	static unsigned int kElementSize = 4;		// Raw buffer must have 32 bit elements only!

	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = kElementSize * element_count;
	buffer_desc.BindFlags = (unordered_access_view ? D3D11_BIND_UNORDERED_ACCESS : 0) | D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	// Transaction: either all the resources are created, or none.

	ID3D11Buffer* vertex_buffer = nullptr;
	ID3D11UnorderedAccessView* uav = nullptr;

	auto guard = make_scope_guard([&vertex_buffer, &uav]{

		if (vertex_buffer)		vertex_buffer->Release();
		if (uav)				uav->Release();

	});

	RETURN_ON_FAIL(device.CreateBuffer(&buffer_desc,
									   nullptr,
									   &vertex_buffer));

	if (unordered_access_view){

		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;

		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_R32_TYPELESS;					// Raw buffers contains 32 bit elements only
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = element_count;
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;			// Raw buffer

		RETURN_ON_FAIL(device.CreateUnorderedAccessView(vertex_buffer,
														&desc,
														&uav));
		
	}

	// Commit

	*buffer = vertex_buffer;

	if (unordered_access_view){

		*unordered_access_view = uav;

	}

	guard.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeVertexBuffer(ID3D11Device& device, const void* vertices, size_t size, ID3D11Buffer** buffer){

	// Fill in a buffer description.
	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	buffer_desc.ByteWidth = static_cast<unsigned int>(size);
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA init_data;

	init_data.pSysMem = vertices;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	// Create the buffer
	return device.CreateBuffer(&buffer_desc, 
							   &init_data,
							   buffer);

}

HRESULT gi_lib::dx11::MakeIndexBuffer(ID3D11Device& device, const unsigned int* indices, size_t size, ID3D11Buffer** buffer){

	// Fill in a buffer description.
	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = static_cast<unsigned int>(size);
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;

	// Define the resource data.
	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = indices;
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;

	// Create the buffer
	return device.CreateBuffer(&buffer_desc, 
							   &init_data, 
							   buffer);

}

HRESULT gi_lib::dx11::MakeConstantBuffer(ID3D11Device& device, size_t size, ID3D11Buffer** buffer){

	static unsigned int kMultipleOf = 16;		// Buffer size must be a multiple of 16 bytes

	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.ByteWidth = (static_cast<unsigned int>(size - 1) / kMultipleOf) * kMultipleOf + kMultipleOf;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	// Create the buffer
	return device.CreateBuffer(&buffer_desc, 
							   nullptr, 
							   buffer);

}

HRESULT gi_lib::dx11::MakeStructuredBuffer(ID3D11Device& device, unsigned int element_count, unsigned int element_size, bool dynamic, ID3D11Buffer** buffer, ID3D11ShaderResourceView** shader_resource_view, ID3D11UnorderedAccessView** unordered_access_view){

	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

	buffer_desc.ByteWidth = element_size * element_count;

	buffer_desc.BindFlags = (shader_resource_view ? D3D11_BIND_SHADER_RESOURCE : 0) |
							(unordered_access_view ? D3D11_BIND_UNORDERED_ACCESS : 0);

	buffer_desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	buffer_desc.StructureByteStride = element_size;

	// Transaction: either all the resources are created, or none.

	ID3D11Buffer* structured = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	ID3D11UnorderedAccessView* uav = nullptr;

	auto guard = make_scope_guard([&structured, &srv, &uav]{

		if (structured)		structured->Release();
		if (srv)			srv->Release();
		if (uav)			uav->Release();

	});

	RETURN_ON_FAIL(device.CreateBuffer(&buffer_desc,
									   nullptr,
									   &structured));

	if (shader_resource_view){

		RETURN_ON_FAIL(device.CreateShaderResourceView(structured,
													   nullptr,
													   &srv));

	}

	if (unordered_access_view){

		RETURN_ON_FAIL(device.CreateUnorderedAccessView(structured,
														nullptr, 
														&uav));
		
	}

	// Commit

	*buffer = structured;

	if (shader_resource_view){

		*shader_resource_view = srv;

	}

	if (unordered_access_view){

		*unordered_access_view = uav;

	}

	guard.Dismiss();

	return S_OK;

}

HRESULT gi_lib::dx11::MakeStagingBuffer(ID3D11Device& device, unsigned int element_count, unsigned int element_size, bool read_only, ID3D11Buffer** buffer) {

	D3D11_BUFFER_DESC buffer_desc;

	buffer_desc.Usage = D3D11_USAGE_STAGING;

	buffer_desc.ByteWidth = element_size * element_count;

	buffer_desc.BindFlags = 0;

	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | (!read_only ? D3D11_CPU_ACCESS_WRITE : 0);

	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	buffer_desc.StructureByteStride = element_size;

	RETURN_ON_FAIL(device.CreateBuffer(&buffer_desc,
									   nullptr,
									   buffer));
		
	return S_OK;

}

HRESULT gi_lib::dx11::MakeSampler(ID3D11Device& device, D3D11_TEXTURE_ADDRESS_MODE address_mode, D3D11_FILTER texture_filtering, unsigned int anisotropy_level, Vector4f border_color, ID3D11SamplerState** sampler){

	D3D11_SAMPLER_DESC desc;
		
	desc.AddressU = address_mode;
	desc.AddressV = address_mode;
	desc.AddressW = address_mode;
	desc.MipLODBias = 0.0f;
	desc.Filter = texture_filtering;
	desc.MaxAnisotropy = anisotropy_level;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	desc.BorderColor[0] = border_color(0);	
	desc.BorderColor[1] = border_color(1);
	desc.BorderColor[2] = border_color(2);
	desc.BorderColor[3] = border_color(3);

	desc.MinLOD = -FLT_MAX;
	desc.MaxLOD = FLT_MAX;
	
	// Create the sampler state
	return device.CreateSamplerState(&desc, 
									 sampler);

}

HRESULT gi_lib::dx11::MakePCFSampler(ID3D11Device& device, D3D11_TEXTURE_ADDRESS_MODE address_mode, ID3D11SamplerState** sampler) {

	D3D11_SAMPLER_DESC desc;

	desc.AddressU = address_mode;
	desc.AddressV = address_mode;
	desc.AddressW = address_mode;
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;			// Bilinear filtering
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 0;
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

	desc.BorderColor[0] = 0.f;
	desc.BorderColor[1] = 0.f;
	desc.BorderColor[2] = 0.f;
	desc.BorderColor[3] = 0.f;

	desc.MinLOD = -FLT_MAX;
	desc.MaxLOD = FLT_MAX;

	// Create the sampler state
	return device.CreateSamplerState(&desc,
									 sampler);

}

HRESULT gi_lib::dx11::MakeDepthStencilView(ID3D11Device& device, ID3D11Resource& resource, ID3D11DepthStencilView** depth_stencil_view){

	// Create the depth stencil view.

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;

	ZeroMemory(&desc, sizeof(desc));

	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	return device.CreateDepthStencilView(&resource,
										 &desc,
										 depth_stencil_view);

}

HRESULT gi_lib::dx11::MakeRenderTargetView(ID3D11Device& device, ID3D11Resource& resource, ID3D11RenderTargetView** render_target_view){

	return device.CreateRenderTargetView(&resource,
										 nullptr,
										 render_target_view);

}

HRESULT gi_lib::dx11::MakeUnorderedAccessView(ID3D11Device& device, ID3D11Resource& resource, ID3D11UnorderedAccessView** unordered_access_view){

	return device.CreateUnorderedAccessView(&resource,
											nullptr,
											unordered_access_view);

}

Matrix4f gi_lib::dx11::ComputePerspectiveProjectionLH(float field_of_view, float aspect_ratio, float near_plane, float far_plane){

	Matrix4f projection_matrix = Matrix4f::Identity();

	auto height = 1.0f / std::tanf(field_of_view * 0.5f);

	auto width = height / aspect_ratio;

	projection_matrix(0, 0) = width;

	projection_matrix(1, 1) = height;

	projection_matrix(2, 2) = far_plane / (far_plane - near_plane);

	projection_matrix(3, 3) = 0.f;

	projection_matrix(2, 3) = -(near_plane * far_plane) / (far_plane - near_plane);

	projection_matrix(3, 2) = 1.0f;

	return projection_matrix;

}

Matrix4f gi_lib::dx11::ComputeOrthographicProjectionLH(float width, float height, float near_plane, float far_plane) {

	Matrix4f projection_matrix = Matrix4f::Identity();

	projection_matrix(0, 0) = 2.f / width;

	projection_matrix(1, 1) = 2.f / height;

	projection_matrix(2, 2) = 1.f / (far_plane - near_plane);

	projection_matrix(2, 3) = -near_plane / (far_plane - near_plane);

	return projection_matrix;

}