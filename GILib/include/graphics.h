/// \file graphics.h
/// \brief Defines types, classes and methods used to manage the graphical subsystem. 
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <map>
#include <tuple>

#include "resources.h"
#include "bundles.h"
#include "observable.h"

using ::std::wstring;
using ::std::vector;
using ::std::shared_ptr;
using ::std::unique_ptr;
using ::std::weak_ptr;
using ::std::type_index;
using ::std::map;

namespace gi_lib{

	class Window;
	class Resource;
	class Scene;

	/// \brief Enumeration of all the supported API.
	enum class API{

		DIRECTX_11,		///< DirectX 11.0

	};

	/// \brief Enumeration of all supported anti-aliasing techniques.
	enum class AntialiasingMode{

		NONE,			///< No antialiasing.
		MSAA_2X,		///< Multisample antialiasing, 2X.
		MSAA_4X,		///< Multisample antialiasing, 4X.
		MSAA_8X,		///< Multisample antialiasing, 8X.
		MSAA_16X,		///< Multisample antialiasing, 16X.

	};

	/// \brief Describes a video mode.
	struct VideoMode{

		unsigned int horizontal_resolution;		///< Horizontal resolution, in pixels.
		unsigned int vertical_resolution;		///< Vertical resolution, in pixels.
		unsigned int refresh_rate;				///< Refresh rate, in Hz.

	};

	/// \brief Describes the video card's parameters and capabilities.
	struct AdapterProfile{

		wstring name;										///< Name of the video card.
		size_t dedicated_memory;							///< Dedicated memory, in bytes.
		size_t shared_memory;								///< Shared memory, in bytes.		
		vector<VideoMode> video_modes;						///< List of supported video modes.
		vector<AntialiasingMode> antialiasing_modes;		///< List of supported antialiasing modes.

		unsigned int max_anisotropy;						///< Maximum level of anisotropy.
		unsigned int max_mips;								///< Maximum number of MIP levels.

	};

	/// \brief Viewport bounds
	struct Viewport{

		Vector2f position;									///< \brief Position of the top-left corner in screen units. Valid range between 0 (top/left) and 1 (bottom/right)).
		Vector2f extents;									///< \brief Extents of the viewport in screen units. Valid range between 0 and 1 (full size).
		
	};

	/// \brief A color.
	union Color{

		struct{

			float alpha;									///< \brief Alpha component.
			float red;										///< \brief Red component.
			float green;									///< \brief Green component.
			float blue;										///< \brief Blue component.

		} color;											///< \brief Color.

		float argb[4];										///< \brief Array of components.

	};

	/// \brief Interface used to display an image to an output.
	/// \author Raffaele D. Facendola
	class Output{

	public:

		/// \brief Default destructor;
		virtual ~Output(){}

		/// \brief Set the video mode.
		/// \param video_mode The video mode to set.
		virtual void SetVideoMode(const VideoMode & video_mode) = 0;

		/// \brief Get the current video mode.
		/// \return Returns the current video mode.
		virtual const VideoMode & GetVideoMode() const = 0;

		/// \brief Enable or disable fullscreen state.
		/// \param fullscreen Set to true to enable fullscreen mode, false to disable it.
		virtual void SetFullscreen(bool fullscreen) = 0;

		/// \brief Get the current fullscreen state.
		/// \return Returns true if fullscreen is enabled, false otherwise.
		virtual bool IsFullscreen() const = 0;

		/// \brief Enable or disable VSync.
		/// \param vsync Set to true to enable VSync, false to disable it.
		virtual void SetVSync(bool vsync) = 0;

		/// \brief Get the current VSync state.
		/// \return Returns true if VSync is enabled, false otherwise.
		virtual bool IsVSync() const = 0;

		/// \brief Set the hardware antialiasing mode.
		/// \param antialiasing The new antialiasing mode.
		virtual void SetAntialiasing(AntialiasingMode antialiasing) = 0;

		/// \brief Get the current antialiasing mode.
		/// \return Return the current antialiasing mode.
		virtual AntialiasingMode GetAntialiasing() const = 0;

		/// \brief Get the render target associated to this output.
		/// \return Returns the render target associated to this output.
		virtual shared_ptr<RenderTarget> GetRenderTarget() = 0;

	};

	/// \brief Resource manager interface.
	/// \author Raffaele D. Facendola.
	class Resources{

	public:

