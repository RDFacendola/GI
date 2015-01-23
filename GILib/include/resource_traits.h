/// \file resource_traits.h
/// \brief Template header for graphical resource's traits
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

	/// \brief Resources' load setting's template.
	template <typename TResource, typename TResource::LoadMode kLoadMode> struct LoadSettings;

	/// \brief Settings used to load a Texture2D from a DDS file.

	/// The load setting allows caching.
	template <> struct LoadSettings < Texture2D, Texture2D::LoadMode::kFromDDS> {

		/// \brief Name of the file to load relative to the resource folder.
		wstring file_name;	

		/// \brief Get the cache key associated to the load settings.
		size_t GetCacheKey() const;

	};
	
	/// \brief Build settings for normal textured meshes.
	template <> struct LoadSettings<Mesh, Mesh::LoadMode::kNormalTextured>{

		/// \brief Indices' data.
		vector<unsigned int> indices;

		/// \brief Vertices' data.
		vector<VertexFormatNormalTextured> vertices;

	};

	/// \brief Settings used to load a Material from a shader file.
	template <> struct LoadSettings < Material, Material::LoadMode::kFromShader> {

		/// \brief Name of the file to load relative to the resource folder.
		wstring file_name;

		/// \brief Get the cache key associated to the load settings.
		size_t GetCacheKey() const;

	};




}