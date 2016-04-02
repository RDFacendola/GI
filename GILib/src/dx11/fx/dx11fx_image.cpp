#include "dx11/fx/dx11fx_image.h"

#include "core.h"

#include "dx11/dx11graphics.h"
#include <algorithm>

#undef max

using namespace gi_lib;
using namespace gi_lib::dx11;

/////////////////////////////////////// DX11 FX LUMINANCE /////////////////////////////////////// 

const Tag DX11FxLuminance::kSourceTexture = "gSource";
const Tag DX11FxLuminance::kHistogram = "gHistogram";
const Tag DX11FxLuminance::kParameters = "Parameters";

const unsigned int DX11FxLuminance::kBinCount = 64;

DX11FxLuminance::DX11FxLuminance(const Parameters& parameters) : 
    fx_downscale_(gi_lib::fx::FxScale::Parameters{}){

	auto directory = Application::GetInstance().GetDirectory();
    
	clear_shader_ = new DX11Computation(IComputation::CompileFromFile{ directory + L"Data\\shaders\\common\\clear_uint.hlsl" });

	luminance_shader_ = new DX11Computation(IComputation::CompileFromFile{ directory + L"Data\\Shaders\\luminance_histogram.hlsl" });

	log_luminance_histogram_ = new DX11ScratchStructuredArray(IScratchStructuredArray::FromElementSize{ kBinCount, 
																										sizeof(unsigned int) });

    rt_cache_ = std::make_unique<DX11RenderTargetCache>(IRenderTargetCache::Singleton{});

	// One-time setup

	clear_shader_->SetOutput(Tag("gBuffer"),
							 ObjectPtr<IScratchStructuredArray>(log_luminance_histogram_));

	luminance_parameters_ = new DX11StructuredBuffer(sizeof(ShaderParameters));

	luminance_shader_->SetInput(kParameters,
								ObjectPtr<IStructuredBuffer>(luminance_parameters_));

	luminance_shader_->SetOutput(kHistogram,
								 ObjectPtr<IScratchStructuredArray>(log_luminance_histogram_));

	SetMinLuminance(parameters.min_luminance_);
	SetMaxLuminance(parameters.max_luminance_);
	SetLowPercentage(parameters.low_percentage_);
	SetHighPercentage(parameters.high_percentage_);

    downscale_ = parameters.downscale_;

}

float DX11FxLuminance::ComputeAverageLuminance(const ObjectPtr<ITexture2D>& source) const {

	auto& graphics_ = DX11Graphics::GetInstance();

	graphics_.PushEvent(L"Average luminance");

	auto context = DX11Graphics::GetInstance().GetContext().GetImmediateContext();

	auto width = source->GetWidth();
	auto height = source->GetHeight();

	// Clear the histogram first

	graphics_.PushEvent(L"Clear");

	clear_shader_->Dispatch(*context, 
							kBinCount, 
							1, 
							1);

	graphics_.PopEvent();



    // Downscaling
    
    graphics_.PushEvent(L"Downscaling");

    ObjectPtr<ITexture2D> source_texture = source;   // Texture whose average luminance needs to be computed

    ObjectPtr<IRenderTarget> downscaled_texture = nullptr;
    ObjectPtr<IRenderTarget> temp;

    for (unsigned int index = 0; index < downscale_; ++index){

        width = std::max(width >> 1, 2u);
        height = std::max(height >> 1, 2u);

        temp = downscaled_texture;
                
        downscaled_texture = rt_cache_->PopFromCache(width, height, { source->GetFormat() }, false);

        fx_downscale_.Copy(source_texture,
                           downscaled_texture);

        source_texture = (*downscaled_texture)[0];
        
        if (temp){

            rt_cache_->PushToCache(temp);

        }
        
    }

    graphics_.PopEvent();
    
	// Compute the image histogram on the downscaled surface

	luminance_shader_->SetInput(kSourceTexture,
								source_texture);

	luminance_shader_->Dispatch(*context,
								width,
								height,
								1);
	
	graphics_.PopEvent();
    
    // Cleanup

    if (downscaled_texture){

        rt_cache_->PushToCache(downscaled_texture);

    }

	// Process the luminance histogram to find the average luminance of the image

	log_luminance_histogram_->Refresh(*context);

	int low_samples = static_cast<int>(width * height * low_percentage_);
	int high_samples = static_cast<int>(width * height * high_percentage_);

	size_t low_bin_index = 0;
	size_t high_bin_index = 0;

	size_t bins_count = log_luminance_histogram_->GetCount();

	while (low_samples > 0 && low_bin_index < bins_count) {

		low_samples -= log_luminance_histogram_->ElementAt<unsigned int>(low_bin_index);
		
		++low_bin_index;

	}

	while (high_samples > 0 && high_bin_index < bins_count) {

		high_samples -= log_luminance_histogram_->ElementAt<unsigned int>(high_bin_index);

		++high_bin_index;

	}

	float low_luminance = (static_cast<float>(--low_bin_index) / bins_count) * (max_log_luminance_ - min_log_luminance_) + min_log_luminance_;
	float high_luminance = (static_cast<float>(--high_bin_index) / bins_count) * (max_log_luminance_ - min_log_luminance_) + min_log_luminance_;
	
	// Convert to logarithmic luminance
	low_luminance = std::exp2f(low_luminance);
	high_luminance = std::exp2f(high_luminance);
        
	return (low_luminance + high_luminance) * 0.5f;

}

void DX11FxLuminance::SetMinLuminance(float min_luminance) {

	auto min_log_luminance = std::log2f(min_luminance);

	min_log_luminance_ = min_log_luminance;

	luminance_parameters_->Lock<ShaderParameters>()->gLogMinimum = min_log_luminance;

	luminance_parameters_->Unlock();

}

void DX11FxLuminance::SetMaxLuminance(float max_luminance) {

	auto max_log_luminance = std::log2f(max_luminance);

	max_log_luminance_ = max_log_luminance;

	luminance_parameters_->Lock<ShaderParameters>()->gLogMaximum = max_log_luminance;

	luminance_parameters_->Unlock();

}

void DX11FxLuminance::SetLowPercentage(float low_percentage) {

	low_percentage_ = low_percentage;

}

void DX11FxLuminance::SetHighPercentage(float high_percentage) {

	high_percentage_ = high_percentage;

}

size_t DX11FxLuminance::GetSize() const{
	
	return 0;

}
