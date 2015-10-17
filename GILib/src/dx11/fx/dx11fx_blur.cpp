#include "dx11/fx/dx11fx_blur.h"

#include "core.h"
#include "dx11/dx11graphics.h"

using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::dx11::fx;

const Tag DX11FxGaussianBlur::kSourceTexture = "gSource";

const Tag DX11FxGaussianBlur::kDestinationTexture = "gBlurred";

const Tag DX11FxGaussianBlur::kBlurKernel = "gBlurKernel";

DX11FxGaussianBlur::DX11FxGaussianBlur(float sigma) {

	horizontal_blur_shader_ = new DX11Computation(IComputation::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\hblur.hlsl" });

	vertical_blur_shader_ = new DX11Computation(IComputation::CompileFromFile{ Application::GetInstance().GetDirectory() + L"Data\\Shaders\\vblur.hlsl" });

	blur_kernel_ = new DX11StructuredArray(kKernelSize, sizeof(float));

	// One-time setup

	horizontal_blur_shader_->SetInput(kBlurKernel,
									  ObjectPtr<IStructuredArray>(blur_kernel_));

	vertical_blur_shader_->SetInput(kBlurKernel,
									ObjectPtr<IStructuredArray>(blur_kernel_));

	SetSigma(sigma);

}

void DX11FxGaussianBlur::SetSigma(float sigma){

	sigma_ = sigma;

	auto kernel = reinterpret_cast<float*>(blur_kernel_->Lock());

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

	blur_kernel_->Unlock();

}

void DX11FxGaussianBlur::Blur(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination) {

	auto context = DX11Graphics::GetInstance().GetImmediateContext();

	auto width = source->GetWidth();
	auto height = source->GetHeight();
	auto format = resource_cast(source)->GetFormat();

	// Lazy initialization of the working texture
	if (temp_texture_ == nullptr ||
		temp_texture_->GetWidth() != width ||
		temp_texture_->GetHeight() != height ||
		temp_texture_->GetFormat() != format) {

		temp_texture_ = new DX11GPTexture2D(width, height, format);


	}

	// Horizontal blur - Source => Temp

	horizontal_blur_shader_->SetInput(kSourceTexture,
									  source);

	horizontal_blur_shader_->SetOutput(kDestinationTexture,
									   ObjectPtr<IGPTexture2D>(temp_texture_));

	horizontal_blur_shader_->Dispatch(*context,
									  width,
									  height, 
									  1);

	// Vertical blur - Temp => Destination

	vertical_blur_shader_->SetInput(kSourceTexture,
									ObjectPtr<ITexture2D>(temp_texture_->GetTexture()));

	vertical_blur_shader_->SetOutput(kDestinationTexture,
									 destination);

	vertical_blur_shader_->Dispatch(*context,
									width,
									height, 
									1);

}