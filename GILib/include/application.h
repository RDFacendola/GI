/// \file application.h
/// \brief 
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <Windows.h>

#endif

#include <string>
#include <map>
#include <memory>
#include <functional>

#include "timer.h"

using ::std::wstring;
using ::std::map;
using ::std::shared_ptr;
using ::std::weak_ptr;
using ::std::function;

#ifdef _WIN32

// Under Windows this is a macro which clashes with Application::CreateWindow method. Just disable it.
#undef CreateWindow

#endif

namespace gi_lib{

	/// \brief A window.
	class Window{

		friend class Application;

	public:

#ifdef _WIN32

		/// \brief Window handle type under Windows.
		typedef HWND Handle;

#endif

		/// \brief Create a new window.
		/// \remarks The window is created with default style and dimensions.
		Window();

		virtual ~Window();

		/// \brief Get the window's handle.
		/// \return Returns a constant reference to the window's handle.
		inline const Handle & GetHandle() const{

			return handle_;

		}

		/// \brief Set the window's title.
		/// \param title The title to show in the title bar.
		void SetTitle(const wstring & title);

		/// \brief Show or hide the window.
		/// \param show Shows the window if "true", hides it otherwise.
		void Show(bool show = true);

	private:

		/// \brief Update the window logic.
		/// \param time The application-coherent time.
		virtual void Update(const Timer::Time & time) = 0;

#ifdef _WIN32

		/// \brief Handle a Windows message.
		/// \param message_id The message.
		/// \param wparameter Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
		/// \param lparameterAdditional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
		/// \return The return value specifies the result of the message processing and depends on the message sent.
		virtual LRESULT ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter) = 0;

#endif

		Handle handle_;

	};
	
	/// \brief The application singleton
	class Application{

	public:

		/// \brief Default destructor.
		~Application();

		/// \brief Get the application singleton.
		/// \return Returns a reference to the application singleton.
		static inline Application & GetInstance(){

			static Application application;

			return application;

		}

		/// \brief Get the full application path.
		/// \return Returns the full application path.
		wstring GetPath() const;

		/// \brief Get the application name.
		/// \param extension Set whether the extension should be returned or not.
		/// \return Returns the application name including its extension eventually.
		wstring GetName(bool extension = true) const;

		/// \brief Create a new window.

		/// Create a new window with default style and dimensions.
		/// \tparam TWindow Type of the window to create. It must derive from Window.
		/// \tparam TArgs Type of the arguments to pass to the new instance's constructor.
		/// \param arguments Arguments to pass to the new instance's constructor.
		/// \return Returns a weak pointer to the new window.
		template<typename TWindow, typename... TArgs>
		weak_ptr<TWindow> CreateWindow(TArgs&&... arguments){

			//Ensures that TWindow is derived from Window at compile time
			static_assert(typename std::is_base_of<Window, TWindow>::value, "TWindow must inherit from Window");

#ifdef _WIN32

			auto window = std::make_shared<TWindow>(arguments);

			windows_[window->GetHandle()] = window;

			return window;

#else

			static_assert(false, "Unsupported OS");

#endif
		}

		/// \brief Create a new window.

		/// Create a new window with default style and dimensions.
		/// \tparam TWindow Type of the window to create. It must derive from Window.
		/// \return Returns a weak pointer to the new window.
		template<typename TWindow>
		weak_ptr<TWindow> CreateWindow(){

			//Ensures that TWindow is derived from Window at compile time
			static_assert(typename std::is_base_of<Window, TWindow>::value, "TWindow must inherit from Window");

#ifdef _WIN32

			auto window = std::make_shared<TWindow>();

			windows_[window->GetHandle()] = window;

			return window;

#else

			static_assert(false, "Unsupported OS");

#endif
		}

		/// /brief Dispose an existing window.

		/// If a window is destroyed the handle is no longer valid.
		/// /param handle The handle of the window do dispose
		void DisposeWindow(const Window::Handle & handle);

		/// \brief Wait until all the windows get closed.
		void Join();

	private:

		Application();

		/// \brief Get a window by handle.
		/// \param handle The handle of the window to find.
		/// \return Returns a weak pointer to the matching window. Returns an empty pointer if no match was found.
		weak_ptr<Window> GetWindow(const Window::Handle & handle);
		
#ifdef _WIN32

		static LRESULT __stdcall ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

		void RegisterWindowClass();

		void UnregisterWindowClass();

		HICON window_icon_;

#endif

		map<Window::Handle, shared_ptr<Window>> windows_;
		
	};

}