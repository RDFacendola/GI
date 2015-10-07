/// \file core.h
/// \brief Classes and methods to manage the backbone of an application.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <vector>

#include "observable.h"
#include "input.h"

using ::std::wstring;
using ::std::vector;

namespace gi_lib{
	
	class System;
	class FileSystem;
	class Application;
	class IWindowLogic;
	class Window;
	class Time;
	
	/// \brief Describes the CPU's capabilities.
	/// \author Raffaele D. Facendola
	struct CpuProfile{

		unsigned int cores;					///< Number of logical cores.
		unsigned long long frequency;		///< Frequency of each core in Hz.
		
	};

	/// \brief Describes a particular drive.
	/// \author Raffaele D. Facendola
	struct DriveProfile{

		unsigned long long size;			///< Total space, in bytes.
		unsigned long long available_space; ///< Available space, in bytes.
		wstring unit_letter;				///< Unit letter.

	};

	/// \brief Describes the storage capabilities.
	/// \author Raffaele D. Facendola
	struct StorageProfile{

		vector<DriveProfile> fixed_drives;	///< Vector of all fixed drives' profiles.

	};

	/// \brief Describes the system memory.
	/// \author Raffaele D. Facendola
	struct MemoryProfile{

		unsigned long long total_physical_memory;		///< Total physical memory, in bytes.
		unsigned long long total_virtual_memory;		///< Total virtual address space for the current process, in bytes.
		unsigned long long total_page_memory;			///< Total page memory, in bytes.
		unsigned long long available_physical_memory;	///< Available physical memory, in bytes.
		unsigned long long available_virtual_memory;	///< Available virtual address space for the current process, in bytes.
		unsigned long long available_page_memory;		///< Available page memory, in bytes.

	};

	/// \brief Describes the desktop.
	/// \author Raffaele D. Facendola
	struct DesktopProfile{

		unsigned int width;			///< Horizontal resolution of the dekstop.
		unsigned int height;		///< Vertical resolution of the desktop.
		unsigned int refresh_rate;	///< Refresh rate, in Hz.

	};

	/// \brief Operating system.
	/// \author Raffaele D. Facendola
	enum class OperatingSystem{

		Windows		///< Windows OS

	};

	/// \brief Exposes methods to query system's capabilities.
	/// \author Raffaele D. Facendola
	class System{

	public:

		/// \brief Get the system singleton.
		/// \return Returns a reference to the system singleton.
		static System& GetInstance();

		/// \brief Get the current operating system
		/// \return Returns the current operating system
		virtual OperatingSystem GetOperatingSystem() const = 0;

		/// \brief Get the CPU capabilities.

		/// \return Returns the CPU capabilities.
		virtual CpuProfile GetCPUProfile() const = 0;

		/// \brief Get the memory capabilities.

		/// \return Returns the memory capabilities.
		virtual MemoryProfile GetMemoryProfile() const = 0;

		/// \brief Get informations about storage media

		/// \return Returns informations about storage media.
		virtual StorageProfile GetStorageProfile() const = 0;

		/// \brief Get informations about user's desktop.

		/// \return Returns informations about user's desktop.
		virtual DesktopProfile GetDesktopProfile() const = 0;

	};

	// \brief Exposes file system-related methods.
	// \author Raffaele D. Facendola
	class FileSystem{

	public:

		/// \brief Get the file system singleton.
		/// \return Returns a reference to the file system singleton.
		static FileSystem& GetInstance();

		/// \brief Get the directory part of a full path.
		/// \param file_name File name.
		/// \return Returns the directory path of the specified file.
		virtual wstring GetDirectory(const wstring& file_name) const = 0;

		/// \brief Read the content of a file.
		/// \param file_name File to read.
		/// \return Returns the content of the specified file.
		virtual wstring Read(const wstring& file_name) const = 0;

	};

	/// \brief Manages the application instance.
	/// \author Raffaele D. Facendola
	class Application{

	public:
		
		/// \brief Get the application singleton.
		/// \return Returns a reference to the application singleton.
		static Application& GetInstance();

		/// \brief Get the application path.
		/// \return Returns the application path.
		virtual wstring GetPath() const = 0;

		/// \brief Get the application directory.
		/// \return Returns the application directory.
		virtual wstring GetDirectory() const = 0;
		
		/// \brief Wait until all the windows get closed.
		virtual void Join() = 0;

