/// \file gilib.h
/// \brief Base classes and methods
///
/// \author Raffaele D. Facendola

#pragma once

namespace gi_lib{

	/// \brief Don't care.
	/// Use this structure when you don't need a parameter in a lamba expression
	struct _{

		template <typename... TArguments>
		_(TArguments&&...){}

	};
	
}