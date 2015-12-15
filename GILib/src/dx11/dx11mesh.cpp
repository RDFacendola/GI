#include "dx11/dx11mesh.h"

#include "dx11/dx11graphics.h"

using namespace ::std;
using namespace ::gi_lib;
using namespace ::dx11;
using namespace ::windows;

namespace{

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
					 0.5f * (max_corner - min_corner) };

	}
	
}

///////////////////////////// MESH ////////////////////////////////////////////////

DX11Mesh::DX11Mesh(const FromVertices<VertexFormatNormalTextured>& bundle){

	auto& device = *DX11Graphics::GetInstance().GetDevice();

	// Normal, textured mesh.

	size_t vb_size = bundle.vertices.size() * sizeof(VertexFormatNormalTextured);
	size_t ib_size = bundle.indices.size() * sizeof(unsigned int);
	
	ID3D11Buffer* buffer;
	
	// Vertices

	THROW_ON_FAIL(MakeVertexBuffer(device,
								   &(bundle.vertices[0]),
								   vb_size,
								   &buffer));

	vertex_buffer_ << &buffer;

	// Indices

	if (bundle.indices.size() > 0){

		THROW_ON_FAIL(MakeIndexBuffer(device, 
									  &(bundle.indices[0]), 
									  ib_size,
									  &buffer));
	
		index_buffer_ << &buffer;

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

void DX11Mesh::Bind(ID3D11DeviceContext& context, bool tessellable){

	// Only 1 vertex stream is used.

	unsigned int num_streams = 1;

	ID3D11Buffer* vertex_buffer = vertex_buffer_.Get();

	unsigned int stride = static_cast<unsigned int>(vertex_stride_);

	unsigned int offset = 0;

	// Set the proper primitive type

	auto topology = tessellable ?
					D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST :
					D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_PRIMITIVE_TOPOLOGY previous_topology;

	context.IAGetPrimitiveTopology(&previous_topology);

	if (topology != previous_topology) {
		
		context.IASetPrimitiveTopology(topology);

	}

	// Bind the vertex buffer

	context.IASetVertexBuffers(0,							// Start slot
							   num_streams,
							   &vertex_buffer,
							   &stride,
							   &offset);

	// Bind the index buffer

	context.IASetIndexBuffer(index_buffer_.Get(),
							 DXGI_FORMAT_R32_UINT,
							 0);

}

void DX11Mesh::DrawSubset(ID3D11DeviceContext& context, unsigned int subset_index) const {

	if (index_buffer_) {

		context.DrawIndexed(static_cast<unsigned int>(subsets_[subset_index].count),			// One per index
							static_cast<unsigned int>(subsets_[subset_index].start_index),
							0);

	}
	else {

		context.Draw(static_cast<unsigned int>(subsets_[subset_index].count * 3),				// One per vertex
					 static_cast<unsigned int>(subsets_[subset_index].start_index));

	}
	

}