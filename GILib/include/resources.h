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
using ::std::string;
using ::Eigen::Vector2f;
using ::Eigen::Vector3f;
using ::Eigen::Affine3f;
using ::Eigen::Projective3f;
using ::Eigen::Matrix;

namespace gi_lib{

	class MaterialParameter;
	class Resource;
	class Texture2D;
	class Mesh;
	class Shader;
	class Material;
	class MaterialParameter;

	using Vector4f = Matrix < float, 4, 1, 0, 4, 1 > ;

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

		/// \brief Get the aspect ratio of the render target.

		/// The aspect ratio is Width/Height.
		/// \return Returns the aspect ratio of the render target.
		virtual float GetAspectRatio() const = 0;
		
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

	/// \brief Base interface for materials.

	/// A material is a shader whose parameters are already set.
	/// \author Raffaele D. Facendola
	class Material : public Resource{

	public:

		/// \brief Enumeration of all possible load modes.
		enum class LoadMode{

		};

		/// \brief Enumeration of all possible build modes.
		enum class BuildMode{

			kFromShader = 0,			///< The material is created starting from a shader.

		};

		virtual ~Material(){}

		/// \brief Get a material parameter by name.
		/// \param name The name of the parameter to get.
		/// \return Returns the material parameter whose name is the specified one or null if no parameter could be found.
		virtual shared_ptr<MaterialParameter> GetParameterByName(const string & name) = 0;

		/// \brief Get a material parameter by semantic.
		/// \param semantic The semantic of the parameter to get.
		/// \return Returns the material parameter whose semantic is the specified one or null if no parameter could be found.
		virtual shared_ptr<MaterialParameter> GetParameterBySemantic(const string & semantic) = 0;

		/// \brief Get the shader associated to this material.
		/// return Returns a reference to the shader associated to this material.
		virtual Shader & GetShader() = 0;

		/// \brief Get the shader associated to this material.
		/// return Returns a reference to the shader associated to this material.
		virtual const Shader & GetShader() const = 0;

	};

	/// \brief Base interface for material parameters.

	/// \author Raffaele D. Facendola
	class MaterialParameter{

	public:

		virtual ~MaterialParameter(){}

		/// \brief Attempts to read the parameter as boolean variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was a boolean, false otherwise.
		virtual bool Read(bool & out) = 0;

		/// \brief Attempts to read the parameter as float variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was a float, false otherwise.
		virtual bool Read(float & out) = 0;

		/// \brief Attempts to read the parameter as integer variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was an integer, false otherwise.
		virtual bool Read(int & out) = 0;

		/// \brief Attempts to read the parameter as 2-dimensional float vector variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was a 2-dimensional float vector, false otherwise.
		virtual bool Read(Vector2f & out) = 0;

		/// \brief Attempts to read the parameter as 3-dimensional float vector variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was a 3-dimensional float vector, false otherwise.
		virtual bool Read(Vector3f & out) = 0;

		/// \brief Attempts to read the parameter as 4-dimensional float vector variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was a 4-dimensional float vector, false otherwise.
		virtual bool Read(Vector4f & out) = 0;

		/// \brief Attempts to read the parameter as 4x4 affine matrix variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was a 4x4 affine matrix, false otherwise.
		virtual bool Read(Affine3f & out) = 0;

		/// \brief Attempts to read the parameter as 4x4 projective matrix variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was a 4x4 projective matrix, false otherwise.
		virtual bool Read(Projective3f & out) = 0;

		/// \brief Attempts to read the parameter as 2D texture variable.
		/// \param out Destination of the result if the method succeeds.
		/// \return Return true if the variable was a 2D texture, false otherwise.
		virtual bool Read(shared_ptr<Texture2D> & out) = 0;

		/// \brief Attempts to read the parameter as a structure.
		/// \param out Destination of the resource if the method succeeds.
		/// \return Return true if the variable was a structure, false otherwise.
		virtual bool Read(void ** out) = 0;

		/// \brief Attempts to write the parameter as boolean variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was a boolean, false otherwise.
		virtual bool Write(const bool & in) = 0;

		/// \brief Attempts to write the parameter as float variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was a float, false otherwise.
		virtual bool Write(const float & in) = 0;

		/// \brief Attempts to write the parameter as integer variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was an integer, false otherwise.
		virtual bool Write(const int & in) = 0;

		/// \brief Attempts to write the parameter as 2-dimensional float vector variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was a 2-dimensional float vector, false otherwise.
		virtual bool Write(const Vector2f & in) = 0;

		/// \brief Attempts to write the parameter as 3-dimensional float vector variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was a 3-dimensional float vector, false otherwise.
		virtual bool Write(const Vector3f & in) = 0;

		/// \brief Attempts to write the parameter as 4-dimensional float vector variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was a 4-dimensional float vector, false otherwise.
		virtual bool Write(const Vector4f & in) = 0;

		/// \brief Attempts to write the parameter as 4x4 affine matrix variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was a 4x4 affine matrix, false otherwise.
		virtual bool Write(const Affine3f & in) = 0;

		/// \brief Attempts to write the parameter as 4x4 projective matrix variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was a 4x4 projective matrix, false otherwise.
		virtual bool Write(const Projective3f & in) = 0;

		/// \brief Attempts to write the parameter as 2D texture variable.
		/// \param in Value to copy.
		/// \return Return true if the variable was a 2D texture, false otherwise.
		virtual bool Write(const shared_ptr<Texture2D> in) = 0;

		/// \brief Attempts to write the parameter as a structure.
		/// \param in Value to copy.
		/// \return Return true if the variable was a structure, false otherwise.
		virtual bool Write(void ** in) = 0;

	};

}