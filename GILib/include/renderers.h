/// \file renderers.h
/// \brief Defines renderer classes.
///
/// \author Raffaele D. Facendola

namespace gi_lib{

	class IRenderer;
	
	/// \brief Base interface for renderers.
	/// \author Raffaele D. Facendola
	class IRenderer{

	public:

		/// \brief Virtual destructor.
		virtual ~IRenderer(){}

	protected:

		/// \brief Protected constructor. Prevent instantiation.
		IRenderer(){}

	};

	/// \brief Deferrend renderer with tiled lighting computation.
	/// \author Raffaele D. Facendola
	class TiledDeferredRenderer : public IRenderer{

	public:

		/// \brief Virtual destructor.
		virtual ~TiledDeferredRenderer(){}

	};

}
