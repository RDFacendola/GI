#pragma comment(lib,"DirectXTK")
#pragma comment(lib,"DirectXTex")

#include "dx11resources.h"

#include <set>
#include <map>
#include <unordered_map>
#include <math.h>

#include <DDSTextureLoader.h>
#include <DirectXTex.h>
#include <DirectXMath.h>
#include <Eigen/Core>

#include "..\..\include\core.h"
#include "..\..\include\enums.h"
#include "..\..\include\exceptions.h"
#include "..\..\include\scope_guard.h"
#include "..\..\include\bundles.h"
#include "..\..\include\observable.h"
#include "..\..\include\debug.h"

#include "dx11.h"
#include "dx11graphics.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::windows;

using namespace DirectX;
using namespace Eigen;

namespace{
	
	/////////////////////////// TEXTURE 2D //////////////////////////////

	/// \brief Ratio between a Bit and a Byte size.
	const float kBitOverByte = 1.0f / 8.0f;

	/// \brief Size ration between two consecutive MIP levels of a texture 2D.
	const float kMIPRatio2D = 1.0f / 4.0f;

	//////////////////////////// MESH ////////////////////////////////////

	template <typename TVertexFormat>
	Bounds VerticesToBounds(const std::vector<TVertexFormat> & vertices){

		if (vertices.size() == 0){

			return Bounds{ Vector3f::Zero(), Vector3f::Zero() };

		}

		Vector3f min_corner;
		Vector3f max_corner;

		min_corner = vertices[0].position;
		max_corner = vertices[0].position;

		for (auto & vertex : vertices){

			// Find maximum and minimum coordinates for each axis independently

			for (int coordinate = 0; coordinate < 3; ++coordinate){

				if (min_corner(coordinate) > vertex.position(coordinate)){

					min_corner(coordinate) = vertex.position(coordinate);

				}
				else if (max_corner(coordinate) < vertex.position(coordinate)){

					max_corner(coordinate) = vertex.position(coordinate);

				}

			}

		}

		return Bounds{ 0.5f * (max_corner + min_corner),
					   max_corner - min_corner };

	}

	/////////////////////////// MATERIAL ////////////////////////////////

	/// \brief Bundle of shader resources that will be bound to the pipeline.
	struct ShaderBundle{

		vector<ID3D11Buffer*> buffers;						/// \brief Buffer binding.

		vector<ID3D11ShaderResourceView*> resources;		/// \brief Resource binding.

		unsigned int sampler_count;							/// \brief Number of sampler needed by the shader. The actual sampler is defined by the renderer.

		/// \brief Default constructor;
		ShaderBundle();

		/// \brief Move constructor.
		ShaderBundle(ShaderBundle&& other);

	};

	/// \brief Describes the current status of a buffer.
	class BufferStatus{

	public:

		/// \brief Create a new buffer status.
		/// \param device Device used to create the constant buffer.
		/// \param size Size of the buffer to create.
		BufferStatus(ID3D11Device& device, size_t size);

		/// \brief Copy constructor
		/// \param Instance to copy.
		BufferStatus(const BufferStatus& other);

		/// \brief Move constructor.
		/// \param Instance to move.
		BufferStatus(BufferStatus&& other);

		/// \brief No assignment operator.
		BufferStatus& operator=(const BufferStatus&) = delete;

		/// \brief Destructor.
		~BufferStatus();

		/// \brief Write inside the constant buffer.
		/// \param source Buffer to read from.
		/// \param size Size of the buffer to read in bytes.
		/// \param offset Offset from the beginning of the constant buffer in bytes.
		void Write(const void * source, size_t size, size_t offset);

		/// \brief Get the hardware buffer reference.
		ID3D11Buffer& GetBuffer();

		/// \brief Get the hardware buffer reference.
		const ID3D11Buffer& GetBuffer() const;

	private:

		unique_ptr<ID3D11Buffer, COMDeleter> buffer_;		/// \brief Constant buffer to bound to the graphic pipeline.

		void * data_;										/// \brief Buffer containing the data to send to the constant buffer.

		bool dirty_;										/// \brief Whether the constant buffer should be updated.

		size_t size_;										/// \brief Size of the buffer in bytes.

	};

	//------------------------- SHADER BUNDLE -------------------------//

