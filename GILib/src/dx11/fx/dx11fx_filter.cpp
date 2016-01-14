#include "dx11/fx/dx11fx_filter.h"

#include "core.h"
#include "dx11/dx11graphics.h"

using namespace gi_lib;
using namespace gi_lib::dx11;

///////////////////////////// DX11 GAUSSIAN BLUR /////////////////////////////

const Tag DX11FxGaussianBlur::kSourceTexture = "gSource";

const Tag DX11FxGaussianBlur::kDestinationTexture = "gBlurred";

const Tag DX11FxGaussianBlur::kBlurKernel = "gBlurKernel";

DX11FxGaussianBlur::DX11FxGaussianBlur(const Parameters& parameters) {

	auto directory = Application::GetInstance().GetDirectory();

	hblur_shader_ = new DX11Computation(IComputation::CompileFromFile{ directory + L"Data\\Shaders\\hblur.hlsl" });

	vblur_shader_ = new DX11Computation(IComputation::CompileFromFile{ directory + L"Data\\Shaders\\vblur.hlsl" });

	hblur_array_shader_ = new DX11Computation(IComputation::CompileFromFile{ directory + L"Data\\Shaders\\hblur_array.hlsl" });

	vblur_array_shader_ = new DX11Computation(IComputation::CompileFromFile{ directory + L"Data\\Shaders\\vblur_array.hlsl" });

	kernel_ = new DX11StructuredArray(kKernelSize, sizeof(float));

	// One-time setup

	gp_cache_ = std::make_unique<DX11GPTexture2DCache>(IGPTexture2DCache::Singleton{});

	hblur_shader_->SetInput(kBlurKernel,
							ObjectPtr<IStructuredArray>(kernel_));

	vblur_shader_->SetInput(kBlurKernel,
							ObjectPtr<IStructuredArray>(kernel_));

	hblur_array_shader_->SetInput(kBlurKernel,
								  ObjectPtr<IStructuredArray>(kernel_));

	vblur_array_shader_->SetInput(kBlurKernel,
								  ObjectPtr<IStructuredArray>(kernel_));

	SetSigma(parameters.sigma_);

}

void DX11FxGaussianBlur::SetSigma(float sigma){

	sigma_ = sigma;

	auto kernel = reinterpret_cast<float*>(kernel_->Lock());

	// Calculate the Gaussian kernel

	float sum = 0.0f;

	for (int offset = -kBlurRadius; offset <= kBlurRadius; ++offset) {

		auto value = std::expf(-(offset * offset) / (2.0f * sigma * sigma));

		kernel[offset + kBlurRadius] = value;

		sum += value;

	}
	
	// Normalize the kernel s.t. the sum of the weights adds up to 1.

	for (unsigned int index = 0; index < kKernelSize; ++index) {

		kernel[index] /= sum;

	}

	// Done

	kernel_->Unlock();

}

void DX11FxGaussianBlur::Blur(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination) {

	auto context = DX11Graphics::GetInstance().GetImmediateContext();

	auto width = source->GetWidth();
	auto height = source->GetHeight();
	auto format = destination->GetFormat();

	// Lazy initialization of the working texture

	auto temp_texture_ = gp_cache_->PopFromCache(width, height, format, true);
	
	// Horizontal blur - Source => Temp

	hblur_shader_->SetInput(kSourceTexture,
							source);

	hblur_shader_->SetOutput(kDestinationTexture,
							 temp_texture_);

	hblur_shader_->Dispatch(*context,
							width,
							height, 
							1);

	// Vertical blur - Temp => Destination

	vblur_shader_->SetInput(kSourceTexture,
							temp_texture_->GetTexture());

	vblur_shader_->SetOutput(kDestinationTexture,
							 destination);

	vblur_shader_->Dispatch(*context,
							width,
							height, 
							1);

	// Not needed anymore
	gp_cache_->PushToCache(temp_texture_);

}

void DX11FxGaussianBlur::Blur(const ObjectPtr<ITexture2DArray>& source, const ObjectPtr<IGPTexture2DArray>& destination){

	auto context = DX11Graphics::GetInstance().GetImmediateContext();

	auto width = source->GetWidth();
	auto height = source->GetHeight();
	auto depth = source->GetCount();		// dispatched along the Z axis.

	auto format = destination->GetFormat();

	// Lazy initialization of the working texture
	if (temp_texture_array_ == nullptr ||
		temp_texture_array_->GetWidth() != width ||
		temp_texture_array_->GetHeight() != height ||
		temp_texture_array_->GetCount() != depth ||
		temp_texture_array_->GetFormat() != format) {

		temp_texture_array_ = new DX11GPTexture2DArray(ITexture2DArray::FromDescription{ width,
																						 height,
																						 depth,
																						 1,
																						 format });

	}

	// Horizontal blur - Source => Temp

	hblur_array_shader_->SetInput(kSourceTexture,
								  source);

	hblur_array_shader_->SetOutput(kDestinationTexture,
								   ObjectPtr<IGPTexture2DArray>(temp_texture_array_));

	hblur_array_shader_->Dispatch(*context,
								  width,
								  height, 
								  depth);

	// Vertical blur - Temp => Destination

	vblur_array_shader_->SetInput(kSourceTexture,
								  temp_texture_array_->GetTextureArray());

	vblur_array_shader_->SetOutput(kDestinationTexture,
								   destination);

	vblur_array_shader_->Dispatch(*context,
								  width,
								  height, 
								  depth);

}