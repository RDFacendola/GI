/// \file dx11graphics.h
/// \brief Declare classes and interfaces used to manage the core of DirectX 11 API.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include "factory.h"
#include "graphics.h"
#include "observable.h"

struct ID3D11Device;
struct IDXGIFactory;
struct DXGI_SWAP_CHAIN_DESC;
struct IDXGISwapChain;

namespace gi_lib{

	class Window;

	namespace dx11{

		/// \brief DirectX object used to display an image to an output.
		/// \author Raffaele D. Facendola
		class DX11Graphics : public Graphics{

		public:

			// Non copyable class
			DX11Graphics(const DX11Graphics &) = delete;
			DX11Graphics & operator=(const DX11Graphics &) = delete;

			/// \brief Create a new DirectX11 graphics object.
			/// \param window The window where the final image will be displayed.
			/// \param device The device used to create the resources.
			/// \param factory The factory used to create the swapchain.
			DX11Graphics(Window & window, ID3D11Device & device, IDXGIFactory & factory);

			/// \brief Default destructor.
			~DX11Graphics();

			virtual void SetVideoMode(const VideoMode & video_mode);

			inline virtual const VideoMode & GetVideoMode() const{

				return video_mode_;

			}

			virtual void SetAntialisingMode(const AntialiasingMode & antialiasing_mode);

			inline virtual const AntialiasingMode & GetAntialisingMode() const{

				return antialiasing_mode_;

			}

			virtual void SetFullscreen(bool fullscreen);

			inline virtual bool IsFullscreen() const{

				return fullscreen_;

			}

			inline virtual void SetVSync(bool vsync){

				vsync_ = vsync;

			}

			virtual bool IsVSync() const{

				return vsync_;

			}

			virtual void Commit();

		private:

			/// \brief Get the default swapchain mode description. 
			/// \return Returns the default swapchain mode description.
			DXGI_SWAP_CHAIN_DESC GetDefaultSwapchainMode() const;

			/// \brief Create a new swapchain
			/// \param desc The description of the swapchain.
			void CreateSwapChain(DXGI_SWAP_CHAIN_DESC desc);

			VideoMode video_mode_;

			AntialiasingMode antialiasing_mode_;

			bool fullscreen_;

			bool vsync_;

			// Listeners

			ListenerKey on_window_resized_listener_;

			// DirectX stuffs

			Window & window_;

			ID3D11Device & device_;

			IDXGIFactory & factory_;

			IDXGISwapChain * swap_chain_;

			/*
			ID3D11DeviceContext * immediate_context_;

			ID3D11RenderTargetView * backbuffer_view_;
			*/

		};

	}

}

#else

static_assert(false, "DirectX is not supported");

#endif