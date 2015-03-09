/// \file macros.h
/// \brief Bad and ugly macros are declared here :D
///
/// \author Raffaele D. Facendola

#pragma once 

#if _MSC_VER >= 1500

/// \brief A "while(0)" statement who doesn't throw warning #4127 (Requires VS 2008 or later).
#define WHILE0 __pragma(warning(push)) \
			   __pragma(warning(disable:4127)) \
			   			   			   while(0) \
               __pragma(warning(pop))

#else

/// \brief A "while(0)" statement. Some compilers will throw warning #4127 because the condition is constant.
#define WHILE0 while(0)

#endif

// \brief Token concatenation.
#define CONCATENATE_DETAIL(x, y) x##y

// \brief Token concatenation with argument expansion.
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)

// \brief Token stringification.
#define TO_STRING_DETAIL(x) #x

// \brief Token stringification with argument expansion.
#define TO_STRING(x) TO_STRING_DETAIL(x)

// \brief Token stringification with argument expansion.
#define TO_WSTRING(x) CONCATENATE(L, TO_STRING(x))

// \brief Debug boilerplate used to localize exceptions.
// The format is "<File>:<Line> (<Function>)"
#define DEBUG_BOILERPLATE __FILE__ ":" TO_STRING(__LINE__) " (" __FUNCTION__ ")"

// \brief Debug boilerplate used to localize exceptions.
// The format is "<File>:<Line> (<Function>)"
#define DEBUG_BOILERPLATEW CONCATENATE(L, __FILE__) L":" TO_WSTRING(__LINE__) L" (" CONCATENATE(L, __FUNCTION__) L")"

#ifdef _MSC_VER

// \brief Anonymous name generation (unique).
#define ANONYMOUS CONCATENATE(anon_, __COUNTER__)

#else

// \brief Anonymous name generation (unique).
// This macro should be used at most once per line.
#define ANONYMOUS CONCATENATE(anon_, __LINE__)

#endif