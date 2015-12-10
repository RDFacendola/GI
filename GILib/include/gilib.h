/// \file gilib.h
/// \brief Base classes and methods
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <locale>
#include <vector>
#include <codecvt>

#include "macros.h"
#include "exceptions.h"

namespace gi_lib{

	/// \brief Don't care.
	/// Use this structure when you don't need a parameter in a lambda expression
	struct _{

		template <typename... TArguments>
		_(TArguments&&...){}

	};
	
	/// \brief Converts a string to a wstring.
	inline std::wstring to_wstring(const std::string& string){

		return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(string);

	}

	/// \brief Converts a wstring to a string.
	inline std::string to_string(const std::wstring& wstring){

		return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(wstring);

	}

	/// \brief Split a string using the specified delimiter character.
	/// \param source String to split.
	/// \param delimiter Character delimiting each token.
	/// \return Returns a vector containing the tokens extracted by the source string.
	std::vector<std::string> Split(const std::string& source, char delimiter = ' ');

}