		/// \brief Create a new window.
		/// Create a new window with default style and dimensions.
		/// \tparam TWindowLogic Type of the window logic associated to the window. Must derive from IWindowLogic.
		/// \tparam TArguments Type of the arguments to pass to the logic's constructor.
		/// \param arguments Arguments to pass to the logic's constructor.
		/// \return Returns a reference to the new window.
		template<typename TWindowLogic, typename... TArguments>
		Window& AddWindow(TArguments&&... arguments);

	protected:

		/// \brief Create a new window.
		/// \param logic Logic to associated to the window.
		/// \return Returns a reference to the created window.
		virtual Window& InstantiateWindow(unique_ptr<IWindowLogic> logic) = 0;

	};

	/// \brief Represents the core logic of the application.
	/// \author Raffaele D. Facendola.
	class IWindowLogic{

	public:

		/// \brief Initialize the window logic.
		/// \param window The window associated to this logic.
		virtual void Initialize(Window& window) = 0;

		/// \brief Update the window logic.
		/// \param time The application time.
		virtual void Update(const Time& time) = 0;

		/// \brief Virtual destructor.
		virtual ~IWindowLogic();

	};

	/// \brief A window.
	/// \author Raffaele D. Facendola
	class Window{

	public:

		struct OnClosedEventArgs{

			Window* window;

		};

		struct OnResizedEventArgs{

			Window* window;

			unsigned int width;

			unsigned height;

		};
				
		/// \brief Create a new windows instance.
		/// \tparam TWindowLogic Type of the logic associated to the window. Must derive from IWindowLogic.
		/// \tparam TArguments Type of the arguments that will be passed to the window logic's constructor.
		/// \param arguments Arguments that will be passed to the window logic's constructor.
		/// \return Returns 
		template <typename TWindowLogic, typename... TArguments>
		static Window& CreateInstance(TArguments&&... arguments);

		/// \brief Create a new window.
		/// \remarks The window is created with default style and dimensions.
		Window(unique_ptr<IWindowLogic> logic);

		virtual ~Window();

		/// \brief Set the window's title.
		/// \param title The title to show in the title bar.
		virtual void SetTitle(const wstring& title) = 0;

		/// \brief Show or hide the window.
		/// \param show Shows the window if "true", hides it otherwise.
		virtual void Show(bool show = true) = 0;

		/// \brief Check whether this windows is visible or not.
		/// \return Returns true if the window is not minimized, false otherwise.
		virtual bool IsVisible() = 0;

		/// \brief Destroy this window.
		virtual void Destroy() = 0;

		/// \brief Get the input interface for this window.
		/// \return Returns the input interface.
		virtual const IInput& GetInput() const = 0;

		/// \brief Event fired when the window has been closed.
		/// \return Returns an observable event which notifies when the window is closed.
		virtual Observable<OnClosedEventArgs>& OnClosed();

		/// \brief Event fired when the window has been resized.
		/// \return Returns an observable event which notifies when the window is resized.
		virtual Observable<OnResizedEventArgs>& OnResized();
		
	protected:

		Event<OnClosedEventArgs> on_closed_;		///< \brief Fired when the window get closed.

		Event<OnResizedEventArgs> on_resized_;		///< \brief Fired when the window get resized.
			
		unique_ptr<IWindowLogic> logic_;			///< \brief Pointer to the internal window logic.

	};

	//////////////////////////////////// APPLICATION /////////////////////////////////////

	template<typename TWindowLogic, typename... TArguments>
	Window& Application::AddWindow(TArguments&&... arguments){

		unique_ptr<IWindowLogic> logic_ptr = make_unique<TWindowLogic>(std::forward<TArguments&&>(arguments)...);

		auto& logic = *logic_ptr;

		auto& window = InstantiateWindow(std::move(logic_ptr));

		logic.Initialize(window);

		return window;

	}

	//////////////////////////////////// IWINDOW LOGIC //////////////////////////////

	inline IWindowLogic::~IWindowLogic() {}

	//////////////////////////////////// WINDOW /////////////////////////////////////

	inline Window::Window(unique_ptr<IWindowLogic> logic) :
	logic_(std::move(logic)){}

	inline Window::~Window(){
	
		logic_ = nullptr;

	}

	inline Observable<Window::OnClosedEventArgs>& Window::OnClosed(){

		return on_closed_;

	}

	inline Observable<Window::OnResizedEventArgs>& Window::OnResized(){

		return on_resized_;

	}

}