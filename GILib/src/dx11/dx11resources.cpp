#pragma comment(lib,"DirectXTK")
#pragma comment(lib,"DirectXTex")

#include "dx11resources.h"

#include <set>
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

#include "dx11.h"

using namespace std;
using namespace gi_lib;
using namespace gi_lib::dx11;
using namespace gi_lib::windows;

using namespace DirectX;
using namespace Eigen;

namespace{

	/// \brief Ratio between a Bit and a Byte size.
	const float kBitOverByte = 1.0f / 8.0f;

	/// \brief Size ration between two consecutive MIP levels of a texture 2D.
	const float kMIPRatio2D = 1.0f / 4.0f;
		
	/// \brief Describes the current status of a buffer.
	class BufferStatus{

	public:

		/// \brief Create a new buffer status.
		/// \param device Device used to create the constant buffer.
		/// \param size Size of the buffer to create.
		BufferStatus(ID3D11Device& device, size_t size);

		/// \brief No copy constructor.
		BufferStatus(const BufferStatus&) = delete;

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
		void Write(void * source, size_t size, size_t offset);

	private:

		unique_ptr<ID3D11Buffer, COMDeleter> buffer_;		/// \brief Constant buffer to bound to the graphic pipeline.

		void * data_;										/// \brief Buffer containing the data to send to the constant buffer.

		bool dirty_;										/// \brief Whether the constant buffer should be updated.

		size_t size_;										/// \brief Size of the buffer in bytes.

	};

	/// \brief Describes the current status of a shader.
	class ShaderStatus{

	public:

		ShaderStatus();

	private:

		vector<ID3D11Buffer*> buffers_;						/// \brief Buffer binding status.

		vector<ID3D11ShaderResourceView*> resources_;		/// \brief Resource binding status.
		
		vector<ID3D11SamplerState*> samplers_;				/// \brief Sampler binding status.

	};
	
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

	void BufferStatus::Write(void * source, size_t size, size_t offset){

		memcpy_s(static_cast<char*>(data_) + offset,
				 size_ - offset,
				 source,
				 size);

		dirty_ = true;

	}

	////////////////////////// SHADER SETUP /////////////////////////////




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

	view_desc.Format = format == DXGI_FORMAT_UNKNOWN ? texture_desc.Format : format;
	view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
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

	

	vector<BufferStatus> buffer_status;

	vector<shared_ptr<ShaderResource>> resources_status;

};

/// \brief Shared implementation of DX11Material.
struct DX11Material::MaterialImpl{

	unique_ptr<ID3D11VertexShader, COMDeleter> vertex_shader;			///< \brief Vertex shader.

	unique_ptr<ID3D11HullShader, COMDeleter> hull_shader;				///< \brief Hull shader.

	unique_ptr<ID3D11DomainShader, COMDeleter> domain_shader;			///< \brief Domain shader.

	unique_ptr<ID3D11GeometryShader, COMDeleter> geometry_shader;		///< \brief Geometry shader.

	unique_ptr<ID3D11PixelShader, COMDeleter> pixel_shader;				///< \brief Pixel shader.

	ShaderReflection reflection;										/// \brief Combined reflection of the shaders.
	
};

// MATERIAL

DX11Material::DX11Material(ID3D11Device& device, const CompileFromFile& bundle){

	string code = IO::ReadFile(bundle.file_name);

	string file_name = string(bundle.file_name.begin(), bundle.file_name.end());
	
	// Shared implementation
	shared_impl_ = make_shared<MaterialImpl>();

	// Private implementation
	ID3D11VertexShader * vs = nullptr;
	ID3D11HullShader * hs = nullptr;
	ID3D11DomainShader * ds = nullptr;
	ID3D11GeometryShader * gs = nullptr;
	ID3D11PixelShader * ps = nullptr;

	wstring error;

	auto rollback = make_scope_guard([&](){

		if (vs) vs->Release();
		if (hs) hs->Release();
		if (ds) ds->Release();
		if (gs) gs->Release();
		if (ps) ps->Release();

	});

	// Vertex shader and pixel shader are mandatory for materials.

	THROW_ON_FAIL(MakeShader(device, code, file_name, &vs, &(shared_impl_->reflection), &error), error);
				  MakeShader(device, code, file_name, &hs, &(shared_impl_->reflection));
				  MakeShader(device, code, file_name, &ds, &(shared_impl_->reflection));
				  MakeShader(device, code, file_name, &gs, &(shared_impl_->reflection));
	THROW_ON_FAIL(MakeShader(device, code, file_name, &ps, &(shared_impl_->reflection), &error), error);

}

DX11Material::DX11Material(ID3D11Device& device, const InstantiateFromMaterial& bundle){

	// TODO: Instantiate etc.
	

}

DX11Material::~DX11Material(){


}

shared_ptr<Material::Variable> DX11Material::GetVariable(const string& name){

	return nullptr;

}

shared_ptr<Material::Resource> DX11Material::GetResource(const string& name){

	return nullptr;

}

size_t DX11Material::GetSize() const{

	return 0;

}