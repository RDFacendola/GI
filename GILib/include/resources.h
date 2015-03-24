/// \file resources.h
/// \brief Generic graphical resource interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <memory>
#include <string>

#include "gimath.h"

using ::std::vector;
using ::std::shared_ptr;
using ::std::wstring;
using ::std::string;
using ::Eigen::Vector2f;
using ::Eigen::Vector3f;
using ::Eigen::Affine3f;
using ::Eigen::Projective3f;
using ::Eigen::Matrix;

namespace gi_lib{

	class Resource;
	class Texture2D;
	class Mesh;
	class Material;

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

	};

	/// \brief Base interface for graphical resources that can also be bound to the pipeline as shader resources.
	class ShaderResource{

	public:

		/// \brief Needed for virtual classes.
		virtual ~ShaderResource(){}

	};

	/// \brief Base interface for plain textures.
	/// \author Raffaele D. Facendola.
	class Texture2D : public Resource, public ShaderResource{

	public:

		/// \brief Interface destructor.
		virtual ~Texture2D(){}

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the mip map level count.
		/// \return Returns the mip map level count.
		virtual unsigned int GetMipMapCount() const = 0;

	};

	/// \brief Base interface for render targets.
	
	/// A render target is a texture that can be used to draw an image onto.
	/// It may also have its how depth and stencil buffer.
	/// This class handles Multi Render Targets (MRT) as well.
	/// \author Raffaele D. Facendola.
	class RenderTarget : public Resource{

	public:

		virtual ~RenderTarget(){};

		/// \brief Get the number of surfaces in this render target.
		/// \return Returns the number of surfaces in this render target.
		virtual unsigned int GetCount() const = 0;

		/// \brief Get the texture associated to this render target.
		/// \param index The index of the render target texture.
		/// \return Returns the texture associated to the index-th render target.
		virtual shared_ptr<Texture2D> GetTexture(int index) = 0;

		/// \brief Get the texture associated to this render target.
		/// \param index The index of the render target texture.
		/// \return Returns the texture associated to the index-th render target.
		virtual shared_ptr<const Texture2D> GetTexture(int index) const = 0;

		/// \brief Get the texture associated to the depth stencil buffer.
		
		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target.
		virtual shared_ptr<Texture2D> GetZStencil() = 0;

		/// \brief Get the texture associated to the depth stencil buffer.

		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target.
		virtual shared_ptr<const Texture2D> GetZStencil() const = 0;

		/// \brief Get the aspect ratio of the render target.

		/// The aspect ratio is Width/Height.
		/// \return Returns the aspect ratio of the render target.
		virtual float GetAspectRatio() const = 0;
		
	};

	/// \brief Base interface for meshes.
	/// \author Raffaele D. Facendola.
	class Mesh : public Resource{

	public:

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

		/// \brief Get the bounds of the mesh.
		/// \return Returns the bounds of the mesh in object space.
		virtual Bounds GetBounds() const = 0;

	};

	/// \brief Base interface for materials.

	/// \author Raffaele D. Facendola
	class Material : public Resource{

	public:

		/// \brief Interface for material variables.
		class Variable{

		public:

			/// \brief Default destructor.
			virtual ~Variable(){}

			/// \brief Set the variable value.
			/// \param value The value to set.
			template <typename TValue>
			void Set(const TValue& value);

		protected:

			/// \brief Set the variable value.
			/// \param buffer Pointer to the buffer holding the data to write.
			/// \param size Size of the buffer.
			virtual void Set(void * buffer, size_t size) = 0;

		};

		/// \brief Interface for material resources.
		class Resource{

		public:

			/// \brief Default destructor.
			virtual ~Resource(){}

			/// \brief Set the resource value.
			/// \param resource The resource to bind to the material.
			virtual void Set(shared_ptr<ShaderResource> resource) = 0;

		};

		/// \brief Default destructor.
		virtual ~Material(){}

		/// \brief Get a material variable by name.
		/// \param name The name of the variable.
		/// \return Returns a pointer to the variable matching the specified name.
		virtual shared_ptr<Variable> GetVariable(const string& name) = 0;

		/// \brief Get a material resource by name.
		/// \param name The name of the resource.
		/// \return Returns a pointer to the resource matching the specified name.
		virtual shared_ptr<Resource> GetResource(const string& name) = 0;

	};
	
}