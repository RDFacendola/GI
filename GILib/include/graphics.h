/// \file graphics.h
/// \brief Define base interface for the graphics subsystem.
///
/// \author Raffaele D. Facendola

#pragma once

namespace gi_lib{
	
	struct VideoMode;
	enum class AntialiasingMode;

	/// \brief Interface used to display an image to an output.
	/// \remarks Interface.
	/// \author Raffaele D. Facendola
	class Graphics{

	public:
		
		virtual ~Graphics(){}

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