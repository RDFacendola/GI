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

#include "time.h"

using ::std::wstring;
using ::std::map;
using ::std::unique_ptr;

#ifdef _WIN32

// Under Windows this is a macro which clashes with Application::CreateWindow method. Just disable it.
#undef CreateWindow

#endif

namespace gi_lib{

	class WindowProcedure;

	/// \brief The application singleton
	class Application{

	public:

		/// \brief An application window
		class Window{

		public:

#ifdef _WIN32

			/// \brief Window handle type under Windows.
			typedef HWND Handle;

#else

			///

#endif

#ifdef _WIN32

			/// \brief Create a new named window.
			/// \param procedure 
			/// \param instance A handle to the instance of the module to be associated with the window.
			/// \param name Window's name. It must be unique.
			/// \remarks The window is created with default style and dimensions.
			Window(WindowProcedure & procedure, HINSTANCE instance, wstring name);

			/// \brief Create a new named window.
			/// \param instance A handle to the instance of the module to be associated with the window.
			/// \param name Window's name. It must be unique.
			/// \param width The width of the window, in pixel.
			/// \param height The height of the window, in pixel.
			/// \remarks The window is created with a default style.
			Window(WindowProcedure & procedure, HINSTANCE instance, wstring name, unsigned int width, unsigned int height);

#else

			///

#endif

			virtual ~Window();

			/// \brief Get the window's handle.
			/// \return Returns a constant reference to the window's handle.
			inline const Handle & GetHandle() const{

				return handle_;
				
			}

			/// \brief Get the window name.
			/// \return Returns a constant reference to the window's name.
			inline const wstring & GetName() const{

				return name_;

			}

			/// \brief Set the window's title.
			/// \param title The title to show in the title bar.
			void SetTitle(const wstring & title);

			/// \brief Close the window.
			void Close();

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

#ifdef _WIN32

			/// \brief Handle a Windows message.
			/// \param message_id The message.
			/// \param wparameter Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
			/// \param lparameterAdditional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
			/// \return The return value specifies the result of the message processing and depends on the message sent.
			inline LRESULT ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter){
		
				return procedure_.ReceiveMessage(message_id, wparameter, lparameter);
		
			}

#else

			///

#endif

			WindowProcedure & procedure_;

			Handle handle_;

			wstring name_;
			
		};

		/// \brief Get the application singleton
		/// \return Returns a reference to the application singleton
		inline Application & GetInstance(){

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
		/// \param procedure The procedure to bind to the new window.
		/// \return Returns the reference to the created window.
		template <typename TWindow>
		TWindow & CreateWindow(WindowProcedure & procedure);

		/// \brief Get a window by handle.
		/// \param handle The handle of the window to find.
		/// \return Returns a reference to the window matching the given handle.
		Window & GetWindow(const Window::Handle & handle);

		/// \brief Get a window by name.
		/// \param name The name of the window to find.
		/// \return Returns a reference to the window matching the given name.
		Window & GetWindow(const wstring & name);

		/// \brief Get a window by handle.
		/// \param handle The handle of the window to find.
		/// \return Returns a constant reference to the window matching the given handle.
		const Window & GetWindow(const Window::Handle & handle) const;

		/// \brief Get a window by name.
		/// \param name The name of the window to find.
		/// \return Returns a constant reference to the window matching the given name.
		const Window & GetWindow(const wstring & name) const;

		/// \brief Wait until all the windows get closed.
		void Join();

	private:

		Application(){}

		map<Window::Handle, unique_ptr<Window>> windows_;

	};

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
		LRESULT ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

#else

		///

#endif

	};

}