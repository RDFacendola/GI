#pragma comment(lib,"DirectXTK")
#pragma comment(lib,"DirectXTex")
#pragma comment(lib,"dxguid.lib")

#include "dx11resources.h"

#include <set>
#include <map>
#include <unordered_map>
#include <math.h>

#include <DDSTextureLoader.h>
#include <DirectXTex.h>
#include <DirectXMath.h>
#include <Eigen/Core>

#include "..\..\include\gimath.h"
#include "..\..\include\core.h"
#include "..\..\include\enums.h"
#include "..\..\include\exceptions.h"
#include "..\..\include\scope_guard.h"
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
	AABB VerticesToBounds(const std::vector<TVertexFormat> & vertices){

		if (vertices.size() == 0){

			return AABB{ Vector3f::Zero(), Vector3f::Zero() };

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

		return AABB{ 0.5f * (max_corner + min_corner),
					   max_corner - min_corner };

	}

	/////////////////////////// MATERIAL ////////////////////////////////

	/// \brief Bundle of shader resources that will be bound to the pipeline.
	struct ShaderBundle{

		vector<ID3D11Buffer*> buffers;						/// \brief Buffer binding.

		vector<ID3D11ShaderResourceView*> resources;		/// \brief Resource binding.

		vector<ID3D11SamplerState*> samplers;				/// \brief Sampler binding.

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

		/// \brief Write a value inside the constant buffer
		/// \param value The value to write.
		/// \param offset Offset from the beginning of the constant buffer in bytes.
		/// \remarks This method should have better performances while working with smaller values.
		template <typename TType>
		void Write(const TType& value, size_t offset);

		/// \brief Commit any uncommitted change to the constant buffer.
		void Commit(ID3D11DeviceContext& context);

		/// \brief Get the constant buffer.
		/// \return Return the constant buffer.
		ID3D11Buffer& GetBuffer();

		/// \brief Get the constant buffer.
		/// \return Return the constant buffer.
		const ID3D11Buffer& GetBuffer() const;

	private:

		unique_ptr<ID3D11Buffer, COMDeleter> buffer_;		/// \brief Constant buffer to bound to the graphic pipeline.

		void * data_;										/// \brief Buffer containing the data to send to the constant buffer.

		bool dirty_;										/// \brief Whether the constant buffer should be updated.

		size_t size_;										/// \brief Size of the buffer in bytes.

	};

	/////////////////////////// SHADER BUNDLE ///////////////////////////

	ShaderBundle::ShaderBundle(){}

	ShaderBundle::ShaderBundle(ShaderBundle&& other){

		buffers = std::move(other.buffers);
		resources = std::move(other.resources);
		samplers = std::move(other.samplers);

	}

	/////////////////////////// BUFFER STATUS ///////////////////////////

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

	inline void BufferStatus::Write(const void * source, size_t size, size_t offset){

		memcpy_s(static_cast<char*>(data_) + offset,
				 size_ - offset,
				 source,
				 size);

		dirty_ = true;

	}

	template <typename TType>
	inline void BufferStatus::Write(const TType& value, size_t offset){

		*reinterpret_cast<TType*>((static_cast<char*>(data_)+offset)) = value;

		dirty_ = true;

	}

	void BufferStatus::Commit(ID3D11DeviceContext& context){

		if (dirty_){

			D3D11_MAPPED_SUBRESOURCE mapped_buffer;

			context.Map(buffer_.get(),			
						0,
						D3D11_MAP_WRITE_DISCARD,		// Discard the previous buffer
						0,					
						&mapped_buffer);

			memcpy_s(mapped_buffer.pData,
					 size_,
					 data_,
					 size_);
			
			context.Unmap(buffer_.get(),
						  0);

			dirty_ = false;

		}

	}

	ID3D11Buffer& BufferStatus::GetBuffer(){

		return *buffer_;

	}

	const ID3D11Buffer& BufferStatus::GetBuffer() const{

		return *buffer_;

	}

	/////////////////////////// MISC ////////////////////////////////////

	/// \brief Convert a shader type to a numeric index.
	/// \return Returns a number from 0 to 4 if the specified type was referring to a single shader type, returns -1 otherwise.
	size_t ShaderTypeToIndex(ShaderType shader_type){

		// Sorted from the most common to the least common

		switch (shader_type){

		case ShaderType::VERTEX_SHADER:		

			return 0;

		case ShaderType::PIXEL_SHADER:

			return 4;

		case ShaderType::GEOMETRY_SHADER:

			return 3;

		case ShaderType::HULL_SHADER:

			return 1;

		case ShaderType::DOMAIN_SHADER:

			return 2;

		default:

			return static_cast<size_t>(-1);

		}

	}

	/// \brief Convert a numeric index to a shader type.
	/// \return Returns the shader type associated to the given numeric index.
	ShaderType IndexToShaderType(size_t index){

		switch (index){

		case 0:
		
			return ShaderType::VERTEX_SHADER;

		case 4:

			return ShaderType::PIXEL_SHADER;

		case 3:

			return ShaderType::GEOMETRY_SHADER;
		
		case 1:

			return ShaderType::HULL_SHADER;

		case 2:
			
			return ShaderType::DOMAIN_SHADER;

		default:

			return ShaderType::NONE;

		}

	}

	////////////////////////// DIRECTX 11 ///////////////////////////////

	/// \brief Load a shader from code and return a pointer to the shader object.
	/// \param device Device used to load the shader.
	/// \param code HLSL code to compile.
	/// \param file_name Name of the file used to resolve the #include directives.
	/// \param mandatory Whether the compile success if compulsory or not.
	/// \return Returns a pointer to the shader object if the method succeeded.
	///			If the entry point couldn't be found the method will throw if the mandatory flag was true, or will return nullptr otherwise.
	template <typename TShader>
	unique_ptr<TShader, COMDeleter> MakeShader(ID3D11Device& device, const string& code, const string& file_name, bool mandatory, ShaderReflection& reflection){

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

			if (mandatory ||
				errors.find(entry_point_exception_code) == wstring::npos){

				THROW(errors);

			}
			else{

				return nullptr;

			}

		}
		else{

			return unique_com(shader);

		}

	}

	/// \brief Bind a shader along with its resources to a device context.
	/// \param context The context where the shader will be bound.
	/// \param shader Pointer to the shader. Can be nullptr to disable a pipeline stage.
	/// \param bundle Bundle of resources associated to the shader.
	template <typename TShader, typename TDeleter>
	void BindShader(ID3D11DeviceContext& context, const unique_ptr<TShader, TDeleter>& shader_ptr, const ShaderBundle& bundle){

		TShader* shader = shader_ptr ?
						  shader_ptr.get() :
						  nullptr;
		// Shader

		SetShader(context,
				  shader);

		// Constant buffers
				
		SetConstantBuffers<TShader>(context,
									0,
									bundle.buffers.size() > 0 ? &bundle.buffers[0] : nullptr,
									bundle.buffers.size());

		


		// Resources

		SetShaderResources<TShader>(context,
									0,
									bundle.resources.size() > 0 ? &bundle.resources[0] : nullptr,
									bundle.resources.size());

		// Samplers

		SetShaderSamplers<TShader>(context,
								   0,
								   bundle.samplers.size() > 0 ? &bundle.samplers[0] : nullptr,
								   bundle.samplers.size());

	}
		
}

