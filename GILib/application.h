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

#include "time.h"

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
	
	/// \brief Expose methods to intialize, finalize and update a window.
	/// \remarks Interface.
	class WindowProcedure{

	public:

		/// \brief Initialize the window logic.
		virtual void Initialize() = 0;

		/// \brief Finalize the window logic.
		virtual void Dispose() = 0;

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

	};

	/// \brief The application singleton
	class Application{

	public:

		/// \brief A window.
		class Window{

			friend class Application;

		public:

#ifdef _WIN32

			/// \brief Window handle type under Windows.
			typedef HWND Handle;

#endif

			/// \brief Create a new window.
			/// \param procedure The procedure which manages the window's logic.
			/// \remarks The window is created with default style and dimensions.
			Window(WindowProcedure & procedure);

			~Window();

			/// \brief Get the window's handle.
			/// \return Returns a constant reference to the window's handle.
			inline const Handle & GetHandle() const{

				return handle_;

			}

			/// \brief Set the window's title.
			/// \param title The title to show in the title bar.
			void SetTitle(const wstring & title);

		private:

			/// \brief Initialize the window logic.
			inline void Initialize(){

				procedure_.Initialize();

			}

			/// \brief Update the window logic.
			/// \param time The application-coherent time.
			inline void Update(const Timer::Time & time){

				procedure_.Update(time);

			}

			WindowProcedure & procedure_;

			Handle handle_;

#ifdef _WIN32

			/// \brief Handle a Windows message.
			/// \param message_id The message.
			/// \param wparameter Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
			/// \param lparameterAdditional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
			/// \return The return value specifies the result of the message processing and depends on the message sent.
			inline LRESULT ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter){

				return procedure_.ReceiveMessage(message_id, wparameter, lparameter);

			}

#endif

		};

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
		/// The new window instance is initialized as well.
		/// \param procedure The procedure to bind to the new window.
		/// \return Returns a weak pointer to the new window.
		weak_ptr<Window> CreateWindow(WindowProcedure & procedure);

		/// /brief Dispose an existing window.

		/// If a window is destroyed, handle is invalidated.
		/// /param handle The handle of the window do dispose
		void DisposeWindow(Window::Handle & handle);

		/// \brief Get a window by handle.
		/// \param handle The handle of the window to find.
		/// \return Returns a weak pointer to the matching window. Returns an empty pointer if no match was found.
		weak_ptr<Window> GetWindow(const Window::Handle & handle);

		/// \brief Get a window by handle.
		/// \param handle The handle of the window to find.
		/// \return Returns a constant weak pointer to the matching window. Returns an empty pointer if no match was found.
		weak_ptr<const Window> GetWindow(const Window::Handle & handle) const;

		/// \brief Wait until all the windows get closed.
		void Join();

	private:

		Application();

		map<Window::Handle, shared_ptr<Window>> windows_;
		
#ifdef _WIN32

		static LRESULT __stdcall ReceiveMessage(HWND window_handle, unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

		void RegisterWindowClass();

		void UnregisterWindowClass();

		HICON window_icon_;

#endif

	};

}