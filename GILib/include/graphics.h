/// \file graphics.h
/// \brief Define base interface for the graphics subsystem.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "resources.h"

using ::std::wstring;
using ::std::vector;
using ::std::unique_ptr;

namespace gi_lib{
	
	class IGraphics;
	class Window;

	/// \brief Describes a video mode.
	/// \author Raffaele D. Facendola
	struct VideoMode{

		unsigned int horizontal_resolution;		///< Horizontal resolution, in pixels.
		unsigned int vertical_resolution;		///< Vertical resolution, in pixels.
		unsigned int refresh_rate;				///< Refresh rate, in Hz.

	};

	/// \brief Enumeration of all supported anti-aliasing techniques.
	/// \author Raffaele D. Facendola
	enum class AntialiasingMode{

		NONE,			///< No antialiasing.
		MSAA_2X,		///< Multisample antialiasing, 2X.
		MSAA_4X,		///< Multisample antialiasing, 4X.
		MSAA_8X,		///< Multisample antialiasing, 8X.
		MSAA_16X,		///< Multisample antialiasing, 16X.

	};

	/// \brief Describes the video card's parameters and capabilities.
	/// \author Raffaele D. Facendola
	struct AdapterProfile{

		wstring name;										///< Name of the video card.
		size_t dedicated_memory;							///< Dedicated memory, in bytes.
		size_t shared_memory;								///< Shared memory, in bytes.		
		vector<VideoMode> video_modes;						///< List of supported video modes.
		vector<AntialiasingMode> antialiasing_modes;		///< List of supported antialiasing modes.

	};

	/// \brief Common interface for graphics API.

	/// A factory is used to instantiate the basic objects needed by various API.
	/// \remarks Interface.
	/// \author Raffaele D. Facendola
	class IFactory{

	public:

		/// \brief Get the video card's parameters and capabilities.
		virtual AdapterProfile GetAdapterProfile() const = 0;

		/// \brief Create a graphics subsystem.
		/// \param window The window used to display the frames.
		/// \return Returns a reference to the graphic subsystem.
		virtual unique_ptr<IGraphics> CreateGraphics(Window & window) = 0;

		/// \brief Get the resource manager.
		/// \return Returns a reference to the resource mananger.
		virtual Resources & GetResources() = 0;

	};

	/// \brief Interface used to display an image to an output.
	/// \remarks Interface.
	/// \author Raffaele D. Facendola
	class IGraphics{

	public:

		virtual ~IGraphics(){}

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

}