/// \file core.h
/// \brief Classes and methods to manage the backbone of an application.
///
/// \author Raffaele D. Facendola

#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32

#include <Windows.h>

#endif

#include "observable.h"

using ::std::shared_ptr;
using ::std::weak_ptr;
using ::std::unique_ptr;
using ::std::map;
using ::std::string;
using ::std::wstring;
using ::std::vector;

namespace gi_lib{
	
	class System;
	class Application;
	class Window;
	class Time;
	
#ifdef _WIN32

	/// \brief Type used to identify univocally a window instance.
	using WindowHandle = HWND;

#endif

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

		WINDOWS		///< Windows OS

	};

	/// \brief Exposes methods to query system's capabilities.
	/// \author Raffaele D. Facendola
	class System{

	public:

		System() = delete;

		/// \brief Get the current operating system

		/// \return Returns the current operating system
		static OperatingSystem GetOperatingSystem();

		/// \brief Get the CPU capabilities.

		/// \return Returns the CPU capabilities.
		static CpuProfile GetCPUProfile();

		/// \brief Get the memory capabilities.

		/// \return Returns the memory capabilities.
		static MemoryProfile GetMemoryProfile();

		/// \brief Get informations about storage media

		/// \return Returns informations about storage media.
		static StorageProfile GetStorageProfile();

		/// \brief Get informations about user's desktop.

		/// \return Returns informations about user's desktop.
		static DesktopProfile GetDesktopProfile();

	};

	// \brief Exposes Input\Output methods.
	// \author Raffaele D. Facendola
	class IO{

	public:

		IO() = delete;

		// \brief Read an entire file.
		
		// \return Returns a string filled with the file content.
		static string ReadFile(const wstring& file_name);

	};

	/// \brief Manages the application instance.
	/// \author Raffaele D. Facendola
	class Application{

	public:
		
		/// \brief Default destructor.
		~Application();

		/// \brief Get the application singleton.
		/// \return Returns a reference to the application singleton.
		static Application & GetInstance();

		/// \brief Get the application directory.
		/// \return Returns the application directory.
		static wstring GetDirectory();

		/// \brief Strips the file name out of a file path.
		/// \param file_name Full file path.
		/// \return Returns the file path stripped of the file name and file extension.
		/// \return The trailing slash is always included.
		static wstring GetBaseDirectory(const wstring & file_name);

		/// \brief Get the full application path.
		/// \return Returns the full application path.
		static wstring GetPath();

		/// \brief Get the application name.
		/// \param extension Set whether the extension should be returned or not.
		/// \return Returns the application name including its extension eventually.
		static wstring GetName(bool extension = true);

		/// \brief Create and add new window.

		/// Create a new window with default style and dimensions.
		/// \tparam TWindow Type of the window to create. It must derive from Window.
		/// \tparam TArgs Type of the arguments to pass to the new instance's constructor.
		/// \param arguments Arguments to pass to the new instance's constructor.
		/// \return Returns a weak pointer to the new window.
		template<typename TWindow, typename... TArgs>
		typename std::enable_if_t< std::is_base_of<Window, TWindow>::value, weak_ptr<TWindow>> 
			AddWindow(TArgs&&... arguments);

		/// \brief Get a window by handle.
		/// \param handle The handle of the window to find.
		/// \return Returns a weak pointer to the matching window. Returns an empty pointer if no match was found.
		weak_ptr<Window> GetWindow(const WindowHandle & handle);

		/// /brief Dispose an existing window.

		/// If a window is destroyed the handle is no longer valid.
		/// /param handle The handle of the window do dispose
		void DisposeWindow(const WindowHandle & handle);

		/// \brief Wait until all the windows get closed.
		void Join();

	private:

		Application();

		map<WindowHandle, shared_ptr<Window>> windows_;
		
	};

	/// \brief A window.
	/// \author Raffaele D. Facendola
	class Window{

		friend class Application;

	public:

		struct OnClosedEventArgs{

			Window* window;

		};

		struct OnResizedEventArgs{

			Window* window;

			unsigned int width;

			unsigned height;

		};

		/// \brief Create a new window.
		/// \remarks The window is created with default style and dimensions.
		Window();

		virtual ~Window();

		/// \brief Get the window's handle.
		/// \return Returns a constant reference to the window's handle.
		const WindowHandle& GetHandle() const;

		/// \brief Set the window's title.
		/// \param title The title to show in the title bar.
		void SetTitle(const wstring & title);

		/// \brief Show or hide the window.
		/// \param show Shows the window if "true", hides it otherwise.
		void Show(bool show = true);

		/// \brief Check whether this windows is visible or not.
		/// \return Returns true if the window is not minimized, false otherwise.
		bool IsVisible();

		/// \brief Event fired when the window has been closed.
		/// \return Returns an observable event which notifies when the window is closed.
		Observable<OnClosedEventArgs> & OnClosed();

		/// \brief Event fired when the window has been resized.
		/// \return Returns an observable event which notifies when the window is resized.
		Observable<OnResizedEventArgs> & OnResized();
		
	protected:

		/// \brief The window has been closed.
		Event<OnClosedEventArgs> on_closed_;

		/// \brief The window has been resized.
		Event<OnResizedEventArgs> on_resized_;

	private:

		/// \brief Update the window logic.
		/// \param time The application-coherent time.
		virtual void Update(const Time& time) = 0;

		WindowHandle handle_;

#ifdef _WIN32

		class WindowClass;

		/// \brief Handle a Windows message.
		/// \param message_id The message.
		/// \param wparameter Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
		/// \param lparameterAdditional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
		/// \return The return value specifies the result of the message processing and depends on the message sent.
		virtual LRESULT ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

#endif

	};

	//////////////////////////////////// APPLICATION /////////////////////////////////////

	template<typename TWindow, typename... TArgs>
	typename std::enable_if_t< std::is_base_of<Window, TWindow>::value, weak_ptr<TWindow>> 
	Application::AddWindow(TArgs&&... arguments){

		auto window = std::make_shared<TWindow>(std::forward<TArgs>(arguments)...);

		windows_[window->GetHandle()] = window;

		return window;

	}

	//////////////////////////////////// WINDOW /////////////////////////////////////

	inline const WindowHandle& Window::GetHandle() const{

		return handle_;

	}

	inline Observable<Window::OnClosedEventArgs>& Window::OnClosed(){

		return on_closed_;

	}

	inline Observable<Window::OnResizedEventArgs>& Window::OnResized(){

		return on_resized_;

	}

}