/// \file dx11graphics.h
/// \brief Declare classes and interfaces used to manage the core of DirectX 11 API.
///
/// \author Raffaele D. Facendola

#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <memory>

#include "dx11render_target.h"

#include "observable.h"
#include "graphics.h"

#include "windows/win_core.h"
#include "windows/win_os.h"

#include "fx/dx11fx_transform.h"

struct ID3D11Device;
struct IDXGIFactory;
struct DXGI_SWAP_CHAIN_DESC;
struct IDXGISwapChain;

namespace gi_lib{

	class Window;
	class Camera;
	class SceneNode;

	namespace dx11{

		using windows::COMPtr;

		/// \brief DirectX11 object used to display an image to an output.
		/// \author Raffaele D. Facendola
		class DX11Output : public IOutput{

		public:

			/// \brief No copy-constructor.
			DX11Output(const DX11Output&) = delete;

			/// \brief No assignment operator.
			DX11Output & operator=(const DX11Output&) = delete;

			/// \brief Create a new DirectX11 output window.
			/// \param window The window where the final image will be displayed.
			/// \param video_mode Video mode used to initialize the output.
			DX11Output(windows::Window& window, const VideoMode& video_mode);

			/// \brief Default destructor.
			~DX11Output();

			virtual void SetVideoMode(const VideoMode& video_mode) override;

			virtual const VideoMode& GetVideoMode() const override;

			virtual void SetFullscreen(bool fullscreen) override;

			virtual bool IsFullscreen() const override;

			virtual void SetVSync(bool vsync) override;

			virtual bool IsVSync() const override;

			virtual void SetAntialiasing(AntialiasingMode antialiasing) override;

			virtual AntialiasingMode GetAntialiasing() const override;
	
			virtual void Display(const ObjectPtr<ITexture2D>& image) override;
			
		private:

			void CreateSwapChain();

			void UpdateBackbuffer();

			VideoMode video_mode_;
			
			bool fullscreen_;

			bool vsync_;

			AntialiasingMode antialiasing_;

			// Listeners

			unique_ptr<Listener> on_window_resized_listener_;
			
			// DirectX stuffs

			windows::Window& window_;

			COMPtr<IDXGISwapChain> swap_chain_;

			COMPtr<ID3D11Texture2D> back_buffer_;					///< \brief Reference to the actual backbuffer. It is never referenced outside this class otherwise the resize wouldn't work.

			ObjectPtr<IRenderTarget> render_target_;				///< \brief Render target wrapped around the backbuffer. Only one render target is defined.

			unique_ptr<DX11FxScale> scaler_;						///< \brief Used to scale the image onto the backbuffer.

		};

		/// \brief Resource manager interface for DirectX11.
		/// \author Raffaele D. Facendola.
		class DX11Resources : public Resources{

		public:

			using Resources::Load;

			/// \brief Get the DX11 resources manager singleton.
			/// \return Returns a reference to the DX11 resources manager singleton.
			static DX11Resources& GetInstance();

			/// \brief No copy constructor.
			DX11Resources(const DX11Resources&) = delete;

			/// \brief No assignment operator.
			DX11Resources & operator=(const DX11Resources&) = delete;

		protected:

			virtual ObjectPtr<IResource> Load(const type_index& resource_type, const type_index& args_type, const void* args) const override;

		private:

			/// \brief Create a new instance of DirectX11 resource manager.
			DX11Resources();

		};

		/// \brief DirectX11 graphics class.
		/// \author Raffaele D. Facendola
		class DX11Graphics : public Graphics{

		public:

			/// \brief Get the DirectX11 graphics singleton.
			/// \return Returns a reference to the DirectX11 factory singleton.
			static DX11Graphics& GetInstance();

			virtual AdapterProfile GetAdapterProfile() const override ;

			virtual unique_ptr<IOutput> CreateOutput(Window& window, const VideoMode& video_mode) override;

			virtual DX11Resources& GetResources() override;

			COMPtr<ID3D11Device> GetDevice();

			COMPtr<IDXGIFactory> GetFactory();

			COMPtr<IDXGIAdapter> GetAdapter();

			COMPtr<ID3D11DeviceContext> GetImmediateContext();

		protected:

			virtual IRenderer* CreateRenderer(const type_index& renderer_type, Scene& scene) const override;

		private:

			DX11Graphics();

			~DX11Graphics();

			void FlushAll();

			void DebugReport();

			COMPtr<ID3D11Device> device_;

			COMPtr<IDXGIFactory> factory_;

			COMPtr<IDXGIAdapter> adapter_;

			COMPtr<ID3D11DeviceContext> immediate_context_;

		};

		/////////////////////// DX11OUTPUT /////////////////////

		inline const VideoMode & DX11Output::GetVideoMode() const{

			return video_mode_;

		}

		inline bool DX11Output::IsFullscreen() const{

			return fullscreen_;

		}

		inline void DX11Output::SetVSync(bool vsync){

			vsync_ = vsync;

		}

		inline bool DX11Output::IsVSync() const{

			return vsync_;

		}

		inline AntialiasingMode DX11Output::GetAntialiasing() const{

			return antialiasing_;

		}

		/////////////////////// DX11GRAPHICS ///////////////////

		inline COMPtr<ID3D11Device> DX11Graphics::GetDevice(){

			return device_;

		}
		
		inline COMPtr<IDXGIFactory> DX11Graphics::GetFactory(){

			return factory_;

		}

		inline COMPtr<IDXGIAdapter> DX11Graphics::GetAdapter(){

			return adapter_;

		}

		inline COMPtr<ID3D11DeviceContext> DX11Graphics::GetImmediateContext(){

			return immediate_context_;

		}

	}

}