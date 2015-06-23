/// \file eigen.h
/// \brief Wrapper around Eigen headers.
///
/// \author Raffaele D. Facendola

#pragma once

#if _MSC_VER >= 1500

#pragma warning(push)
#pragma warning(disable:4127)

#include <Eigen\Geometry>

#pragma warning(pop)

#else

#include <Eigen\Geometry>

#endif

namespace gi_lib{

	using ::Eigen::Vector2f;
	using ::Eigen::Vector3f;
	using ::Eigen::Vector4f;
	using ::Eigen::Vector2i;
	using ::Eigen::Vector3i;
	using ::Eigen::Vector4i;
	using ::Eigen::Affine3f;
	using ::Eigen::Matrix4f;
	using ::Eigen::Matrix3f;

	using ::Eigen::AngleAxisf;
	using ::Eigen::Translation3f;
	using ::Eigen::AlignedScaling3f;
	using ::Eigen::Quaternionf;

}
