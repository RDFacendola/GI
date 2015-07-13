/// \file debug.h
/// \brief Debug macros and methods.
///
/// \author Raffaele D. Facendola

#pragma once 

#include <assert.h>

namespace gi_lib{

	/// \brief Performs a checked cast of a pointer to another type.
	/// If the conversion is invalid, this method will throw.
	/// The check is performed only at debug time.
	template <typename TOutput, typename TInput>
	inline TOutput* checked_cast(TInput* ptr){

		assert(ptr == nullptr ||
			   dynamic_cast<TOutput*>(ptr) != nullptr);

		return static_cast<TOutput*>(ptr);

	}

}