////////////////////////////// TEXTURE 2D //////////////////////////////////////////

DX11Texture2D::DX11Texture2D(const FromFile& bundle){
	
	auto& device = DX11Graphics::GetInstance().GetDevice();

	DDS_ALPHA_MODE alpha_mode;
	ID3D11Resource * resource;
	ID3D11ShaderResourceView * shader_view;


	THROW_ON_FAIL( CreateDDSTextureFromFileEx(&device, 
											  bundle.file_name.c_str(), 
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
	shader_view_.reset(shader_view);

	UpdateDescription();
	
}

DX11Texture2D::DX11Texture2D(ID3D11Texture2D& texture, DXGI_FORMAT format){

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
	shader_view_.reset(shader_view);

	UpdateDescription();

}

DX11Texture2D::DX11Texture2D(ID3D11Texture2D& texture, ID3D11ShaderResourceView& shader_view){
	
	texture_.reset(&texture);
	shader_view_.reset(&shader_view);

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
	format_ = description.Format;
	
}

///////////////////////////// RENDER TARGET ///////////////////////////////////////

DX11RenderTarget::DX11RenderTarget(ID3D11Texture2D & target){

	zstencil_ = nullptr;
	zstencil_view_ = nullptr;
	
	SetBuffers({ &target });

}

DX11RenderTarget::DX11RenderTarget(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format){
	
	zstencil_ = nullptr;
	zstencil_view_ = nullptr;

	Initialize(width,
			   height,
			   target_format);
	
}

DX11RenderTarget::~DX11RenderTarget(){

	ResetBuffers();

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

	auto& target = **targets.begin();

	// Rollback guard ensures that the state of the render target is cleared on error
	// (i.e.: if one buffer causes an exception, the entire operation is rollback'd)

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

		textures_.push_back(new DX11Texture2D(*target, 
											  DXGI_FORMAT_UNKNOWN));

		target_views_.push_back(render_target_view);

	}

	// Create the z-stencil and the z-stencil view
		
	target.GetDesc(&desc);

	THROW_ON_FAIL(MakeDepthStencil(*device, 
								   desc.Width, 
								   desc.Height, 
								   &zstencil, 
								   &zstencil_view_));

	zstencil_ = new DX11Texture2D(*zstencil, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);					// This is the only format compatible with R24G8_TYPELESS used to create the depth buffer resource
	
	// Everything went as it should have...
	rollback.Dismiss();

}

bool DX11RenderTarget::Resize(unsigned int width, unsigned int height){

	if (width == GetWidth() && height == GetHeight()){

		return false;

	}

	// Naive approach: discard the old targets and create the new ones

	std::vector<DXGI_FORMAT> target_format;

	for (auto&& texture : textures_){

		target_format.push_back(texture->GetFormat());

	}

	Initialize(width, 
			   height,
			   target_format);

	return true;

}

void DX11RenderTarget::ResetBuffers(){

	// Release the targets

	textures_.clear();
	
	// Release the target views

	for (auto&& target_view : target_views_){

		target_view->Release();

	}

	target_views_.clear();

	// Release the zstencil

	zstencil_ = nullptr;

	// Release the zstencil view

	if (zstencil_view_){

		zstencil_view_->Release();

		zstencil_view_ = nullptr;

	}
	
}

void DX11RenderTarget::ClearDepthStencil(ID3D11DeviceContext& context, unsigned int clear_flags, float depth, unsigned char stencil){

	context.ClearDepthStencilView(zstencil_view_, clear_flags, depth, stencil);
	
}

void DX11RenderTarget::ClearTargets(ID3D11DeviceContext& context, Color color){

	// The color is ARGB, however the method ClearRenderTargetView needs an RGBA.

	float rgba_color[4];

	rgba_color[0] = color.color.red;
	rgba_color[1] = color.color.green;
	rgba_color[2] = color.color.blue;
	rgba_color[3] = color.color.alpha;

	for (auto & target_view : target_views_){

		context.ClearRenderTargetView(target_view, 
									  rgba_color);

	}

}

void DX11RenderTarget::Bind(ID3D11DeviceContext& context){

	context.OMSetRenderTargets(static_cast<unsigned int>(target_views_.size()),
							   &target_views_[0],
							   zstencil_view_);
	
}

void DX11RenderTarget::Initialize(unsigned int width, unsigned int height, const std::vector<DXGI_FORMAT>& target_format){

	ResetBuffers();

	// If the method throws ensures that the resource is left in a clear state.
	auto&& guard = make_scope_guard([this](){

		ResetBuffers();

	});

	auto&& device = DX11Graphics::GetInstance().GetDevice();
	   
	ID3D11Texture2D* texture;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;
		
	// Create the render target surfaces.

	for (auto&& format : target_format){

		THROW_ON_FAIL(MakeRenderTarget(device,
									   width,
									   height,
 									   format,
									   &texture,
									   &rtv,
									   &srv));
		
		textures_.push_back(new DX11Texture2D(*texture,
											  *srv));

		target_views_.push_back(rtv);

	}

	// Depth stencil

	ID3D11Texture2D* zstencil;

	THROW_ON_FAIL(MakeDepthStencil(device,
								   width,
								   height,
								   &zstencil,
								   &zstencil_view_));
		
	zstencil_ = new DX11Texture2D(*zstencil, 
								  DXGI_FORMAT_R24_UNORM_X8_TYPELESS);	// This is the only format compatible with R24G8_TYPELESS used to create the depth buffer resource

	guard.Dismiss();

}

///////////////////////////// MESH ////////////////////////////////////////////////

DX11Mesh::DX11Mesh(const FromVertices<VertexFormatNormalTextured>& bundle){

	auto& device = DX11Graphics::GetInstance().GetDevice();

	// Normal, textured mesh.

	size_t vb_size = bundle.vertices.size() * sizeof(VertexFormatNormalTextured);
	size_t ib_size = bundle.indices.size() * sizeof(unsigned int);
	
	ID3D11Buffer* buffer;
	
	// Vertices

	THROW_ON_FAIL(MakeVertexBuffer(device,
								   &(bundle.vertices[0]),
								   vb_size,
								   &buffer));

	vertex_buffer_.reset(buffer);

	buffer = nullptr;

	// Indices

	if (bundle.indices.size() > 0){

		THROW_ON_FAIL(MakeIndexBuffer(device, 
									  &(bundle.indices[0]), 
									  ib_size,
									  &buffer));
	
		index_buffer_.reset(buffer);

		polygon_count_ = bundle.indices.size() / 3;

	}
	else{

		polygon_count_ = bundle.vertices.size() / 3;

	}

	subsets_ = bundle.subsets;

	vertex_count_ = bundle.vertices.size();
	LOD_count_ = 1;
	size_ = vb_size + ib_size;
	vertex_stride_ = sizeof(VertexFormatNormalTextured);

	bounding_box_ = VerticesToBounds(bundle.vertices);

}

size_t DX11Mesh::GetVertexCount() const{

	return vertex_count_;

}

size_t DX11Mesh::GetPolygonCount() const{

	return polygon_count_;

}

size_t DX11Mesh::GetLODCount() const{

	return LOD_count_;

}

size_t DX11Mesh::GetSize() const{

	return size_;

}

const AABB& DX11Mesh::GetBoundingBox() const{

	return bounding_box_;

}

size_t DX11Mesh::GetSubsetCount() const{

	return subsets_.size();

}

const MeshSubset& DX11Mesh::GetSubset(unsigned int subset_index) const{

	return subsets_[subset_index];

}

void DX11Mesh::Bind(ID3D11DeviceContext& context){

	// Only 1 vertex stream is used.

	unsigned int num_streams = 1;

	ID3D11Buffer* vertex_buffer = vertex_buffer_.get();

	unsigned int stride = static_cast<unsigned int>(vertex_stride_);

	unsigned int offset = 0;
	
	// Bind the vertex buffer

	context.IASetVertexBuffers(0,							// Start slot
							   num_streams,
							   &vertex_buffer,
							   &stride,
							   &offset);

	// Bind the index buffer

	context.IASetIndexBuffer(index_buffer_.get(),
							 DXGI_FORMAT_R32_UINT,
							 0);

}

////////////////////////////// MATERIAL //////////////////////////////////////////////

/// \brief Private implementation of DX11Material.
struct DX11Material::InstanceImpl{

	InstanceImpl(ID3D11Device& device, const ShaderReflection& reflection);

	InstanceImpl(const InstanceImpl& impl);

	/// \brief No assignment operator.
	InstanceImpl& operator=(const InstanceImpl&) = delete;
	
	void SetVariable(size_t index, const void * data, size_t size, size_t offset);

	template<typename TType>
	void SetVariable(size_t index, const TType& value, size_t offset);

	void SetResource(size_t index, ObjectPtr<DX11ResourceView> resource);
	
	/// \brief Write any uncommitted change to the constant buffers.
	void Commit(ID3D11DeviceContext& context);

	unordered_map<ShaderType, ShaderBundle> shader_bundles;				/// <\brief Shader bundles for each shader.

private:

	void AddShaderBundle(ShaderType shader_type);

	void CommitResources();

	vector<BufferStatus> buffer_status_;								///< \brief Status of constant buffers.

	vector<ObjectPtr<DX11ResourceView>> resources_;						///< \brief Status of bound resources.

	ObjectPtr<DX11Sampler> sampler_;									///< \brief Sampler.

	ShaderType resource_dirty_mask_;									///< \brief Dirty mask used to determine which bundle needs to be updated resource-wise.

	const ShaderReflection& reflection_;
		
};

/// \brief Shared implementation of DX11Material.
struct DX11Material::MaterialImpl{

	MaterialImpl(ID3D11Device& device, const CompileFromFile& bundle);

	ShaderReflection reflection;													/// \brief Combined reflection of the shaders.

	unique_ptr<ID3D11VertexShader, COMDeleter> vertex_shader;						/// \brief Pointer to the vertex shader.

	unique_ptr<ID3D11HullShader, COMDeleter> hull_shader;							/// \brief Pointer to the hull shader.

	unique_ptr<ID3D11DomainShader, COMDeleter> domain_shader;						/// \brief Pointer to the domain shader.

	unique_ptr<ID3D11GeometryShader, COMDeleter> geometry_shader;					/// \brief Pointer to the geometry shader.

	unique_ptr<ID3D11PixelShader, COMDeleter> pixel_shader;							/// \brief Pointer to the pixel shader.
	
	unique_ptr<ID3D11InputLayout, COMDeleter> input_layout;							/// \brief Vertices input layout. (Associated to the material, sigh)

};

//////////////////////////////  MATERIAL :: INSTANCE IMPL //////////////////////////////

DX11Material::InstanceImpl::InstanceImpl(ID3D11Device& device, const ShaderReflection& reflection) :
reflection_(reflection){

	// Buffer status

	for (auto& buffer : reflection.buffers){

		buffer_status_.push_back(BufferStatus(device, buffer.size));

	}

	// Resource status (empty)

	resources_.resize(reflection.resources.size());

	// Sampler state (the same)

	Resources& resources = DX11Graphics::GetInstance().GetResources();

	sampler_ = resources.Load<DX11Sampler, DX11Sampler::FromDescription>({ TextureMapping::WRAP, 16 });
	
	// Shader bundles

	AddShaderBundle(ShaderType::VERTEX_SHADER);
	AddShaderBundle(ShaderType::HULL_SHADER);
	AddShaderBundle(ShaderType::DOMAIN_SHADER);
	AddShaderBundle(ShaderType::GEOMETRY_SHADER);
	AddShaderBundle(ShaderType::PIXEL_SHADER);

	resource_dirty_mask_ = ShaderType::NONE;

}

DX11Material::InstanceImpl::InstanceImpl(const InstanceImpl& impl) :
buffer_status_(impl.buffer_status_),				// Copy ctor will copy the buffer status
resources_(impl.resources_),						// Same resources bound
sampler_(impl.sampler_),
resource_dirty_mask_(impl.reflection_.shaders),		// Used to lazily update the resources' shader views
reflection_(impl.reflection_){

	// Shader bundles

	AddShaderBundle(ShaderType::VERTEX_SHADER);
	AddShaderBundle(ShaderType::HULL_SHADER);
	AddShaderBundle(ShaderType::DOMAIN_SHADER);
	AddShaderBundle(ShaderType::GEOMETRY_SHADER);
	AddShaderBundle(ShaderType::PIXEL_SHADER);

}

void DX11Material::InstanceImpl::SetVariable(size_t index, const void * data, size_t size, size_t offset){

	buffer_status_[index].Write(data, 
								size, 
								offset);

}

template<typename TType>
void DX11Material::InstanceImpl::SetVariable(size_t index, const TType& value, size_t offset){

	buffer_status_[index].Write(value, 
								offset);

}

void DX11Material::InstanceImpl::SetResource(size_t index, ObjectPtr<DX11ResourceView> resource){

	resources_[index] = resource;

	resource_dirty_mask_ |= reflection_.resources[index].shader_usage;	// Let the bundle know that the resource status changed.

}

void DX11Material::InstanceImpl::AddShaderBundle(ShaderType shader_type){

	if (reflection_.shaders && shader_type){
		
		ShaderBundle bundle;

		// Buffers, built once, updated automatically.
		for (size_t buffer_index = 0; buffer_index < buffer_status_.size(); ++buffer_index){

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

		// Sampler never change (unless the game options change at runtime)

		for (auto&& sampler : reflection_.samplers){

			if (sampler.shader_usage && shader_type){

				bundle.samplers.push_back(&(sampler_->GetSamplerState()));

			}

		}

		// Move it to the vector.

		shader_bundles[shader_type] = std::move(bundle);

	}

}

void DX11Material::InstanceImpl::CommitResources(){

	if (resource_dirty_mask_ == ShaderType::NONE){

		// Most common case: no resource was touched.

		return;

	}

	size_t resource_index;								// Index of a resource within the global resource array.
	size_t bind_point;									// Resource bind point relative to a particular shader.
	size_t index = 0;									// Shader index

	ObjectPtr<DX11ResourceView> resource_view;			// Current resource view

	for (auto&& bundle_entry : shader_bundles){		

		// Cycle through dirty bundles

		if (resource_dirty_mask_ && bundle_entry.first){

			auto& resource_views = bundle_entry.second.resources;

			// Cycle trough every resource. If any given resource is used by the current shader type, update the shader resource view vector's corresponding location.

			for (resource_index = 0, bind_point = 0; resource_index < reflection_.resources.size(); ++resource_index){

				if (reflection_.resources[resource_index].shader_usage && bundle_entry.first){

					resource_view = resources_[resource_index];

					resource_views[bind_point] = resource_view ?
												 &resource_view->GetShaderView() :
												 nullptr;

					++bind_point;

				}

			}

		}

		++index;

	}

	resource_dirty_mask_ = ShaderType::NONE;

}

void DX11Material::InstanceImpl::Commit(ID3D11DeviceContext& context){

	// Commit dirty constant buffers

	for (auto&& buffer : buffer_status_){

		buffer.Commit(context);
			
	}

	// Commit resource views

	CommitResources();

}

//////////////////////////////  MATERIAL :: MATERIAL IMPL //////////////////////////////

DX11Material::MaterialImpl::MaterialImpl(ID3D11Device& device, const CompileFromFile& bundle){
	
	auto& file_system = FileSystem::GetInstance();

	string code = to_string(file_system.Read(bundle.file_name));

	string file_name = string(bundle.file_name.begin(), 
							  bundle.file_name.end());

	auto rollback = make_scope_guard([&](){

		vertex_shader = nullptr;
		hull_shader = nullptr;
		domain_shader = nullptr;
		geometry_shader = nullptr;
		pixel_shader = nullptr;

	});

	// Shaders

	reflection.shaders = ShaderType::NONE;

	vertex_shader = ::MakeShader<ID3D11VertexShader>(device, code, file_name, true, reflection);		// mandatory
	hull_shader = ::MakeShader<ID3D11HullShader>(device, code, file_name, false, reflection);			// optional
	domain_shader = ::MakeShader<ID3D11DomainShader>(device, code, file_name, false, reflection);		// optional
	geometry_shader = ::MakeShader<ID3D11GeometryShader>(device, code, file_name, false, reflection);	// optional
	pixel_shader = ::MakeShader<ID3D11PixelShader>(device, code, file_name, true, reflection);			// mandatory
				
	// Input layout

	ID3D11InputLayout* input_layout;

	// The bytecode is needed to validate the input layout. Genius idea...

	ID3DBlob * bytecode;

	THROW_ON_FAIL(Compile<ID3D11VertexShader>(code,
											  file_name,
											  &bytecode,
											  nullptr));

	COM_GUARD(bytecode);

	D3D11_INPUT_ELEMENT_DESC input_elements[3];

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

	THROW_ON_FAIL(device.CreateInputLayout(input_elements,
										   3,
										   bytecode->GetBufferPointer(),
										   bytecode->GetBufferSize(),
										   &input_layout));

	this->input_layout.reset(input_layout);

	// Dismiss
	rollback.Dismiss();

}

//////////////////////////////  MATERIAL :: VARIABLE //////////////////////////////

DX11Material::DX11MaterialVariable::DX11MaterialVariable(InstanceImpl& instance_impl, size_t buffer_index, size_t variable_size, size_t variable_offset) :
instance_impl_(&instance_impl),
buffer_index_(buffer_index),
variable_size_(variable_size),
variable_offset_(variable_offset){}

void DX11Material::DX11MaterialVariable::Set(const void * buffer, size_t size){

	if (size > variable_size_){

		THROW(L"Wrong variable size.");

	}

	instance_impl_->SetVariable(buffer_index_, 
								buffer, 
								size, 
								variable_offset_);

}

//////////////////////////////  MATERIAL :: RESOURCE //////////////////////////////

DX11Material::DX11MaterialResource::DX11MaterialResource(InstanceImpl& instance_impl, size_t resource_index) :
instance_impl_(&instance_impl),
resource_index_(resource_index){}

void DX11Material::DX11MaterialResource::Set(ObjectPtr<IResourceView> resource){

	instance_impl_->SetResource(resource_index_, 
								resource_cast(resource));

}

//////////////////////////////  MATERIAL //////////////////////////////

DX11Material::DX11Material(const CompileFromFile& args){

	auto& device = DX11Graphics::GetInstance().GetDevice();

	shared_impl_ = make_shared<MaterialImpl>(device, args);

	private_impl_ = make_unique<InstanceImpl>(device, shared_impl_->reflection);

}

DX11Material::DX11Material(const Instantiate& args){

	auto material = resource_cast(args.base);
	
	shared_impl_ = material->shared_impl_;

	private_impl_ = make_unique<InstanceImpl>(*material->private_impl_);
	
}

DX11Material::~DX11Material(){

	private_impl_ = nullptr;	// Must be destroyed before the shared implementation!
	shared_impl_ = nullptr;

}

ObjectPtr<Material::MaterialVariable> DX11Material::GetVariable(const string& name){

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

			return new DX11MaterialVariable(*private_impl_,
											buffer_index,
											it->size,
											it->offset);

		}

		++buffer_index;

	}
		
	return nullptr;

}

