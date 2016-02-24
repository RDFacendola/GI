#include "dx11/dx11gpgpu.h"

#include "core.h"
#include "gilib.h"
#include "scope_guard.h"

#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

namespace{

}

///////////////////////////////// DX11 COMPUTATION ///////////////////////////////////

DX11Computation::DX11Computation(const CompileFromFile& arguments) :
shader_composite_(std::make_unique<ShaderStateComposite>()){

	std::string hlsl = to_string(FileSystem::GetInstance().Read(arguments.file_name));

	std::string file_name = to_string(arguments.file_name);
	
	vector<D3D_SHADER_MACRO> macros;

	for (auto& macro : arguments.macros) {

		macros.push_back({ macro.macro.c_str(),
						   macro.value.c_str() });

	}

	if(!shader_composite_->AddShader<ID3D11ComputeShader>(hlsl,
														  file_name,
														  macros)){

		// The function returns false only if the entry point couldn't be found.
		THROW(L"Invalid compute shader code.");

	}

	// Access thread group size.

	auto& reflection = shader_composite_->GetShaderState(0).GetReflection().compute_shader;	

	group_size_ = Vector3i(reflection.thread_group_x,
						   reflection.thread_group_y,
						   reflection.thread_group_z);

}

DX11Computation::~DX11Computation(){}

void DX11Computation::Dispatch(ID3D11DeviceContext& context, unsigned int x, unsigned int y, unsigned int z){

	// Bind to the context (implies committing pending resources)

	shader_composite_->Bind(context);

	// Dispatch - Approximate by excess the number of thread groups to dispatch per axis.

	unsigned int dispatch_width = (x + group_size_(0) - 1) / group_size_(0);
	unsigned int dispatch_height = (y + group_size_(1) - 1) / group_size_(1);
	unsigned int dispatch_depth = (z + group_size_(2) - 1) / group_size_(2);

	context.Dispatch(dispatch_width,
					 dispatch_height,
					 dispatch_depth);

	// Unbind from the context

	shader_composite_->Unbind(context);
	
	// Yay!

}