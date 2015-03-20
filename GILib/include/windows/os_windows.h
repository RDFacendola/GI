/// \file os_windows.h
/// \brief Windows-specific interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <memory>
#include <string>

#include "..\macros.h"

using std::unique_ptr;

/// \brief If the provided expression fails the caller returns the expression value, otherwise nothing happens.
/// The expression fails if FAILED(.) is true.
#define RETURN_ON_FAIL_1(expr) \
do{ \
	HRESULT __hr = expr; \
	if (FAILED(__hr)) return __hr; \
}WHILE0

/// \brief If the provided expression fails the caller returns the expression value, otherwise nothing happens.
/// The expression fails if FAILED(.) is true.
#define RETURN_ON_FAIL_2(expr, retrn) \
do{ \
	HRESULT __hr = expr; \
	if (FAILED(__hr)) return retrn; \
}WHILE0

/// \brief Macro selector. If the provided expression fails the caller returns either the failure value or a specified value.
#define RETURN_ON_FAIL(...) EXPAND( SELECT_3RD(__VA_ARGS__ , RETURN_ON_FAIL_2, RETURN_ON_FAIL_1)(__VA_ARGS__ ) )

/// \brief If the provided expression fails the caller throws an exception with the error code, otherwise nothing happens.
/// The expression fails if FAILED(.) is true.
#define THROW_ON_FAIL_1(expr) \
do{ \
	HRESULT __hr = expr; \
	if(FAILED(__hr)) THROW(std::to_wstring(__hr)); \
}WHILE0

/// \brief If the provided expression fails the caller throws an exception with the error code followed by the specified error message, otherwise nothing happens.
/// The expression fails if FAILED(.) is true.
#define THROW_ON_FAIL_2(expr, thrw_mssg) \
do{ \
	HRESULT __hr = expr; \
	if(FAILED(__hr)) THROW(std::to_wstring(__hr) + L": " + thrw_mssg); \
}WHILE0

/// \brief Macro selector. If the provided expression fails the caller returns either the failure value or a specified value.
#define THROW_ON_FAIL(...) \
EXPAND( SELECT_3RD(__VA_ARGS__ , THROW_ON_FAIL_2, THROW_ON_FAIL_1)(__VA_ARGS__ ) )

/// \brief Defines a raii guard for COM interfaces.
#define COM_GUARD(com) \
unique_ptr<IUnknown, COMDeleter> ANONYMOUS(com, COMDeleter{})

namespace gi_lib{

	namespace windows{

		/// \brief Functor used to delete COM interfaces.
		struct COMDeleter{

			/// \brief Release the given COM interface.
			/// \param ptr Pointer to the COM resource to delete.
			void operator()(IUnknown * com);
			
		};

		template <typename TCOM>
		inline unique_ptr<TCOM, COMDeleter> unique_com(TCOM* com_object){

			return unique_ptr<TCOM, COMDeleter>(com_object, COMDeleter{});

		}

		inline void release_com(IUnknown * com){

			if (com){

				com->Release();

			}

		}

		inline void release_com(std::initializer_list<IUnknown*> com_list){

			for (auto com : com_list){

				release_com(com);

			}

		}

		// COMDeleter

		inline void COMDeleter::operator()(IUnknown * com){

			release_com(com);
			
		}

	}
	
}

#endif