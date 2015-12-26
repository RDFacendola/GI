/// \file mesh.h
/// \brief This file contains the interfaces used to define mesh resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "eigen.h"
#include "resources.h"
#include "enums.h"

namespace gi_lib{

	struct AABB;
	
	ENUM_FLAGS(MeshFlags, int) {

		kNone = 0,
		kShadowcaster = 1,

	};

	/// \brief The vertex declares position, texture coordinates and normals.
	struct VertexFormatNormalTextured{

		Vector3f position;			///< \brief Position of the vertex.
		Vector3f normal;			///< \brief Vertex normal.
		Vector2f tex_coord;			///< \brief Texture coordinates.
		Vector3f tangent;			///< \brief Tangent vector.
		Vector3f binormal;			///< \brief Binormal vector.

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

			std::vector<unsigned int> indices;				///< \brief Indices definition.

			std::vector<TVertexFormat> vertices;			///< \brief Vertices definition. 

			std::vector<MeshSubset> subsets;				///< \brief Mesh subset definition.

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

		/// \brief Get a mesh subset's flags.
		/// \param subset_index Index of the subset to access.
		/// \return Returns the flags of the specified subset.
		virtual MeshFlags GetFlags(unsigned int subset_index) const = 0;

		/// \brief Set a mesh subset's flags.
		/// \param subset_index Index of the subset to access.
		/// \param flag Subset's flags.
		virtual void SetFlags(unsigned int subset_index, MeshFlags flags) = 0;

		/// \brief Get the shared mesh flags.
		/// This method will return the list of flags that are shared among all the subsets.
		/// \return Returns the flags of the mesh.
		virtual MeshFlags GetFlags() const = 0;

		/// \brief Set the mesh flags.
		/// \param subset_index Index of the subset to access.
		/// \remarks This method will override each individual subset flags.
		virtual void SetFlags(MeshFlags flags) = 0;

	};


}