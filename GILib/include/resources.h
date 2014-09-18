/// \file resources.h
/// \brief Generic graphical resource interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>

#include "gimath.h"

using ::std::vector;
using ::Eigen::Vector2f;
using ::Eigen::Vector3f;

namespace gi_lib{
	
	/// \brief Describe the priority of the resources.
	enum class ResourcePriority{

		MINIMUM,		///< Lowest priority. These resources will be the first one to be freed when the system will run out of memory.
		LOW,			///< Low priority.
		NORMAL,			///< Normal priority. Default value.
		HIGH,			///< High priority.
		CRITICAL		///< Highest priority. These resources will be kept in memory at any cost.

	};

	/// \brief Techniques used to resolve texture coordinates that are outside of the texture's boundaries.
	enum class WrapMode{

		WRAP,		///< Texture coordinates are repeated with a period of 1.
		CLAMP		///< Texture coordinates are clamped inside the range [0, 1]

	};

	/// \brief The vertex declares position, texture coordinates and normals.
	struct VertexFormatNormalTextured{

		Vector3f position;			///< Position of the vertex.
		Vector3f normal;			///< Vertex normal.
		Vector2f tex_coord;			///< Texture coordinates.

	};
	
	/// \brief Base interface for graphical resources.
	/// \author Raffaele D. Facendola.
	class Resource{

	public:

		virtual ~Resource(){}

		/// \brief Get the memory footprint of this resource.
		/// \return Returns the size of the resource, in bytes.
		virtual size_t GetSize() const = 0;

		/// \brief Get the priority of the resource.
		/// \return Returns the resource priority.
		virtual ResourcePriority GetPriority() const = 0;

		/// \brief Set the priority of the resource.
		/// \param priority The new priority.
		virtual void SetPriority(ResourcePriority priority) = 0;

	};

	/// \brief Base interface for plain textures.
	/// \author Raffaele D. Facendola.
	class Texture2D : public Resource{

	public:

		/// \brief Enumeration of all possible load modes.
		enum class LoadMode{

			kFromDDS = 0,				///< The resource is loaded from a DDS file.

		};

		/// \brief Interface destructor.
		virtual ~Texture2D(){}

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual size_t GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual size_t GetHeight() const = 0;

		/// \brief Get the mip map level count.
		/// \return Returns the mip map level count.
		virtual size_t GetMipMapCount() const = 0;

		/// \brief Get the wrap mode.

		/// \see WrapMode
		/// \return Return the wrap mode used by this texture.
		virtual WrapMode GetWrapMode() const = 0;

		/// \brief Set the wrap mode.
		/// \param wrap_mode Wrap mode to use.
		virtual void SetWrapMode(WrapMode wrap_mode) = 0;

	};

	/// \brief Base interface for static meshes.
	/// \author Raffaele D. Facendola.
	class Mesh : public Resource{

	public:

		/// \brief Enumeration of all possible load modes.
		enum class LoadMode{
		
		};

		/// \brief Enumeration of all possible build modes.
		enum class BuildMode{

			kNormalTextured = 0,	///< The mesh declares vertex coordinates, texture coordinates and vertex normal.

		};

		virtual ~Mesh(){}

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

	};

	/// \brief Base interface for shaders.
	/// \author Raffaele D. Facendola.
	class Shader : public Resource{

	public:

		/// \brief Enumeration of all possible load modes.
		enum class LoadMode{

			kCompileFromFile = 0,		///< Load the shader from a source file before compiling it.

		};

		/// \brief Enumeration of all possible build modes.
		enum class BuildMode{

		};

		virtual ~Shader(){}

	};

}