ObjectPtr<Material::MaterialResource> DX11Material::GetResource(const string& name){

	auto& resources = shared_impl_->reflection.resources;

	if (resources.size() > 0){
	
		// O(#total resources)

		auto it = std::find_if(resources.begin(),
							   resources.end(),
							   [&name](const ShaderResourceDesc& desc){

									return desc.name == name;

							   });

		if (it != resources.end()){

			return new DX11MaterialResource(*private_impl_,
											std::distance(resources.begin(),
														  it));
			
		}

	}

	return nullptr;

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

void DX11Material::Commit(ID3D11DeviceContext& context){

	// Update the constant buffers

	private_impl_->Commit(context);

	// Set the vertex input layout

	context.IASetInputLayout(shared_impl_->input_layout.get());

	// Bind Every shader to the pipeline

	auto& bundles = private_impl_->shader_bundles;

	BindShader(context,
			   shared_impl_->vertex_shader,
			   bundles[ShaderType::VERTEX_SHADER]);

	BindShader(context,
			   shared_impl_->hull_shader,
			   bundles[ShaderType::HULL_SHADER]);

	BindShader(context,
			   shared_impl_->domain_shader,
			   bundles[ShaderType::DOMAIN_SHADER]);

	BindShader(context,
			   shared_impl_->geometry_shader,
			   bundles[ShaderType::GEOMETRY_SHADER]);

	BindShader(context,
			   shared_impl_->pixel_shader,
			   bundles[ShaderType::PIXEL_SHADER]);

}

//////////////////////////////// DIRECTX11 SAMPLER ////////////////////////////////

size_t DX11Sampler::FromDescription::GetCacheKey() const{

	// | ... | texture_mapping | anisotropy_level |
	//      40                 8                  0

	return (anisotropy_level & 0xFF) | (static_cast<unsigned int>(texture_mapping) << 8);

}

DX11Sampler::DX11Sampler(const FromDescription& description){

	ID3D11SamplerState* sampler_state;

	THROW_ON_FAIL(MakeSampler(DX11Graphics::GetInstance().GetDevice(),
							  description.texture_mapping,
							  description.anisotropy_level,
							  &sampler_state));

	sampler_state_.reset(sampler_state);

}

////////////////////////////// DX11 STRUCTURED VECTOR //////////////////////////////////

DX11StructuredVector::DX11StructuredVector(const FromDescription& args) :
element_count_(args.element_count),
element_size_(args.element_size),
dirty_(false){

	ID3D11Buffer* buffer;
	ID3D11ShaderResourceView* shader_view;

	THROW_ON_FAIL(MakeStructuredBuffer(DX11Graphics::GetInstance().GetDevice(),
									   static_cast<unsigned int>(element_count_),
									   static_cast<unsigned int>(element_size_),
									   true,
									   &buffer,
									   &shader_view,
									   nullptr));

	data_ = new char[element_size_ * element_count_];

	buffer_.reset(buffer);
	shader_view_.reset(shader_view);
	
}

DX11StructuredVector::~DX11StructuredVector(){

	if (data_){

		delete[] data_;

	}
	
}

void DX11StructuredVector::Unlock()
{
	
	dirty_ = true;

}

void DX11StructuredVector::Unmap(ID3D11DeviceContext& context){
	
	context.Unmap(buffer_.get(),
				  0);							// Unmap everything.

	dirty_ = false;

}

void DX11StructuredVector::Commit(ID3D11DeviceContext& context){

	if (dirty_){

		auto size = static_cast<unsigned int>(element_count_ * element_size_);
		
		memcpy_s(Map<void>(context),
				 size,
				 data_,
				 size);
			
		Unmap(context);


	}

}

void* DX11StructuredVector::LockDiscard()
{

	if (dirty_){

		THROW(L"The buffer is already locked. Be sure to unlock the buffer before locking it again.");

	}

	return data_;

}
