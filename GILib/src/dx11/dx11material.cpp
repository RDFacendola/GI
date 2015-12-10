#include "dx11/dx11material.h"

#include "scope_guard.h"
#include "core.h"
#include "gilib.h"

#include "windows/win_os.h"

#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

//////////////////////////////  MATERIAL //////////////////////////////

DX11Material::DX11Material(const CompileFromFile& args) :
shader_composite_(make_unique<ShaderStateComposite>()){

	string code = to_string(FileSystem::GetInstance().Read(args.file_name));

	string file_name = to_string(args.file_name);

	auto rollback = make_scope_guard([this](){

		shader_composite_->Clear();

	});


	if (!shader_composite_->AddShader<ID3D11VertexShader>(code, file_name) ||
		!shader_composite_->AddShader<ID3D11PixelShader>(code, file_name)){

		THROW(L"A material must declare both a vertex shader and a pixel shader");

	}

	// Optional shaders. The method will do nothing if the entry point couldn't be found.

	shader_composite_->AddShader<ID3D11HullShader>(code, file_name);
	shader_composite_->AddShader<ID3D11DomainShader>(code, file_name);
	shader_composite_->AddShader<ID3D11GeometryShader>(code, file_name);

	// Input layout

	ID3D11InputLayout* input_layout;

	// The bytecode is needed to validate the input layout. Genius idea... - TODO: Remove this from here

	ID3DBlob* blob;
	auto&& device = *DX11Graphics::GetInstance().GetDevice();

	CompileHLSL<ID3D11VertexShader>(code,
									file_name,
									&blob);

	auto bytecode = COMMove(&blob);

	static const size_t kInputElements = 5;

	D3D11_INPUT_ELEMENT_DESC input_elements[kInputElements];

	input_elements[0].SemanticName = "SV_Position";
	input_elements[0].SemanticIndex = 0;
	input_elements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	input_elements[0].InputSlot = 0;
	input_elements[0].AlignedByteOffset = 0;
	input_elements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	input_elements[0].InstanceDataStepRate = 0;

	input_elements[1].SemanticName = "NORMAL";
	input_elements[1].SemanticIndex = 0;
	input_elements[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	input_elements[1].InputSlot = 0;
	input_elements[1].AlignedByteOffset = 12;
	input_elements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	input_elements[1].InstanceDataStepRate = 0;

	input_elements[2].SemanticName = "TEXCOORD";
	input_elements[2].SemanticIndex = 0;
	input_elements[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	input_elements[2].InputSlot = 0;
	input_elements[2].AlignedByteOffset = 24;
	input_elements[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	input_elements[2].InstanceDataStepRate = 0;

	input_elements[3].SemanticName = "TANGENT";
	input_elements[3].SemanticIndex = 0;
	input_elements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	input_elements[3].InputSlot = 0;
	input_elements[3].AlignedByteOffset = 32;
	input_elements[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	input_elements[3].InstanceDataStepRate = 0;

	input_elements[4].SemanticName = "BINORMAL";
	input_elements[4].SemanticIndex = 0;
	input_elements[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	input_elements[4].InputSlot = 0;
	input_elements[4].AlignedByteOffset = 44;
	input_elements[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	input_elements[4].InstanceDataStepRate = 0;

	THROW_ON_FAIL(device.CreateInputLayout(input_elements,
										   kInputElements,
										   bytecode->GetBufferPointer(),
										   bytecode->GetBufferSize(),
										   &input_layout));

	input_layout_ << &input_layout;

	// Dismiss
	rollback.Dismiss();

}

DX11Material::DX11Material(unique_ptr<ShaderStateComposite> shader_composite, const COMPtr<ID3D11InputLayout> input_layout) :
shader_composite_(std::move(shader_composite)),
input_layout_(input_layout){}

ObjectPtr<IMaterial> DX11Material::Instantiate() {

	auto instance = new DX11Material(make_unique<ShaderStateComposite>(*shader_composite_),
									 input_layout_);

	// TODO: add a reference to the instance inside the ResourcesManager, for tracking and debugging purposes

	return instance;

}

DX11Material::~DX11Material(){}