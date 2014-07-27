#pragma once

#include <d3d11.h>

#include "igraphics.h"

///DirectX 11 utility class
class DX11Utils{

	DX11Utils() = delete;

public:

	/// Convert a multisample structure to an antialiasing mode
	static ANTIALIASING_MODE SampleToAntialiasing(const DXGI_SAMPLE_DESC & sample);

	/// Convert an antialiasing mode to a multisample structure
	static DXGI_SAMPLE_DESC AntialiasingToSample(const ANTIALIASING_MODE & antialiasing);

	/// Convert a video mode to a dxgi mode
	static DXGI_MODE_DESC VideoModeToDXGIMode(const VIDEO_MODE & video);

	/// Convert a dxgi mode to a video mode
	static VIDEO_MODE DXGIModeToVideoMode(const DXGI_MODE_DESC & dxgi);

};

///DirectX 11 Factory
class DX11Factory : public IFactory{

public:

	///Create a new factory
	DX11Factory();

	///Destroy the factory
	virtual ~DX11Factory();

	///Get the capabilities for this API
	virtual ADAPTER_PROFILE GetProfile() const;

	///Create the graphic object
	virtual BaseGraphics * Create(Window & window);
	
	///Get the current device
	inline ID3D11Device & GetDevice() const{

		return *device_;

	}

	///Get the current dxgi factory
	inline IDXGIFactory & GetFactory() const{

		return *factory_;

	}

	///Get the default dxgi adapter
	inline IDXGIAdapter & GetAdapter() const{

		return *adapter_;

	}
	
private:

	/// Enumerate the supported antialiasing modes
	vector<const ANTIALIASING_MODE> EnumerateAntialiasingModes() const;

	/// Enumerate the supported video modes. Filters by resolution and refresh rate
	vector<const VIDEO_MODE> EnumerateVideoModes() const;

	/// Enumerate the supported DXGI video modes
	vector<const DXGI_MODE_DESC> EnumerateDXGIModes() const;
	
	ID3D11Device * device_;
	
	IDXGIFactory * factory_;

	IDXGIAdapter * adapter_;

};

///DirectX 11 Graphics
class DX11Graphics : public BaseGraphics{

public:

	///Create a new graphics object
	DX11Graphics(Window & window_handle, DX11Factory & factory);

	///Destroy the graphic object
	virtual ~DX11Graphics();

	///Get the resources' loader
	virtual IResources & GetResources();

	///Get the renderer
	virtual IRenderer & GetRenderer();

	///Show the current frame and prepare the next one
	virtual void NextFrame();

	///Set the video mode
	virtual void SetVideo(const VIDEO_MODE & video);

	///Set the antialising mode
	virtual void SetAntialising(const ANTIALIASING_MODE & antialiasing);

	///Enable or disable the fullscreen state
	virtual void EnableFullscreen(bool enable);
	
private:

	///Return the default DXGI mode for the swap chain
	DXGI_SWAP_CHAIN_DESC GetDefaultSwapchainMode() const;

	///Create a new swapchain given its description
	void CreateSwapChain(DXGI_SWAP_CHAIN_DESC & desc);

	Window & window_;

	ID3D11Device & device_;

	DX11Factory & factory_;

	IResources * resources_;

	IRenderer * renderer_;

	IDXGISwapChain * swap_chain_;

	Window::TMessageEvent::TListener * on_window_message_listener_;

	/*
	ID3D11DeviceContext * immediate_context_;

	ID3D11RenderTargetView * backbuffer_view_;
	*/

};