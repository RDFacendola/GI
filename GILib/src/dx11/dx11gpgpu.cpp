#include "dx11/dx11gpgpu.h"

#pragma comment(lib, "d3dcompiler.lib")

#include "core.h"

#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

namespace{



}

DX11Computation::DX11Computation(const CompileFromFile& arguments){

	auto& file_system = FileSystem::GetInstance();

	string code = to_string(file_system.Read(arguments.file_name));

	string file_name = to_string(arguments.file_name);

	auto rollback = make_scope_guard([&](){

		compute_shader_ = nullptr;

	});

	ID3D11ComputeShader* compute_shader;
	wstring errors;

	reflection_.shaders = ShaderType::NONE;
	
	THROW_ON_FAIL(MakeShader<ID3D11ComputeShader>(DX11Graphics::GetInstance().GetDevice(),
												  code,
												  file_name,
												  &compute_shader,
												  &reflection_,
												  &errors),
				  errors);

	compute_shader_.reset(compute_shader);

}

DX11Computation::~DX11Computation(){


}

void DX11Computation::Dispatch(unsigned int x, unsigned int y, unsigned int z)
{
	
}

ObjectPtr<Object> DX11Computation::GetArgument(const string& name, const std::type_index& argument_type, GPUAccess access)
{

	if (access == GPUAccess::Read){

		// Either a shader resource view or a trivial type

	}
	else if (access == GPUAccess::Random){

		// Unordered access view.



		

	}
	else{

		return nullptr;

	}

}


