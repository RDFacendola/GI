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

DX11Computation::DX11Computation(const CompileFromFile& arguments) :
shader_composite_(std::make_unique<ShaderStateComposite>()){

	std::string hlsl = to_string(FileSystem::GetInstance().Read(arguments.file_name));

	std::string file_name = to_string(arguments.file_name);
	
	if(!shader_composite_->AddShader<ID3D11ComputeShader>(hlsl,
														  file_name)){

		// The function returns false only if the entry point couldn't be found.
		THROW(L"Invalid compute shader code.");

	}

}

DX11Computation::~DX11Computation(){}

void DX11Computation::Dispatch(unsigned int x, unsigned int y, unsigned int z)
{
	
	

}



