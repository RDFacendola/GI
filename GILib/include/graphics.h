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
#include "resource_traits.h"

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

	/// \brief Enumeration of all supported graphical API.
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

		/// \brief Set the antialiasing mode.
		/// \param antialiasing_mode The antialiasing technique to activate.
		virtual void SetAntialisingMode(const AntialiasingMode & antialiasing_mode) = 0;

		/// \brief Get the current antialiasing mode.
		/// \return Returns the current antialiasing mode.
		virtual const AntialiasingMode & GetAntialisingMode() const = 0;

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

		/// \brief Draw the specified scene and commit.
		virtual void Draw(Scene & scene) = 0;

		/// \brief Get the render target associated to this output.
		/// \return Returns the render target associated to this output.
		virtual shared_ptr<RenderTarget> GetRenderTarget() = 0;

	};

	/// \brief Resource manager interface.
	/// \author Raffaele D. Facendola.
	class Manager{

	public:

		/// \brief Filename of the built-in phong shader.
		static wchar_t * kPhongShaderFile;

		/// \brief Default constructor.
		Manager();

		/// \brief Default destructor;
		~Manager(){};

		/// \brief Load a resource.
		/// \tparam Type of resource to load.
		/// \param settings In depth load settings.
		/// \return Return an handle to the specified resource. Throws if no resource is found.
		template <typename TResource, typename TResource::LoadMode kLoadMode>
		std::enable_if_t<std::is_base_of<Resource, TResource>::value, shared_ptr<TResource> > Load(const typename LoadSettings<TResource, kLoadMode> & settings);

		/// \brief Create a resource.
		/// \tparam TResource Type of the resource to load.
		/// \param settings The creation settings.
		/// \return Returns a pointer to the new resource.
		template <typename TResource, typename TResource::BuildMode kBuildMode>
		std::enable_if_t<std::is_base_of<Resource, TResource>::value, shared_ptr<TResource> > Build(const typename BuildSettings<TResource, kBuildMode> & settings);

		/// \brief Get the amount of memory used by the resources loaded.
		size_t GetSize();

	protected:

		/// \brief Used as key to address the resource map flyweight.
		struct LoadKey{

			/// \brief Type index of the load mode.

			/// Type_index is guaranteed to be unique among different resources and load modes.
			type_index key;

			/// \brief Unique tag associated to a particular tag configuration.
			char tag[16];

			/// \brief Default constructor.
			LoadKey();

			/// \brief Lesser comparison operator.
			bool operator<(const LoadKey & other) const;
			
		};

		/// \brief Type of resource map values.
		using LoadValue =  weak_ptr < Resource >;

		/// \brief Type of resource map. 
		using ResourceMap = map < LoadKey, LoadValue >;

		/// \brief Load a resource.
		/// \param resource_type Resource's type index.
		/// \param load_mode Load mode index.
		/// \param settings Raw pointer to the load settings.
		/// \return Returns a pointer to the loaded resource
		virtual unique_ptr<Resource> LoadResource(const type_index & resource_type, int load_mode, const void * settings) = 0;

		/// \brief Build a resource.
		/// \param resource_type Resource's type index.
		/// \param build_mode Build mode index.
		/// \param settings Raw pointer to the build settings.
		/// \return Returns a pointer to the built resource
		virtual unique_ptr<Resource> BuildResource(const type_index & resource_type, int build_mode, const void * settings) = 0;

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
		static Graphics & GetAPI(API api);

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
		virtual Manager & GetManager() = 0;

	};

	//

	template <typename TResource, typename TResource::LoadMode kLoadMode>
	std::enable_if_t<std::is_base_of<Resource, TResource>::value, shared_ptr<TResource> > Manager::Load(const typename LoadSettings<TResource, kLoadMode> & settings){

		//Fill the key object with the type index and the tag
		LoadKey key;

		key.key = std::type_index(typeid(settings));

		settings.FillTag(key.tag, sizeof(key.tag));

		auto it = resources_.find(key);

		if (it != resources_.end()){

			if (auto resource = it->second.lock()){

				return static_pointer_cast<TResource>(resource);

			}

			//Resource was expired...

		}
		

		// Load the actual resource
		auto resource = shared_ptr<Resource>(std::move(LoadResource(type_index(typeid(TResource)),
																	static_cast<int>(kLoadMode), 
																	&settings)));


		resources_[key] = std::weak_ptr<Resource>(resource);	// To weak ptr

		//  This cast should be safe...
		return static_pointer_cast<TResource>(resource);
		
	}

	template <typename TResource, typename TResource::BuildMode kBuildMode>
	std::enable_if_t<std::is_base_of<Resource, TResource>::value, shared_ptr<TResource> > Manager::Build(const typename BuildSettings<TResource, kBuildMode> & settings){
		
		auto resource = shared_ptr<Resource>(std::move(BuildResource(type_index(typeid(TResource)),
													   static_cast<int>(kBuildMode),
													   &settings)));

		//  This cast should be safe...
		return static_pointer_cast<TResource>(resource);
	}

}