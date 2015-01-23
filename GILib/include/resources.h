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

		/// \brief Get the wrap mode.

		/// \see WrapMode
		/// \return Return the wrap mode used by this texture.
		virtual WrapMode GetWrapMode() const = 0;

		/// \brief Set the wrap mode.
		/// \param wrap_mode Wrap mode to use.
		virtual void SetWrapMode(WrapMode wrap_mode) = 0;

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

		virtual ~Material(){}

		/// \brief Get the index of a parameter knowing its name.

		/// The parameter name is case-sensitive.
		/// \param name The name of the parameter.
		/// \return Returns the index of the parameter whose name matches the specified one.
		virtual unsigned int GetParameterIndex(const wstring& name) const = 0;

		/// \brief Get the index of a texture knowing its name.

		/// The texture name is case-sensitive.
		/// \param name The name of the texture.
		/// \return Returns the index of the texture whose name matches the specified one.
		virtual unsigned int GetTextureIndex(const wstring& name) const = 0;

	};

	/// \brief A material instance.

	/// A material instance is a material whose parameter have already been set.
	/// \author Raffaele D. Facendola
	class MaterialInstance : public Resource{

	public:

		virtual ~MaterialInstance(){}

		/// \brief Get the index of a parameter knowing its name.

		/// The parameter name is case-sensitive.
		/// \param name The name of the parameter.
		/// \return Returns the index of the parameter whose name matches the specified one.
		virtual unsigned int GetParameterIndex(const wstring& name) const  = 0;

		/// \brief Set a new value for a parameter.

		/// \tparam TType type of the parameter to set.
		/// \param name The name of the parameter to set.
		/// \param value The value the parameter must be set to.
		/// \return Returns true if the method succeeds, returns false otherwise.
		/// \remarks The method fails if the specified name could not be found or the type specified for the parameter is not compatible with the expected type.
		template <typename TType>
		bool SetParameter(const wstring& name, const TType& value);

		/// \brief Set a new value for a parameter.

		/// \tparam TType type of the parameter to set.
		/// \param index The index of the parameter to set.
		/// \param value The value the parameter must be set to.
		/// \return Returns true if the method succeeds, returns false otherwise.
		/// \remarks The method fails if the index is not valid or the type specified for the parameter is not compatible with the expected type.
		template <typename TType>
		bool SetParameter(unsigned int index, const TType& value);

		/// \brief Get the index of a texture knowing its name.

		/// The texture name is case-sensitive.
		/// \param name The name of the texture.
		/// \return Returns the index of the texture whose name matches the specified one.
		virtual unsigned int GetTextureIndex(const wstring& name) const  = 0;

		/// \brief Set a new value for a texture.
		/// \param name The name of the texture to set.
		/// \param texture Reference to the texture to set.
		/// \return Returns true if the method succeeds, returns false otherwise.
		/// \remarks The method fails if the specified name could not be found or the texture type is not compatible with the expected type.
		virtual bool SetTexture(const wstring &name, shared_ptr<Texture2D> texture) = 0;

		/// \brief Set a new value for a texture.
		/// \param name The name of the texture to set.
		/// \param index The index of the texture to set.
		/// \return Returns true if the method succeeds, returns false otherwise.
		/// \remarks The method fails if the specified name could not be found or the texture type is not compatible with the expected type.
		virtual bool SetTexture(unsigned int index, shared_ptr<Texture2D> texture) = 0;
						
	protected:

		/// \brief Set a parameter value from a raw buffer.
		/// \param name The name of the parameter to set.
		/// \param buffer Pointer to the buffer containing the data.
		/// \param size Size of the buffer.
		/// \return Returns true if the method succeeds, returns false otherwise.
		/// \remarks The method fails if the parameter name could not be found or the type specified for the parameter is not compatible with the expected type.
		virtual bool SetParameter(const wstring & name, const void* buffer, size_t size) = 0;

		/// \brief Set a parameter value from a raw buffer.
		/// \param index The index of the parameter to set.
		/// \param buffer Pointer to the buffer containing the data.
		/// \param size Size of the buffer.
		/// \return Returns true if the method succeeds, returns false otherwise.
		/// \remarks The method fails if the parameter name could not be found or the type specified for the parameter is not compatible with the expected type.
		virtual bool SetParameter(unsigned int index, const void* buffer, size_t size) = 0;

	};

	// Material

	template <typename TType>
	inline bool MaterialInstance::SetParameter(const wstring & name, const TType& value){

		return SetParameter(name, &value, sizeof(TType));

	}

	template <typename TType>
	inline bool MaterialInstance::SetParameter(unsigned int index, const TType& value){

		return SetParameter(index, &value, sizeof(TType));

	}

}