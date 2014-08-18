/// \file factory.h
/// \brief Abstract factory classes used to initialize various graphical subsystems.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <vector>
#include <memory>

using ::std::wstring;
using ::std::vector;
using ::std::unique_ptr;

namespace gi_lib{

	class Window;
	class Graphics;
	class Resources;

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
	class Factory{

	public:

		/// \brief Get the video card's parameters and capabilities.
		virtual AdapterProfile GetAdapterProfile() const = 0;

		/// \brief Create a graphics subsystem.
		/// \param window The window used to display the frames.
		/// \return Returns a reference to the graphic subsystem.
		virtual unique_ptr<Graphics> CreateGraphics(Window & window) = 0;

		/// \brief Get the resource manager.
		/// \return Returns a reference to the resource mananger.
		virtual Resources & GetResources() = 0;

	};

}