/// \file resources.h
/// \brief Generic graphical resource interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <typeinfo>
#include <typeindex>

#include "gilib.h"
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

	enum class AntialiasingMode;

	class IResource;
	class IResourceView;

	class Texture2D;
	class RenderTarget;
	class Mesh;
	class Material;

	/// \brief Macro used to declare that the bundle will use the caching mechanism.
	#define USE_CACHE \
	using use_cache = void

	/// \brief Macro used to declare that the bundle won't use the caching mechanism.
	#define NO_CACHE \
	using no_cache = void

	/// \brief If T declares a type "use_cache", use_cache has a public member "type", otherwise there's no member.
	template <typename T, typename T::use_cache* = nullptr>
	struct use_cache{

		using type = void;

	};

	/// \brief If T declares a type "no_cache", no_cache has a public member "type", otherwise there's no member.
	template <typename T, typename T::no_cache* = nullptr>
	struct no_cache{

		using type = void;

	};

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
	
	/// \brief Base interface for graphical resources.
	/// Resources are reference counted.
	/// \author Raffaele D. Facendola.
	class IResource : public Object{

	public:

		virtual ~IResource(){}

		/// \brief Get the memory footprint of this resource.
		/// \return Returns the size of the resource, in bytes.
		virtual size_t GetSize() const = 0;

	protected:

		/// \brief Protected constructor. Prevent instantiation.
		IResource(){}

	};

	/// \brief A resource view, used to bind resources to the graphic pipeline.
	/// Resource views are reference counted.
	/// \author Raffaele D. Facendola
	class IResourceView : public Object{

	public:

		/// \brief Needed for virtual classes.
		virtual ~IResourceView(){}

	protected:

		/// \brief Protected constructor. Prevent instantiation.
		IResourceView(){}

	};

	/// \brief Base interface for plain textures.
	/// \author Raffaele D. Facendola.
	class Texture2D : public IResource{

	public:

		/// \brief Cached structure used to load a texture 2D from file.	
		struct FromFile{

			USE_CACHE;

			wstring file_name;		///< \brief Name of the file to load.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Interface destructor.
		virtual ~Texture2D(){}

		/// \brief Get the width of the texture.
		/// \return Returns the width of the texture, in pixel.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the texture.
		/// \return Returns the height of the texture, in pixel.
		virtual unsigned int GetHeight() const = 0;

		/// \brief Get the MIP map level count.
		/// \return Returns the MIP map level count.
		virtual unsigned int GetMipMapCount() const = 0;

		/// \brief Get the view to this resource.
		/// Use this view to bind the texture to the graphic pipeline.
		/// \return Returns a pointer to the resource view.
		virtual ObjectPtr<IResourceView> GetView() const = 0;

	};

	/// \brief Base interface for render targets.
	/// A render target is a texture that can be used to draw an image onto.
	/// It may also have its how depth and stencil buffer.
	/// This class handles Multi Render Targets (MRT) as well.
	/// \author Raffaele D. Facendola.
	class RenderTarget : public IResource{

	public:

		virtual ~RenderTarget(){};

		/// \brief Get the number of surfaces in this render target.
		/// \return Returns the number of surfaces in this render target.
		virtual unsigned int GetCount() const = 0;

		/// \brief Get the texture associated to this render target.
		/// \param index The index of the render target texture.
		/// \return Returns the texture associated to the index-th render target.
		virtual ObjectPtr<Texture2D> GetTexture(int index) = 0;

		/// \brief Get the texture associated to this render target.
		/// \param index The index of the render target texture.
		/// \return Returns the texture associated to the index-th render target.
		virtual ObjectPtr<const Texture2D> GetTexture(int index) const = 0;

		/// \brief Get the texture associated to the depth stencil buffer.
		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target.
		virtual ObjectPtr<Texture2D> GetZStencil() = 0;

		/// \brief Get the texture associated to the depth stencil buffer.
		/// The texture is guaranteed to have a 24bit uniform channel for the depth information and a 8bit unsigned int channel for the stencil.
		/// \return Returns the texture associated to the depth stencil buffer used by this render target.
		virtual ObjectPtr<const Texture2D> GetZStencil() const = 0;

		/// \brief Get the aspect ratio of the render target.
		/// The aspect ratio is Width/Height.
		/// \return Returns the aspect ratio of the render target.
		virtual float GetAspectRatio() const = 0;

		/// \brief Get the anti-aliasing mode of the render target.
		/// The anti-aliasing mode influences the number of samples per pixel for techniques like MSAA.
		/// \return Return the anti-aliasing mode of the render target.
		virtual AntialiasingMode GetAntialiasing() const = 0;
		
		/// \brief Resize the render target surfaces.
		/// \param width The new width of the buffer.
		/// \param height The new height of the buffer.
		/// \return Returns true if the texture was resized, returns false otherwise.
		virtual bool Resize(unsigned int width, unsigned int height) = 0;

		/// \brief Get the width of the render target in pixels.
		/// \return Returns the width of the render target.
		virtual unsigned int GetWidth() const = 0;

		/// \brief Get the height of the render target in pixels.
		/// \return Returns the height of the render target.
		virtual unsigned int GetHeight() const = 0;

	};

	/// \brief Base interface for static meshes.
	/// \author Raffaele D. Facendola.
	class Mesh : public IResource{

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

	/// \brief Base interface for materials.
	/// \author Raffaele D. Facendola
	class Material : public IResource{

	public:

		/// \brief Structure used to compile a material from a file.
		struct CompileFromFile{

			USE_CACHE;

			wstring file_name;			///< \brief Name of the file containing the material code.

			/// \brief Get the cache key associated to the structure.
			/// \return Returns the cache key associated to the structure.
			size_t GetCacheKey() const;

		};

		/// \brief Structure used to instantiate an existing material.
		struct Instantiate{

			NO_CACHE;

			ObjectPtr<Material> base;	///< \brief Material to instantiate.

		};
		
		/// \brief Interface for material variables.
		class MaterialVariable : public Object{

		public:

			/// \brief Default destructor.
			virtual ~MaterialVariable(){}

			/// \brief Set the variable value.
			/// \param value The value to set.
			template <typename TValue>
			void Set(const TValue& value);

			/// \brief Set the variable value.
			/// \param buffer Pointer to the buffer holding the data to write.
			/// \param size Size of the buffer.
			virtual void Set(const void* buffer, size_t size) = 0;

		};

		/// \brief Interface for material resources.
		class MaterialResource : public Object{

		public:

			/// \brief Default destructor.
			virtual ~MaterialResource(){}

			/// \brief Set the resource value.
			/// \param resource The resource to bind to the material.
			virtual void Set(ObjectPtr<IResourceView> resource) = 0;

		};

		/// \brief Default destructor.
		virtual ~Material(){}

		/// \brief Get a material variable by name.
		/// \param name The name of the variable.
		/// \return Returns a pointer to the variable matching the specified name if found, returns nullptr otherwise.
		virtual ObjectPtr<MaterialVariable> GetVariable(const string& name) = 0;

		/// \brief Get a material resource by name.
		/// \param name The name of the resource.
		/// \return Returns a pointer to the resource matching the specified name if found, returns nullptr otherwise.
		virtual ObjectPtr<MaterialResource> GetResource(const string& name) = 0;

	};

	/// \brief Represents a strongly typed hardware vector.
	/// The vector can be written by the CPU and read by the GPU.
	/// The size of the vector does not change.
	/// \author Raffaele D. Facendola
	class StructuredVector : public IResource{

	public:

		/// \brief Structure used to build an empty structured vector from a description.
		struct FromDescription{

			NO_CACHE;

			size_t element_count;			///< \brief Elements inside the vector.

			size_t element_size;			///< \brief Size of each element.

		};

		/// \brief Virtual destructor.
		virtual ~StructuredVector(){}

		/// \brief Get the element count of this vector.
		/// \return Returns the element count of this vector.
		virtual size_t GetElementCount() const = 0;

		/// \brief Get the size of each element in bytes.
		/// \return Returns the size of each element.
		virtual size_t GetElementSize() const = 0;

		/// \brief Get the view to this resource.
		/// Use this view to bind the vector to the graphic pipeline.
		/// \return Returns a pointer to the resource view.
		virtual ObjectPtr<IResourceView> GetView() = 0;

		/// \brief Maps the buffer to the system memory, granting CPU access.
		/// \tparam TElement Type of the elements to map. Make sure to match the type this buffer was created with!.
		/// \return Returns a pointer to the first element of the array.
		/// \remarks Be sure to unlock the buffer afterwards: the context will hang trying to access a locked resource.
		template <typename TElement>
		TElement* Lock();

		/// \brief Commit a mapped buffer back to the video memory, revoking CPU access.
		/// \remarks Unlocking the buffer will invalidate the system-memory pointer returned by the Lock method.
		virtual void Unlock() = 0;

	private:

		/// \brief Maps the buffer to the system memory, granting CPU access.
		/// The elements already present inside the vector are discarded.
		/// \return Returns a pointer to the first element of the array.
		/// \remarks Be sure to unlock the buffer afterwards: the context will hang trying to access a locked resource.
		virtual void* LockDiscard() = 0;

	};

	///////////////////////////////////////// MATERIAL /////////////////////////////////////////

	template <typename TValue>
	inline void Material::MaterialVariable::Set(const TValue& value){

		Set(static_cast<const void*>(&value),
			sizeof(TValue));

	}
	
	///////////////////////////////////////// STRUCTURED VECTOR /////////////////////////////////////////

	template <typename TElement>
	inline TElement* StructuredVector::Lock(){

		return static_cast<TElement*>(LockDiscard());

	}
	
}