	ShaderBundle::ShaderBundle(){}

	ShaderBundle::ShaderBundle(ShaderBundle&& other){

		buffers = std::move(other.buffers);
		resources = std::move(other.resources);
		sampler_count = other.sampler_count;

	}

	//------------------------- BUFFER STATUS -------------------------//

	BufferStatus::BufferStatus(ID3D11Device& device, size_t size){

		ID3D11Buffer * buffer;

		THROW_ON_FAIL(MakeConstantBuffer(device,
										 size,
										 &buffer));

		buffer_.reset(buffer);

		data_ = new char[size];

		size_ = size;

		dirty_ = false;

	}

	BufferStatus::BufferStatus(const BufferStatus& other){

		ID3D11Device* device;
		ID3D11Buffer * buffer;

		other.buffer_->GetDevice(&device);

		COM_GUARD(device);

		THROW_ON_FAIL(MakeConstantBuffer(*device,
										 other.size_,
										 &buffer));

		buffer_.reset(buffer);

		data_ = new char[other.size_];

		size_ = other.size_;

		dirty_ = true;			// lazily update the constant buffer.

		memcpy_s(data_,			// copy the current content of the buffer.
				 size_, 
				 other.data_, 
				 other.size_);

	}

	BufferStatus::BufferStatus(BufferStatus&& other){

		buffer_ = std::move(other.buffer_);

		data_ = other.data_;
		other.data_ = nullptr;

		size_ = other.size_;

		dirty_ = other.dirty_;

	}

	BufferStatus::~BufferStatus(){

		if (data_){

			delete[] data_;

		}

	}

	void BufferStatus::Write(const void * source, size_t size, size_t offset){

		memcpy_s(static_cast<char*>(data_) + offset,
				 size_ - offset,
				 source,
				 size);

		dirty_ = true;

	}

	ID3D11Buffer& BufferStatus::GetBuffer(){

		return *buffer_;

	}

	const ID3D11Buffer& BufferStatus::GetBuffer() const{

		return *buffer_;

	}

}

////////////////////////////// TEXTURE 2D //////////////////////////////////////////

DX11Texture2D::DX11Texture2D(ID3D11Device & device, const LoadFromFile& bundle){
	
	DDS_ALPHA_MODE alpha_mode;
	ID3D11Resource * resource;
	ID3D11ShaderResourceView * shader_view;

	wstringstream file_name;

	file_name << Application::GetInstance().GetDirectory() << bundle.file_name;

	THROW_ON_FAIL( CreateDDSTextureFromFileEx(&device, 
											  file_name.str().c_str(), 
											  0,									// Load everything.
											  D3D11_USAGE_IMMUTABLE, 
											  D3D11_BIND_SHADER_RESOURCE, 
											  0,									// No CPU access.
											  0,
											  false,								// No forced sRGB
											  &resource,
											  &shader_view, 
											  &alpha_mode) );						//Alpha informations

	texture_.reset(static_cast<ID3D11Texture2D*>(resource));	
	shader_view_.reset(shader_view, COMDeleter{});

	UpdateDescription();
	
}

