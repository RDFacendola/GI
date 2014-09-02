/// \file resource_traits.h
/// \brief Template header for graphical resource's traits
///
/// \author Raffaele D. Facendola

#pragma once

#include "resources.h"

namespace gi_lib{

	/// \brief Resources' load setting's template.
	template <typename TResource, typename TResource::LoadMode kLoadMode> struct LoadSettings;

	/// \brief Resources' build setting's template.
	template <typename TResource, typename TResource::BuildMode kBuildMode> struct BuildSettings;

	/// \brief Settings used to load a Texture2D from a DDS file.
	template <> struct LoadSettings < Texture2D, Texture2D::LoadMode::kFromDDS > {

		wchar_t * file_name;		///< \brief Name of the file to load relative to the resource folder.

	};

	/// \brief Settings used to load a Mesh from a FBX file.
	template <> struct LoadSettings < Mesh, Mesh::LoadMode::kFromFBX > {

		wchar_t * file_name;		///< \brief Name of the scene file relative to the resource folder.
		wchar_t * mesh_name;		///< \brief Mesh node name.

	};

	/// \brief Settings used to build a mesh from its attributes' description.
	template <> struct BuildSettings<Mesh, Mesh::BuildMode::kFromAttributes>{

		/// \brief Position of each vertex.
		vector<Vector3f> positions;

		/// \brief Indices of the mesh.
		vector<unsigned int> indices;

		/// \brief Normal of each vertex. Optional.
		vector<Vector3f> normals;

		/// \brief Binormals of each vertex. Optional.
		vector<Vector3f> binormals;

		/// \brief Tangents of each vertex. Optional.
		vector<Vector3f> tangents;

		/// \brief UV coordinates of each vertex. Optional.
		vector<Vector2f> UVs;

		/// \brief Specified how the normals are mapped against the mesh.
		AttributeMappingMode normal_mapping;

		/// \brief Specified how the binormals are mapped against the mesh.
		AttributeMappingMode binormal_mapping;

		/// \brief Specified how the tangents are mapped against the mesh.
		AttributeMappingMode tangent_mapping;

		/// \brief Specified how the UV coordinates are mapped against the mesh.
		AttributeMappingMode UV_mapping;

	};

}