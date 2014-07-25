#pragma once

#include <d3d11.h>

#include "igraphics.h"

class DX11Graphics : public IGraphics{

public:

	DX11Graphics();

	///Create the graphic interface. Throws on failure
	virtual void CreateOrDie(const HWND & window_handle, const GRAPHIC_MODE & graphic_mode);
	
	///Finalize the current frame and present it to the screen
	virtual void Present();

	///Return the video mode who matches with the graphic mode specified
	static VIDEO_MODE GetVideoModeOrDie(const GRAPHIC_MODE & graphic_mode);

	///Get the multisample structure associated to a specific antialiasing mode
	static MULTISAMPLE GetMultisampleOrDie(const ANTIALIASING_MODE & antialiasing_mode);

private:

	GRAPHIC_MODE graphic_mode_;

	ID3D11Device * device_;

	IDXGISwapChain * swap_chain_;

	ID3D11DeviceContext * immediate_context_;

	ID3D11RenderTargetView * backbuffer_view_;

};