DX11Texture2D::DX11Texture2D(ID3D11Texture2D & texture, DXGI_FORMAT format){

	ID3D11Device * device;

	texture.GetDevice(&device);

	COM_GUARD(device);

	ID3D11ShaderResourceView * shader_view;

	D3D11_TEXTURE2D_DESC texture_desc;

	texture.GetDesc(&texture_desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc;

	view_desc.Format = (format == DXGI_FORMAT_UNKNOWN) ? texture_desc.Format : format;
	view_desc.ViewDimension = (texture_desc.SampleDesc.Count == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
	view_desc.Texture2D.MostDetailedMip = 0;
	view_desc.Texture2D.MipLevels = texture_desc.MipLevels;

	THROW_ON_FAIL(device->CreateShaderResourceView(reinterpret_cast<ID3D11Resource *>(&texture),
												   &view_desc,
												   &shader_view));

	texture_.reset(&texture);
	shader_view_.reset(shader_view, COMDeleter{});

	UpdateDescription();

}

size_t DX11Texture2D::GetSize() const{

	auto level_size = width_ * height_ * bits_per_pixel_ * kBitOverByte;	//Size of the most detailed level.

	// MIP map footprint -> Sum of a geometrical serie...

	return static_cast<size_t>( level_size * ((1.0f - std::powf(kMIPRatio2D, static_cast<float>(mip_levels_))) / (1.0f - kMIPRatio2D)) );

}

void DX11Texture2D::UpdateDescription(){
	
	D3D11_TEXTURE2D_DESC description;

	texture_->GetDesc(&description);

	width_ = description.Width;
	height_ = description.Height;
	mip_levels_ = description.MipLevels;
	bits_per_pixel_ = static_cast<unsigned int>(BitsPerPixel(description.Format));

}

///////////////////////////// RENDER TARGET ///////////////////////////////////////

DX11RenderTarget::DX11RenderTarget(ID3D11Texture2D & target){

	SetBuffers({ &target });

}

void DX11RenderTarget::SetBuffers(std::initializer_list<ID3D11Texture2D*> targets){

	/// The render target view format and the shader resource view format for the render targets are the same of the textures they are generated from (DXGI_FORMAT_UNKNOWN).
	/// The depth stencil texture is created with a 24bit channel for the depth and a 8bit channel for the stencil, both without a type (DXGI_FORMAT_R24G8_TYPELESS).
	/// The depth stencil view format of the depth stencil texture is 24bit uniform for the depth and 8bit unsigned int for the stencil (DXGI_FORMAT_D24_UNORM_S8_UINT).
	/// The shader resource view of the depth stencil texture is 24bit uniform for the depth. The stencil cannot be sampled inside the shader (DXGI_FORMAT_R24_UNORM_X8_TYPELESS).
	
	ResetBuffers();

	ID3D11Device * device;
	
	ID3D11Texture2D * zstencil;
	D3D11_TEXTURE2D_DESC desc;

	ID3D11RenderTargetView * render_target_view;

	ID3D11DepthStencilView * zstencil_view;

	auto& target = **targets.begin();

	// Rollback guard ensures that the state of the render target is cleared on error
	// (ie: if one buffer causes an exception, the entire operation is rollback'd)

	auto rollback = make_scope_guard([this](){
	
		textures_.clear();
		target_views_.clear();

		zstencil_ = nullptr;
		zstencil_view_ = nullptr;
	
	});

	target.GetDevice(&device);

	COM_GUARD(device);

	for (auto target : targets){
		
		THROW_ON_FAIL(device->CreateRenderTargetView(reinterpret_cast<ID3D11Resource *>(target),
													 nullptr,
													 &render_target_view));

		textures_.push_back(make_shared<DX11Texture2D>(*target, DXGI_FORMAT_UNKNOWN));
		target_views_.push_back(std::move(unique_ptr<ID3D11RenderTargetView, COMDeleter>(render_target_view, COMDeleter{})));

	}

	// Create the z-stencil and the z-stencil view
		
	target.GetDesc(&desc);

	MakeDepthStencil(*device, desc.Width, desc.Height, &zstencil, &zstencil_view);

	zstencil_ = make_shared<DX11Texture2D>(*zstencil, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);				// This is the only format compatible with R24G8_TYPELESS used to create the depth buffer resource
	zstencil_view_ = unique_ptr<ID3D11DepthStencilView, COMDeleter>(zstencil_view, COMDeleter{});

	// Everything went as it should have...
	rollback.Dismiss();

}

void DX11RenderTarget::ResetBuffers(){

	textures_.clear();
	target_views_.clear();
	zstencil_ = nullptr;
	zstencil_view_ = nullptr;

}

void DX11RenderTarget::Bind(ID3D11DeviceContext & context){

	// Actual array of render target views.

	vector<ID3D11RenderTargetView *> target_view_array(target_views_.size());

	std::transform(target_views_.begin(),
		target_views_.end(),
		target_view_array.begin(),
		[](unique_ptr<ID3D11RenderTargetView, COMDeleter> & target_view){

			return target_view.get();

		});

	context.OMSetRenderTargets(static_cast<unsigned int>(target_view_array.size()),
		&target_view_array[0],
		zstencil_view_.get());

}

void DX11RenderTarget::ClearDepthStencil(ID3D11DeviceContext & context, unsigned int clear_flags, float depth, unsigned char stencil){

	context.ClearDepthStencilView(zstencil_view_.get(), clear_flags, depth, stencil);
	
}

void DX11RenderTarget::ClearTargets(ID3D11DeviceContext & context, Color color){

	// The color is ARGB, however the method ClearRenderTargetView needs an RGBA.

	float rgba_color[4];

	rgba_color[0] = color.color.red;
	rgba_color[1] = color.color.green;
	rgba_color[2] = color.color.blue;
	rgba_color[3] = color.color.alpha;

	for (auto & rt_view : target_views_){

		context.ClearRenderTargetView(rt_view.get(), rgba_color);

	}

}

///////////////////////////// MESH ////////////////////////////////////////////////

DX11Mesh::DX11Mesh(ID3D11Device& device, const BuildIndexedNormalTextured& bundle){

	// Normal, textured mesh.

	size_t vb_size = bundle.vertices.size() * sizeof(VertexFormatNormalTextured);
	size_t ib_size = bundle.indices.size() * sizeof(unsigned int);
	
	ID3D11Buffer * buffer;
	
	THROW_ON_FAIL(MakeVertexBuffer(device,
								   &(bundle.vertices[0]),
								   vb_size,
								   &buffer));

	vertex_buffer_.reset(buffer);

	if (bundle.indices.size() > 0){

		THROW_ON_FAIL(MakeIndexBuffer(device, 
									  &(bundle.indices[0]), 
									  ib_size,
									  &buffer));
	
		index_buffer_.reset(buffer);

		polygon_count_ = bundle.indices.size();

	}
	else{

		polygon_count_ = bundle.vertices.size() / 3;

	}

	vertex_count_ = bundle.vertices.size();
	LOD_count_ = 1;
	size_ = vb_size + ib_size;

	bounds_ = VerticesToBounds(bundle.vertices);

}

////////////////////////////// MATERIAL //////////////////////////////////////////////

/// \brief Private implementation of DX11Material.
struct DX11Material::InstanceImpl{

	unordered_map<ShaderType, ShaderBundle> bundles;					///< \brief Bundles of resources bound to shaders.
	
	InstanceImpl(ID3D11Device& device, const ShaderReflection& reflection);

	InstanceImpl(const InstanceImpl& impl);

	/// \brief No assignment operator.
	InstanceImpl& operator=(const InstanceImpl&) = delete;
	
	void SetVariable(size_t index, const void * data, size_t size, size_t offset);

	void SetResource(size_t index, shared_ptr<IBindable> resource);
	
private:

	void AddBundle(ShaderType shader_type);

	void UpdateResourceViews();

	vector<BufferStatus> buffer_status_;								///< \brief Status of constant buffers.

	vector<shared_ptr<IBindable>> resources_;							///< \brief Status of bound resources.

	ShaderType resource_dirty_mask_;									///< \brief Dirty mask used to determine which bundle needs to be updated resource-wise.

	const ShaderReflection& reflection_;
		
};

/// \brief Shared implementation of DX11Material.
struct DX11Material::MaterialImpl{

	ShaderReflection reflection;													/// \brief Combined reflection of the shaders.

	BlendMode blend_mode;															/// \brief Blend mode.

	unordered_map<ShaderType, unique_ptr<ID3D11DeviceChild, COMDeleter>> shaders;	/// \brief Shader objects.

	MaterialImpl(ID3D11Device& device, const CompileFromFile& bundle);
	
private:

	template <typename TShader>
	void MakeShader(ID3D11Device& device, const string& code, const string& file_name, bool mandatory);

};

//----------------------------  MATERIAL :: INSTANCE IMPL -------------------------------//

DX11Material::InstanceImpl::InstanceImpl(ID3D11Device& device, const ShaderReflection& reflection) :
reflection_(reflection){

	// Buffer status
	for (auto& buffer : reflection.buffers){

		buffer_status_.push_back(BufferStatus(device, buffer.size));

	}

	// Resource status (empty)
	resources_.resize(reflection.resources.size());

	// Bundles
	AddBundle(ShaderType::VERTEX_SHADER);
	AddBundle(ShaderType::HULL_SHADER);
	AddBundle(ShaderType::DOMAIN_SHADER);
	AddBundle(ShaderType::GEOMETRY_SHADER);
	AddBundle(ShaderType::PIXEL_SHADER);

	resource_dirty_mask_ = ShaderType::NONE;

}

DX11Material::InstanceImpl::InstanceImpl(const InstanceImpl& impl) :
buffer_status_(impl.buffer_status_),				// Copy ctor will copy the buffer status
resources_(impl.resources_),						// Same resources bound
resource_dirty_mask_(ShaderType::ALL),				// Used to lazily update the resources' shader views
reflection_(impl.reflection_){

	// Bundles
	AddBundle(ShaderType::VERTEX_SHADER);
	AddBundle(ShaderType::HULL_SHADER);
	AddBundle(ShaderType::DOMAIN_SHADER);
	AddBundle(ShaderType::GEOMETRY_SHADER);
	AddBundle(ShaderType::PIXEL_SHADER);

}

void DX11Material::InstanceImpl::SetVariable(size_t index, const void * data, size_t size, size_t offset){

	buffer_status_[index].Write(data, size, offset);

}

void DX11Material::InstanceImpl::SetResource(size_t index, shared_ptr<IBindable> resource){

	resources_[index] = resource;

	resource_dirty_mask_ |= reflection_.resources[index].shader_usage;	// Let the bundle know that the resource status changed.

}

void DX11Material::InstanceImpl::AddBundle(ShaderType shader_type){

	if (reflection_.shaders && shader_type){
		
		ShaderBundle bundle;

		// Buffers, built once, updated automatically.
		for (int buffer_index = 0; buffer_index < buffer_status_.size(); ++buffer_index){

			if (reflection_.buffers[buffer_index].shader_usage && shader_type){

				bundle.buffers.push_back(&buffer_status_[buffer_index].GetBuffer());

			}

		}

		// Resources, built once, update by need (when the material is bound to the pipeline).
		bundle.resources.resize(std::count_if(reflection_.resources.begin(),
			reflection_.resources.end(),
			[shader_type](const ShaderResourceDesc& resource_desc){

			return resource_desc.shader_usage && shader_type;

		}));

		// Sampler count, never changes
		bundle.sampler_count = static_cast<unsigned int>(std::count_if(reflection_.samplers.begin(),
			reflection_.samplers.end(),
			[shader_type](const ShaderSamplerDesc& sampler_desc){

			return sampler_desc.shader_usage && shader_type;

		}));

		bundles[shader_type] = std::move(bundle);

	}

}

void DX11Material::InstanceImpl::UpdateResourceViews(){

	size_t resource_index;
	size_t bind_point;

	for (auto& bundle : bundles){

		if (resource_dirty_mask_ && bundle.first){		// Dirty bundles only.

			auto& resource_views = bundle.second.resources;

			for (resource_index = 0, bind_point = 0; resource_index < resource_views.size(); ++resource_index){

				if (reflection_.resources[resource_index].shader_usage && bundle.first){

					resource_views[bind_point] = &resource_view(*resources_[resource_index]);	// Get the Sampler State from the resource pointer.

					++bind_point;

				}

			}

		}

	}

	resource_dirty_mask_ = ShaderType::NONE;

}

//----------------------------  MATERIAL :: MATERIAL IMPL -------------------------------//

DX11Material::MaterialImpl::MaterialImpl(ID3D11Device& device, const CompileFromFile& bundle){
	
	string code = IO::ReadFile(bundle.file_name);

	string file_name = string(bundle.file_name.begin(), bundle.file_name.end());

	auto rollback = make_scope_guard([&](){

		shaders.clear();

	});

	// Shaders

	reflection.shaders = ShaderType::NONE;

	MakeShader<ID3D11VertexShader>(device, code, file_name, true);		// mandatory
	MakeShader<ID3D11HullShader>(device, code, file_name, false);		// optional
	MakeShader<ID3D11DomainShader>(device, code, file_name, false);		// optional
	MakeShader<ID3D11GeometryShader>(device, code, file_name, false);	// optional
	MakeShader<ID3D11PixelShader>(device, code, file_name, true);		// mandatory
				
	// Blend mode

	TODO("Add support to other blend modes here.");

	blend_mode = BlendMode::Opaque;
	
	// Dismiss
	rollback.Dismiss();

}

template <typename TShader>
void DX11Material::MaterialImpl::MakeShader(ID3D11Device& device, const string& code, const string& file_name, bool mandatory){

	const wstring entry_point_exception_code = L"X3501";	// Exception code thrown when the entry point could not be found (ie: a shader doesn't exists).

	TShader* shader;

	wstring errors;

	if (FAILED(::MakeShader(device, 
							code,
							file_name, 
							&shader, 
							&reflection, 
							&errors))){

		// Throws only if the compilation process fails when the entry point is found (on syntax error). 
		// If the entry point is not found, throws only if the shader presence was mandatory.

		if(mandatory ||
		   errors.find(entry_point_exception_code) == wstring::npos){

			THROW(errors);	

		}

	}
	else{

		shaders[ShaderTraits<TShader>::flag] = unique_com(shader);

	}

}

//----------------------------  MATERIAL :: VARIABLE -------------------------------//

DX11Material::Variable::Variable(InstanceImpl& instance_impl, size_t buffer_index, size_t variable_size, size_t variable_offset) :
instance_impl_(&instance_impl),
buffer_index_(buffer_index),
variable_size_(variable_size),
variable_offset_(variable_offset){}

void DX11Material::Variable::Set(const void * buffer, size_t size){

	if (size > variable_size_){

		THROW(L"Wrong variable size.");

	}

	instance_impl_->SetVariable(buffer_index_, buffer, size, variable_offset_);

}

//----------------------------  MATERIAL :: RESOURCE -------------------------------//

DX11Material::Resource::Resource(InstanceImpl& instance_impl, size_t resource_index) :
instance_impl_(&instance_impl),
resource_index_(resource_index){}

void DX11Material::Resource::Set(shared_ptr<IBindable> resource){

	instance_impl_->SetResource(resource_index_, resource);

}

//----------------------------  MATERIAL -------------------------------//

DX11Material::DX11Material(ID3D11Device& device, const CompileFromFile& bundle){

	shared_impl_ = make_shared<MaterialImpl>(device, bundle);

	private_impl_ = make_unique<InstanceImpl>(device, shared_impl_->reflection);

}

DX11Material::DX11Material(ID3D11Device&, const InstantiateFromMaterial& bundle){

	auto& material = resource_cast(*bundle.base);
	
	shared_impl_ = material.shared_impl_;

	private_impl_ = make_unique<InstanceImpl>(*material.private_impl_);
	
}

DX11Material::~DX11Material(){

	private_impl_ = nullptr;	// Must be destroyed before the shared implementation!
	shared_impl_ = nullptr;

}

shared_ptr<Material::Variable> DX11Material::GetVariable(const string& name){

	auto& buffers = shared_impl_->reflection.buffers;

	size_t buffer_index = 0;

	// O(#total variables)

	for (auto& buffer : buffers){

		auto it = std::find_if(buffer.variables.begin(),
							   buffer.variables.end(),
							   [&name](const ShaderVariableDesc& desc){

									return desc.name == name;

							   });

		if (it != buffer.variables.end()){

			return make_shared<DX11Material::Variable>(*private_impl_,
													   buffer_index,
													   it->size,
													   it->offset);

		}

		++buffer_index;

	}
		
	THROW(L"Could not find the specified shader variable.");

}

shared_ptr<Material::Resource> DX11Material::GetResource(const string& name){

	auto& resources = shared_impl_->reflection.resources;

	// O(#total resources)

	auto it = std::find_if(resources.begin(),
						   resources.end(),
						   [&name](const ShaderResourceDesc& desc){

								return desc.name == name;

						   });

	if (it == resources.end()){

		THROW(L"Could not find the specified shader resource.");

	}

	return make_shared<DX11Material::Resource>(*private_impl_,
											   std::distance(resources.begin(),
															 it));

}

size_t DX11Material::GetSize() const{

	auto& buffers = shared_impl_->reflection.buffers;

	return std::accumulate(buffers.begin(),
						   buffers.end(),
						   static_cast<size_t>(0),
						   [](size_t size, const ShaderBufferDesc& desc){

								return size + desc.size;

						   });

}

BlendMode DX11Material::GetBlendMode() const{

	return shared_impl_->blend_mode;

}