/// \file wavefront_obj.h
/// \brief Classes and methods to import and convert Wavefront OBJ files.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>

using ::std::wstring;

namespace gi_lib {

	class Resources;
	class TransformComponent;

	namespace wavefront {

		class IMtlMaterial {

		public:

		};

		/// \brief Class used to import a .obj scene.
		/// \author Raffaele D. Facendola
		class ObjImporter {

		public:

			ObjImporter(Resources& resources);

			/// \brief Import an OBJ scene.

			/// The scene will load various scene nodes and the appropriate components.
			/// All the nodes will keep their structure but will be attached to the provided root.
			/// \param file_name Name of the OBJ file to import.
			/// \param root The node where all the imported nodes will be attached hierarchically.
			/// \param resources Used to load resources during the import process.
			bool ImportScene(const wstring& file_name, TransformComponent& root) const;

		private:

			/// \brief No assignment operator.
			ObjImporter& operator=(const ObjImporter&) = delete;

			Resources& resources_;						///< \brief Used to load the resources.

		};

	}

}