/// \file exceptions.h
/// \brief Defines classes and macros used for exception handling.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>

#include "macros.h"

using ::std::wstring;

/// \brief Debug boilerplate used to localize exceptions.
/// The format is "<File>:<Line> (<Function>)"
#define EXCEPTION_LOCATION __FILE__ ":" TO_STRING(__LINE__) " (" __FUNCTION__ ")"

/// \brief Debug boilerplate used to localize exceptions.
/// The format is "<File>:<Line> (<Function>)"
#define EXCEPTION_LOCATION_W CONCATENATE(L, __FILE__) L":" TO_WSTRING(__LINE__) L" (" CONCATENATE(L, __FUNCTION__) L")"

/// \brief Throws an exception.
#define THROW(message) throw Exception(message, EXCEPTION_LOCATION_W)

namespace gi_lib{
	
	/// \brief Runtime exception
	/// \author Raffaele D. Facendola
	class Exception{

	public:

		/// \brief Create a new exception.
		/// \param error Error message associated to the exception.
		/// \param location Location of the exception.
		Exception(const wstring& error, const wstring& location);
		
		/// \brief Copy constructor.
		/// \param other Instance to copy from.
		Exception(const Exception& other);

		/// \brief Move constructor.
		/// \param other Instance to move.
		Exception(Exception&& other);

		/// \brief Unified assignment operator.
		Exception& operator=(Exception other);

		/// \brief Get the error message associated with the exception.
		const wstring& GetError() const;

		/// \brief Get the location of the exception.
		const wstring& GetLocation() const;

		/// \brief Get the full stack trace.
		const wstring& GetStackTrace() const;
				
	private:
		
		/// \brief Swap this exception with another one.
		void Swap(Exception& other);

		wstring error_;				///< \brief Message associated with the exception.

		wstring location_;			///< \brief Where the exception occured.
		
		wstring stack_trace_;		///< \brief Full stack trace.

	};

}