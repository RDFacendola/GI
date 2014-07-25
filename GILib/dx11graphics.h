#pragma once

#include <d3d11.h>

#include "igraphics.h"

///DirectX 11.0
class DX11Graphics : public IGraphics{

public:

	///Format used for the screen
	static const DXGI_FORMAT kGraphicFormat;

	///Multisample structure used to describe the antialiasing mode
	struct MULTISAMPLE{

		unsigned int count;
		unsigned int quality;

	};

	///Get the default adapter's capabilities for this API
	virtual ADAPTER_PROFILE GetAdapterProfile() const;
	
	///Create the graphic interface. Throws on failure
	virtual void CreateOrDie(const HWND & window_handle, const GRAPHIC_MODE & graphic_mode);

	///Finalize the current frame and present it to the screen
	virtual void Present();

	/// Convert a multisample structure to an antialiasing mode
	virtual ANTIALIASING_MODE MultisampleToAntialiasing(const MULTISAMPLE & multisample) const;

	/// Convert an antialiasing mode to a multisample structure
	virtual MULTISAMPLE AntialiasingToMultisample(const ANTIALIASING_MODE & antialiasing) const;

	/// Convert a video mode to a dxgi mode
	virtual DXGI_MODE_DESC VideoModeToDXGIMode(const VIDEO_MODE & video) const;

	/// Convert a dxgi mode to a video mode
	virtual VIDEO_MODE DXGIModeToVideoMode(const DXGI_MODE_DESC & dxgi) const;

	/// Return the feature level associated to this API
	virtual D3D_FEATURE_LEVEL GetFeatureLevel() const;

private:

	/// Enumerate the supported antialiasing modes
	vector<const ANTIALIASING_MODE> EnumerateAntialiasingModes() const;

	/// Enumerate the supported video modes. Filters by resolution and refresh rate
	vector<const VIDEO_MODE> EnumerateVideoModes(IDXGIAdapter * adapter) const;

	/// Enumerate the supported DXGI video modes
	vector<const DXGI_MODE_DESC> EnumerateDXGIModes(IDXGIAdapter * adapter) const;
	
	GRAPHIC_MODE graphic_mode_;

	ID3D11Device * device_;

	IDXGISwapChain * swap_chain_;

	ID3D11DeviceContext * immediate_context_;

	ID3D11RenderTargetView * backbuffer_view_;

};