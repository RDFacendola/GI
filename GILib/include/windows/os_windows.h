/// \file os_windows.h
/// \brief Windows-specific interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <memory>

#include "..\macros.h"

using std::unique_ptr;

/// If expr fails, throws a runtime exception with detailed informations. "expr" must be of type "HRESULT"; the error code is defined by the value of the expression itself.
#define THROW_ON_FAIL(expr) do{ \
								HRESULT hr = expr; \
								if(FAILED(hr)) { \
									std::wstringstream stream; \
									stream << L"\"" << #expr << "\" failed with 0x" << std::hex << hr << std::dec << std::endl \
										   << __FILE__ << std::endl \
										   << __FUNCTION__ << L" @ " << __LINE__ << std::endl; \
									throw RuntimeException(stream.str(), {{L"error_code", std::to_wstring(hr)}}); \
																} \
							}WHILE0

/// If expr is 0, throws a runtime exception. "expr" must be of type "bool".
#define THROW_ON_ZERO(expr) do{ \
								auto b = expr; \
								if( b == 0) { \
									std::wstringstream stream; \
									stream << L"\"" << #expr << "\" failed" << std::endl \
										   << __FILE__ << std::endl \
										   << __FUNCTION__ << L" @ " << __LINE__ << std::endl; \
									throw RuntimeException(stream.str()); \
																								} \
						   }WHILE0

/// If "expr" raises an error, throws a runtime exception with detailed informations. "expr" must support the unary operator "!"; the error code must be returned by GetLastError().
#define THROW_ON_ERROR(expr) do { \
								auto hr = expr; \
								if(!hr){ \
									std::wstringstream stream; \
									stream << L"\"" << #expr << "\" failed with 0x" << std::hex << GetLastError() << std::dec << std::endl \
										   << __FILE__ << std::endl \
										   << __FUNCTION__ << L" @ " << __LINE__ << std::endl; \
									throw RuntimeException(stream.str(), {{L"error_code", std::to_wstring(GetLastError())}}); } \
							 }WHILE0

/// If expr fails returns from the routine with the fail code
#define RETURN_ON_FAIL(expr) do{ \
								HRESULT hr = expr; \
								if (FAILED(hr)){ \
									return hr; \
																} \
							 }WHILE0

/// Throw an error. Enhances the error message with the file, the function name and the line.
#define THROW(err_str) do{ \
					      std::wstringstream stream; \
				          stream << err_str << std::endl \
								 << __FILE__ << std::endl \
								 << __FUNCTION__ << L" @ " << __LINE__ << std::endl; \
						  throw RuntimeException(stream.str()); \
			           }WHILE0


// \brief Defines a raii guard for COM interfaces.
#define COM_GUARD(com) unique_ptr<IUnknown, COMDeleter> ANONYMOUS(com, COMDeleter{});

namespace gi_lib{

	namespace windows{

		/// \brief Functor used to delete COM interfaces.
		struct COMDeleter{

			/// \brief Release the given COM interface.
			/// \param ptr Pointer to the COM resource to delete.
			void operator()(IUnknown * com);

		};

		// COMDeleter

		inline void COMDeleter::operator()(IUnknown * com){

			com->Release();
			
		}

	}
	
}

#endif