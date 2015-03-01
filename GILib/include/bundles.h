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

	/// \brief Macro used to declare that the bundle will use the caching mechanism.
	#define USE_CACHE using use_cache = void;

	/// \brief Macro used to declare that the bundle won't use the caching mechanism.
	#define NO_CACHE using no_cache = void;

	/// \brief If TBundle declares a type "use_cache", use_cache has a public member "type", otherwise there's no member.
	template <typename TBundle, typename TBundle::use_cache* = nullptr>
	struct use_cache{

		using type = void;

	};

	/// \brief If TBundle declares a type "no_cache", no_cache has a public member "type", otherwise there's no member.
	template <typename TBundle, typename TBundle::no_cache* = nullptr>
	struct no_cache{

		using type = void;

	};

	/// \brief Bundle used to load a resource from a file.

	/// Almost every resource can be loaded straight from a file.
	struct LoadFromFile{

		USE_CACHE

		/// \brief Name of the file to load relative to the resource folder.
		wstring file_name;

		/// \brief Get the cache key associated to the load settings.
		size_t GetCacheKey() const;

	};

	/// \brief Bundle used to load a resource from an indexed, normal-textured set of vertices.

	/// Used to build a mesh.
	struct BuildIndexedNormalTextured{

		NO_CACHE

		/// \brief Indices' data.
		vector<unsigned int> indices;

		/// \brief Vertices' data.
		vector<VertexFormatNormalTextured> vertices;

	};

	/// \brief Bundle used to instantiate an material from another one.
	struct InstantiateFromMaterial{

		NO_CACHE

		shared_ptr<Material> base;

	};

}