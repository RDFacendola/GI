/// \file win_core.h
/// \brief Classes and methods to manage the backbone of an application under windows.
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

#pragma once

#include <Windows.h>

#include "..\core.h"

namespace gi_lib{

	namespace windows{

		class Window;

		/// \brief Exposes methods to query system's capabilities under Windows.
		/// \author Raffaele D. Facendola
		class System : public gi_lib::System{

		public:

			/// \brief Get the system singleton.
			/// \return Returns a reference to the system singleton.
			static System& GetInstance();

			virtual OperatingSystem GetOperatingSystem() const override;

			virtual CpuProfile GetCPUProfile() const override;

			virtual MemoryProfile GetMemoryProfile() const override;

			virtual StorageProfile GetStorageProfile() const override;

			virtual DesktopProfile GetDesktopProfile() const override;

		private:

			System();

		};

		// \brief Exposes file system-related methods under Windows.
		// \author Raffaele D. Facendola
		class FileSystem : public gi_lib::FileSystem{

		public:

			/// \brief Get the file system singleton.
			/// \return Returns a reference to the file system singleton.
			static FileSystem& GetInstance();

			virtual wstring GetDirectory(const wstring& file_name) const override;

			virtual wstring Read(const wstring& file_name) const override;

		private:

			FileSystem();

		};

		/// \brief A window under Windows, lel.
		/// \author Raffaele D. Facendola
		class Window : public gi_lib::Window{

		public:

			Window(unique_ptr<IWindowLogic> logic);

			virtual ~Window();
			
			virtual void SetTitle(const wstring& title) override;

			virtual void Show(bool show = true) override;

			virtual bool IsVisible() override;
			
			virtual void Destroy() override;

			/// \brief Get the window's handle.
			/// \return Returns a constant reference to the window's handle.
			HWND GetHandle() const;

			/// \brief Update the window
			void Update(const Time& time);

			/// \brief Send a Windows message.
			LRESULT Window::ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

		private:

			HWND handle_;				///< \brief The window handle.

		};

		/// \brief Manages the application instance under Windows.
		/// \author Raffaele D. Facendola
		class Application : public gi_lib::Application{

		public:

			/// \brief Get the application singleton.
			/// \return Returns a reference to the application singleton.
			static Application& GetInstance();

			virtual wstring GetPath() const override;

			virtual wstring GetDirectory() const override;

			virtual void Join() override;

			/// \brief Get a window by handle.
			/// \param handle The handle of the window to find.
			/// \return Returns a pointer to the window matching the given handle if any, or nullptr if none is found.
			Window* GetWindow(HWND handle);

			/// /brief Dispose an existing window.
			/// If a window is destroyed the handle is no longer valid.
			/// /param handle The handle of the window to dispose
			void DisposeWindow(HWND handle);

		protected:

			virtual Window& InstantiateWindow(unique_ptr<IWindowLogic> logic) override;

		private:

			Application();

			map<HWND, unique_ptr<Window>> windows_;		///< \brief Map of the windows instantiated so far.

		};
		
		///////////////////////////////////////// APPLICATION /////////////////////////////////////////

		inline Window* Application::GetWindow(HWND handle){

			auto it = windows_.find(handle);

			return it != windows_.end()?
				   it->second.get() :
				   nullptr;

		}

		inline void Application::DisposeWindow(HWND handle){

			auto it = windows_.find(handle);

			if (it != windows_.end()){

				// The unique_ptr will take care of window's destruction.

				windows_.erase(windows_.find(handle));

			}
			
		}

		///////////////////////////////////////// WINDOW //////////////////////////////////////////

		inline HWND Window::GetHandle() const{

			return handle_;

		}

		inline void Window::Update(const Time& time){

			logic_->Update(time);

		}

	}

}

#endif