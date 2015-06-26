/// \file dx11mesh.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include "mesh.h"

#include "gimath.h"

#include "dx11/dx11.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using windows::unique_com;

		/// \brief DirectX11 static mesh.
		/// \author Raffaele D. Facendola.
		class DX11Mesh : public IStaticMesh{

		public:

			/// \brief Create a new DirectX11 mesh.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to create the mesh.
			DX11Mesh(const FromVertices<VertexFormatNormalTextured>& args);

			virtual size_t GetSize() const override;

			virtual size_t GetVertexCount() const override;

			virtual size_t GetPolygonCount() const override;

			virtual size_t GetLODCount() const override;

			virtual const AABB& GetBoundingBox() const override;

			virtual size_t GetSubsetCount() const override;

			virtual const MeshSubset& GetSubset(unsigned int subset_index) const override;

			/// \brief Bind the mesh to the given context.
			void Bind(ID3D11DeviceContext& context);

		private:

			unique_com<ID3D11Buffer> vertex_buffer_;

			unique_com<ID3D11Buffer> index_buffer_;

			vector<MeshSubset> subsets_;

			size_t vertex_count_;

			size_t polygon_count_;

			size_t LOD_count_;

			size_t size_;

			size_t vertex_stride_;											///< \brief Size of each vertex in bytes

			AABB bounding_box_;

		};
		
	}

}

///////////////////////////// MESH ////////////////////////////////////////////////

inline size_t gi_lib::dx11::DX11Mesh::GetVertexCount() const{

	return vertex_count_;

}

inline size_t gi_lib::dx11::DX11Mesh::GetPolygonCount() const{

	return polygon_count_;

}

inline size_t gi_lib::dx11::DX11Mesh::GetLODCount() const{

	return LOD_count_;

}

inline size_t gi_lib::dx11::DX11Mesh::GetSize() const{

	return size_;

}

inline const gi_lib::AABB& gi_lib::dx11::DX11Mesh::GetBoundingBox() const{

	return bounding_box_;

}

inline size_t gi_lib::dx11::DX11Mesh::GetSubsetCount() const{

	return subsets_.size();

}

inline const gi_lib::MeshSubset& gi_lib::dx11::DX11Mesh::GetSubset(unsigned int subset_index) const{

	return subsets_[subset_index];

}

#endif