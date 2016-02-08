#include "dx11/dx11material.h"

#include "scope_guard.h"
#include "core.h"
#include "gilib.h"

#include "windows/win_os.h"

#include "dx11/dx11graphics.h"
#include "dx11/dx11shader.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::gi_lib::dx11;
using namespace ::windows;

namespace {

	COMPtr<ID3D11InputLayout> CreateInputLayout(const string& hlsl, const string& file_name) {

		// Input layout

		ID3D11InputLayout* input_layout;
		ShaderReflection reflection;

		// The bytecode is needed to validate the input layout. Genius idea... - TODO: Remove this from here

		ID3DBlob* blob;
		auto&& device = *DX11Graphics::GetInstance().GetDevice();

		CompileHLSL<ID3D11VertexShader>(hlsl,
										file_name,
										&blob,
										&reflection);
				
		auto bytecode = COMMove(&blob);

		// Create the input layout

		vector<D3D11_INPUT_ELEMENT_DESC> input_elements;
		
		D3D11_INPUT_ELEMENT_DESC input_element_desc;

		for (auto& element : reflection.vertex_shader.vertex_input) {

			input_element_desc.SemanticName = element.semantic.c_str();
			input_element_desc.SemanticIndex = element.index;
			input_element_desc.Format = element.format;
			input_element_desc.InputSlot = 0;
			input_element_desc.AlignedByteOffset = element.offset;
			input_element_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			input_element_desc.InstanceDataStepRate = 0;

			input_elements.push_back(input_element_desc);

		}
		
		THROW_ON_FAIL(device.CreateInputLayout(&input_elements[0],
											   static_cast<unsigned int>(input_elements.size()),
											   bytecode->GetBufferPointer(),
											   bytecode->GetBufferSize(),
											   &input_layout));

		return COMMove(&input_layout);

	}

}

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
	
	// Create the proper input layout

	input_layout_ = CreateInputLayout(code, file_name);

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