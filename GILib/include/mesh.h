/// \file mesh.h
/// \brief This file contains the interfaces used to define mesh resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "eigen.h"
#include "resources.h"

namespace gi_lib{

	struct AABB;
	
	/// \brief The vertex declares position, texture coordinates and normals.
	struct VertexFormatNormalTextured{

		Vector3f position;			///< Position of the vertex.
		Vector3f normal;			///< Vertex normal.
		Vector2f tex_coord;			///< Texture coordinates.

	};

	/// \brief Subset of a mesh.
	struct MeshSubset{

		size_t start_index;			///< \brief Start index.

		size_t count;				///< \brief Index count.

	};

	/// \brief Base interface for static meshes.
	/// \author Raffaele D. Facendola.
	class IStaticMesh : public IResource{

	public:

		/// \brief Structure used to build a mesh from an array of vertices.
		/// \tparam TVertexFormat Format of each vertex.
		template <typename TVertexFormat>
		struct FromVertices{

			NO_CACHE;

			vector<unsigned int> indices;			///< \brief Indices definition.

			vector<TVertexFormat> vertices;			///< \brief Vertices definition. 

			vector<MeshSubset> subsets;				///< \brief Mesh subset definition.

		};

		/// \brief Virtual destructor.
		virtual ~IStaticMesh(){}

		/// \brief Get the vertices count.
		/// \return Returns the vertices count.
		virtual size_t GetVertexCount() const = 0;

		/// \brief Get the polygons count.
		/// A polygon is a triangle.
		/// \return Returns the polygons count.
		virtual size_t GetPolygonCount() const = 0;

		/// \brief Get the level of detail count.
		/// \return Returns the level of detail count.
		virtual size_t GetLODCount() const = 0;

		/// \brief Get the bounds of the mesh.
		/// \return Returns the bounding box of the mesh in object space.
		virtual const AABB& GetBoundingBox() const = 0;

		/// \brief Total number of subsets used by this mesh.
		/// \return Returns the total number of subsets used by this mesh.
		virtual size_t GetSubsetCount() const = 0;

		/// \brief Get a mesh subset.
		/// \param subset_index Index of the subset to get.
		/// \return Returns the specified mesh subset.
		virtual const MeshSubset& GetSubset(unsigned int subset_index) const = 0;

	};


}