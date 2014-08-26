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

	class Graphics;
	class Output;
	class Manager;

	/// \brief Describes a video mode.
	struct VideoMode{

		unsigned int horizontal_resolution;		///< Horizontal resolution, in pixels.
		unsigned int vertical_resolution;		///< Vertical resolution, in pixels.
		unsigned int refresh_rate;				///< Refresh rate, in Hz.

	};

	/// \brief Enumeration of all supported anti-aliasing techniques.
	enum class AntialiasingMode{

		NONE,			///< No antialiasing.
		MSAA_2X,		///< Multisample antialiasing, 2X.
		MSAA_4X,		///< Multisample antialiasing, 4X.
		MSAA_8X,		///< Multisample antialiasing, 8X.
		MSAA_16X,		///< Multisample antialiasing, 16X.

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

	/// \brief Factory interface used to create and initialize the graphical subsystem.
	/// \author Raffaele D. Facendola
	class Graphics{

	public:

		/// \brief Default destructor;
		virtual ~Graphics(){}

		/// \brief Get the video card's parameters and capabilities.
		virtual AdapterProfile GetAdapterProfile() const = 0;

		/// \brief Initialize an output.
		/// \param window The window used to display the output.
		/// \return Returns a pointer to the new output.
		virtual unique_ptr<Output> CreateOutput(Window & window) = 0;

		/// \brief Get the resource manager.
		/// \return Returns the resource manager.
		virtual Manager & GetManager() = 0;

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

		/// \brief Finalize the current frame and deliver it on the output.
		virtual void Commit() = 0;

	};

	/// \brief Resource manager interface.
	/// \author Raffaele D. Facendola.
	class Manager{

	public:

		/// \brief Folder where the resources are stored.
		static const wstring kResourceFolder;

		/// \brief Default constructor.
		Manager();

		/// \brief Default destructor;
		~Manager(){};

		/// \brief Load an immutable resource.
		/// \tparam Type of resource to load.
		/// \param path The path of the resource.
		/// \param extras Extra loading parameters.
		/// \return Return an handle to the specified resource. Throws if no resource is found.
		template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type* = nullptr>
		shared_ptr<TResource> Load(const wstring & path, const typename TResource::Extra & extras);

		/// \brief Load an immutable resource.
		/// \tparam Type of resource to load.
		/// \param path The path of the resource.
		/// \return Return an handle to the specified resource. Throws if no resource is found.
		template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type* = nullptr>
		shared_ptr<TResource> Load(const wstring & path);

		/// \brief Get the amount of memory used by the resources loaded.
		size_t GetSize();

	protected:

		/// \brief Type of resources' keys.
		using ResourceKey = std::pair < std::type_index, wstring >;

		/// \brief Type of resources' map.
		using ResourceMap = map < ResourceKey, weak_ptr<Resource> >;

		/// \brief Load a resource.

		/// \param key Unique key of the resource to load.
		/// \param extras Extra parameters.
		/// \return Returns a pointer to the loaded resource
		virtual unique_ptr<Resource> LoadDirect(const ResourceKey & key, const void * extras) = 0;

	private:

		template <typename TResource>
		shared_ptr<TResource> LoadExtra(const wstring & path, const void * extras);

		// Map of the immutable resources
		ResourceMap resources_;

		// Base path for the resources
		wstring base_path_;

	};

	//

	template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type*>
	inline shared_ptr<TResource> Manager::Load(const wstring & path, const typename TResource::Extra & extras){

		return LoadExtra<TResource>(path, &extras);

	}

	template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type*>
	inline shared_ptr<TResource> Manager::Load(const wstring & path){

		return LoadExtra<TResource>(path, nullptr);

	}

	template <typename TResource>
	shared_ptr<TResource> Manager::LoadExtra(const wstring & path, const void * extras){

		//Check if the resource exists inside the map
		auto key = make_pair(std::type_index(typeid(TResource)), base_path_ + path);

		auto it = resources_.find(key);

		if (it != resources_.end()){

			if (auto resource = it->second.lock()){

				return static_pointer_cast<TResource>(resource);

			}

			//Resource was expired...

		}

		auto resource = shared_ptr<Resource>(std::move(LoadDirect(key, extras)));		// To shared ptr

		resources_[key] = resource;														// To weak ptr

		return static_pointer_cast<TResource>(resource);								// To shared ptr (of the requested type). Evil downcasting :D

	}

}