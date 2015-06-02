#pragma once

#include <memory>

#include "core.h"
#include "graphics.h"
#include "scene.h"

using namespace gi_lib;
using namespace std;

namespace gi_lib{
	
	class TiledDeferredRenderer;

}

namespace gi{

	///Application's logic
	class GILogic : public IWindowLogic{

	public:

		GILogic();

		virtual ~GILogic();

	protected:

		virtual void Initialize(Window& window) override;

		virtual void Update(const Time & time) override;
		
	private:

		Graphics & graphics_;

		unique_ptr<IOutput> output_;

		unique_ptr<TiledDeferredRenderer> deferred_renderer_;

		Scene scene_;

	};

}