		/// \brief Filename of the built-in phong shader.
		static wchar_t * kPhongShaderFile;

		/// \brief Default constructor.
		Resources();

		/// \brief Default destructor;
		virtual ~Resources(){};
		
		template <typename TResource, typename TBundle, typename use_cache<TBundle>::type* = nullptr>
		shared_ptr<TResource> Load(const typename TBundle& bundle);

		template <typename TResource, typename TBundle, typename no_cache<TBundle>::type* = nullptr>
		shared_ptr<TResource> Load(const typename TBundle& bundle);

		/// \brief Get the amount of memory used by the loaded resources.
		size_t GetSize();

	protected:

		/// \brief Used as key to address the resource map flyweight.
		struct ResourceMapKey{

			/// \brief Type index associated to the type of the resource to load.
			type_index resource_type_id;

			/// \brief Type index associated to the bundle type used to load the resource.
			type_index bundle_type_id;

			/// \brief Unique cache key associated to the bundle used to load the resource.
			size_t cache_key;
			
			/// \brief Default constructor.
			ResourceMapKey();

			/// \brief Create a new resource map key.
			ResourceMapKey(type_index resource_type, type_index bundle_type, size_t key);

			/// \brief Lesser comparison operator.
			bool operator<(const ResourceMapKey & other) const;
			
		};

		/// \brief Type of resource map values.
		using ResourceMapValue =  weak_ptr < Resource >;

		/// \brief Type of resource map. 
		using ResourceMap = map < ResourceMapKey, ResourceMapValue >;

		/// \brief Load a resource.
		/// \param resource_type Resource's type index.
		/// \param bundle_type Bundle's type index.
		/// \param bundle Pointer to the bundle to be used to load the resource.
		/// \return Returns a pointer to the loaded resource
		virtual unique_ptr<Resource> Load(const type_index & resource_type, const type_index & bundle_type, const void * bundle) = 0;

	private:

		// Map of the immutable resources
		ResourceMap resources_;

		// Base path for the resources
		wstring base_path_;

	};

	/// \brief Factory interface used to create and initialize the graphical subsystem.
	/// \author Raffaele D. Facendola
	class Graphics{

	public:
		
		/// \brief Get a reference to a specific graphical subsystem.
		static Graphics& GetAPI(API api);
		
		/// \brief Default destructor;
		virtual ~Graphics(){}

		/// \brief Get the video card's parameters and capabilities.
		virtual AdapterProfile GetAdapterProfile() const = 0;

		/// \brief Initialize an output.
		/// \param window The window used to display the output.
		/// \param video_mode The initial window mode.
		/// \return Returns a pointer to the new output.
		virtual unique_ptr<Output> CreateOutput(Window & window, const VideoMode & video_mode) = 0;

		/// \brief Get the resource manager.
		/// \return Returns the resource manager.
		virtual Resources & GetResources() = 0;

	protected:

		Graphics();

	};

	// Resources

	template <typename TResource, typename TBundle, typename use_cache<TBundle>::type*>
	shared_ptr<TResource> Resources::Load(const typename TBundle & bundle){

		// Cached version

		ResourceMapKey key(std::type_index(typeid(TResource)),
						   std::type_index(typeid(TBundle)),
						   bundle.GetCacheKey());

		auto it = resources_.find(key);

		if (it != resources_.end()){

			if (auto resource = it->second.lock()){

				// The same resource already exists and is still valid.
				return static_pointer_cast<TResource>(resource);

			}

			//Resource was expired...

		}

		// Load the actual resource
		auto resource = shared_ptr<Resource>(std::move(Load(type_index(typeid(TResource)),
															std::type_index(typeid(TBundle)), 
															&bundle)));

		resources_[key] = std::weak_ptr<Resource>(resource);	// Stores a weak reference for caching reasons.

		//  This cast is safe as long as the virtual LoadResource is implemented properly!
		return static_pointer_cast<TResource>(resource);
		
	}

	template <typename TResource, typename TBundle, typename no_cache<TBundle>::type*>
	shared_ptr<TResource> Resources::Load(const typename TBundle & bundle){

		// Uncached version

		// Load the actual resource
		auto resource = shared_ptr<Resource>(std::move(Load(type_index(typeid(TResource)),
															std::type_index(typeid(TBundle)), 
															&bundle)));

		//  This cast is safe as long as the virtual LoadResource is implemented properly!
		return static_pointer_cast<TResource>(resource);

	}

}