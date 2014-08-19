/// \file application.h
/// \brief Classes and methods to manage the backbone of an application.
///
/// \author Raffaele D. Facendola

#pragma once

#include <memory>
#include <map>

#include "window.h"

using ::std::shared_ptr;
using ::std::map;

namespace gi_lib{
	
	/// \brief The application singleton
	/// \author Raffaele D. Facendola
	class Application{

	public:

		/// \brief Default destructor.
		~Application();

		/// \brief Get the application singleton.
		/// \return Returns a reference to the application singleton.
		static Application & GetInstance();

		/// \brief Get the full application path.
		/// \return Returns the full application path.
		wstring GetPath() const;

		/// \brief Get the application name.
		/// \param extension Set whether the extension should be returned or not.
		/// \return Returns the application name including its extension eventually.
		wstring GetName(bool extension = true) const;

		/// \brief Create and add new window.

		/// Create a new window with default style and dimensions.
		/// \tparam TWindow Type of the window to create. It must derive from Window.
		/// \tparam TArgs Type of the arguments to pass to the new instance's constructor.
		/// \param arguments Arguments to pass to the new instance's constructor.
		/// \return Returns a weak pointer to the new window.
		template<typename TWindow, typename... TArgs>
		typename std::enable_if_t< std::is_base_of<Window, TWindow>::value, weak_ptr<TWindow>> AddWindow(TArgs&&... arguments);

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

	//

	template<typename TWindow, typename... TArgs>
	typename std::enable_if_t< std::is_base_of<Window, TWindow>::value, weak_ptr<TWindow>> Application::AddWindow(TArgs&&... arguments){

		auto window = std::make_shared<TWindow>(std::forward<TArgs>(arguments)...);

		windows_[window->GetHandle()] = window;

		return window;

	}

}