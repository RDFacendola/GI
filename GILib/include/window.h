/// \file window.h
/// \brief Classes and methods to manage a window.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <Windows.h>

#endif

#include <string>

#include "observable.h"

using ::std::wstring;

namespace gi_lib{

	class Time;

#ifdef _WIN32

	/// \brief Window handle type.
	using WindowHandle = HWND;

#endif

	/// \brief A window.
	/// \author Raffaele D. Facendola
	class Window{

		friend class Application;

	public:
		
		/// \brief Create a new window.
		/// \remarks The window is created with default style and dimensions.
		Window();

		virtual ~Window();

		/// \brief Get the window's handle.
		/// \return Returns a constant reference to the window's handle.
		inline const WindowHandle & GetHandle() const{

			return handle_;

		}

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
		inline Observable<Window&> & OnClosed(){

			return on_closed_;

		}

		/// \brief Event fired when the window has been resized.
		/// \return Returns an observable event which notifies when the window is resized.
		inline Observable<Window&, unsigned int, unsigned int> & OnResized(){

			return on_resized_;

		}

	protected:

		/// \brief The window has been closed.
		Event<Window &> on_closed_;

		/// \brief The window has been resized.
		Event<Window &, unsigned int, unsigned int> on_resized_;

	private:

		/// \brief Update the window logic.
		/// \param time The application-coherent time.
		virtual void Update(const Time & time) = 0;

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

}