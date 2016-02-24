#include "dx11/fx/dx11fx_filter.h"

#include "core.h"
#include "dx11/dx11graphics.h"

using namespace gi_lib;
using namespace gi_lib::dx11;

namespace {

	struct GaussianBlurParameters {

		Vector2i destination_offset;		///< \brief Destination offset, in pixels.

	};

}

///////////////////////////// DX11 GAUSSIAN BLUR /////////////////////////////

const Tag DX11FxGaussianBlur::kSourceTexture = "gSource";

const Tag DX11FxGaussianBlur::kDestinationTexture = "gDestination";

DX11FxGaussianBlur::DX11FxGaussianBlur(const Parameters& parameters) :
radius_(parameters.kernel_radius_){

	auto directory = Application::GetInstance().GetDirectory();

	std::string radius(std::to_string(parameters.kernel_radius_));

	hblur_shader_ = new DX11Computation(IComputation::CompileFromFile{ directory + L"Data\\Shaders\\1Dconvolution.hlsl", { { "RADIUS", radius }, { "HORIZONTAL" } } });

	vblur_shader_ = new DX11Computation(IComputation::CompileFromFile{ directory + L"Data\\Shaders\\1Dconvolution.hlsl", { { "RADIUS", radius }, { "VERTICAL" } } });

	kernel_ = new DX11StructuredArray(parameters.kernel_radius_ * 2 + 1, sizeof(float));

	parameters_ = new DX11StructuredBuffer(sizeof(GaussianBlurParameters));

	// One-time setup

	gp_cache_ = std::make_unique<DX11GPTexture2DCache>(IGPTexture2DCache::Singleton{});

	bool check;

	check = hblur_shader_->SetInput("gKernel",
								   ObjectPtr<IStructuredArray>(kernel_));

	check = hblur_shader_->SetInput("Parameters",
								   ObjectPtr<IStructuredBuffer>(parameters_));
	
	check = vblur_shader_->SetInput("gKernel",
								   ObjectPtr<IStructuredArray>(kernel_));

	check = vblur_shader_->SetInput("Parameters",
								   ObjectPtr<IStructuredBuffer>(parameters_));

	SetSigma(parameters.sigma_);

}

void DX11FxGaussianBlur::SetSigma(float sigma){

	sigma_ = sigma;

	auto kernel = reinterpret_cast<float*>(kernel_->Lock());

	// Calculate the Gaussian kernel

	float sum = 0.0f;

	for (int offset = -radius_; offset <= radius_; ++offset) {

		auto value = std::expf(-(offset * offset) / (2.0f * sigma * sigma));

		kernel[offset + radius_] = value;

		sum += value;

	}
	
	// Normalize the kernel s.t. the sum of the weights adds up to 1.

	auto kernel_size = radius_ * 2 + 1;

	for (int index = 0; index < kernel_size; ++index) {

		kernel[index] /= sum;

	}

	// Done

	kernel_->Unlock();

}

void DX11FxGaussianBlur::Blur(const ObjectPtr<ITexture2D>& source, const ObjectPtr<IGPTexture2D>& destination, const Vector2i& offset) {

	auto& graphics_ = DX11Graphics::GetInstance();

	graphics_.PushEvent(L"Gaussian Blur");

	auto context = DX11Graphics::GetInstance().GetImmediateContext();

	auto width = source->GetWidth();
	auto height = source->GetHeight();
	auto format = destination->GetFormat();

	// Lazy initialization of the working texture

	auto temp_texture_ = gp_cache_->PopFromCache(width, height, format, true);
	
	// Horizontal blur - Source => Temp

	parameters_->Lock<GaussianBlurParameters>()->destination_offset = Vector2i::Zero();

	parameters_->Unlock();

	hblur_shader_->SetInput(kSourceTexture,
						   source);

	hblur_shader_->SetOutput(kDestinationTexture,
							temp_texture_);

	hblur_shader_->Dispatch(*context,
						   width,
						   height, 
						   1);

	// Vertical blur - Temp => Destination + Offset

	parameters_->Lock<GaussianBlurParameters>()->destination_offset = offset;

	parameters_->Unlock();

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

	graphics_.PopEvent();

}