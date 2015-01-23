/// \file bundles.h
/// \brief Collection of bundles used to load the various resources.
///
/// \author Raffaele D. Facendola

#pragma once

#include <Eigen/Core>
#include <memory>
#include <string>

#include "resources.h"

using ::Eigen::Vector3f;
using ::Eigen::Vector2f;
using ::std::shared_ptr;
using ::std::wstring;

namespace gi_lib{

	/// \brief Bundle used to load a resource from a file.

	/// Almost every resource can be loaded straight from a file.
	struct LoadFromFile{

		/// \brief Name of the file to load relative to the resource folder.
		wstring file_name;

		/// \brief Get the cache key associated to the load settings.
		size_t GetCacheKey() const;

	};

	/// \brief Bundle used to load a resource from an indexed, normal-textured set of vertices.

	/// Used to build a mesh.
	struct BuildIndexedNormalTextured{

		/// \brief Indices' data.
		vector<unsigned int> indices;

		/// \brief Vertices' data.
		vector<VertexFormatNormalTextured> vertices;

	};

}