/// \file macros.h
/// \brief Bad and ugly macros are declared here :D
///
/// \author Raffaele D. Facendola

#pragma once 

#if _MSC_VER >= 1500

/// \brief A "while(0)" statement who doesn't throw warning #4127 (Requires VS 2008 or later).
#define WHILE0 \
__pragma(warning(push)) \
__pragma(warning(disable:4127)) \
	while(0) \
__pragma(warning(pop))

#else

/// \brief A "while(0)" statement. Some compilers will throw warning #4127 because the condition is constant.
#define WHILE0 \
while(0)

#endif

#if defined(_MSC_VER) && defined(_DEBUG)

#define DEBUG_ONLY(x) x

#else

#define DEBUG_ONLY(x)

#endif

/// \brief Expand the given token.
#define EXPAND(x) x

/// \brief Select the 3rd token.
#define SELECT_3RD(_0, _1, x, ...) x

/// \brief Select the 4th token.
#define SELECT_4TH(_0, _1, _2, x, ...) x

/// \brief Select the 5th token.
#define SELECT_5TH(_0, _1, _2, _3, x, ...) x

/// \brief Token concatenation.
#define CONCATENATE_DETAIL(x, y) \
x##y

/// \brief Token concatenation with argument expansion.
#define CONCATENATE(x, y) \
CONCATENATE_DETAIL(x, y)

/// \brief Token stringification.
#define TO_STRING_DETAIL(x) \
#x

/// \brief Token stringification with argument expansion.
#define TO_STRING(x) \
TO_STRING_DETAIL(x)

/// \brief Token stringification with argument expansion.
#define TO_WSTRING(x) \
CONCATENATE(L, TO_STRING(x))

#ifdef _MSC_VER

/// \brief Anonymous name generation (unique).
#define ANONYMOUS \
CONCATENATE(anon_, __COUNTER__)

#else

// \brief Anonymous name generation (unique).
// This macro should be used once per line at most.
#define ANONYMOUS \
CONCATENATE(anon_, __LINE__)

#endif

/// \brief SFINAE trick for template type constraints. Use during definion.
#define DERIVES_FROM_DEF(derived, base) \
typename std::enable_if<std::is_base_of<base, derived>::value>::type*

/// \brief SFINAE trick for template type constraints. use during declaration.
#define DERIVES_FROM(derived, base) \
DERIVES_FROM_DEF(derived, base) = nullptr

/// \brief Replaced by "t" if "derived" derives from "base". Use during declaration.
#define DERIVES_FROM_T(derived, base, t) \
std::enable_if_t<std::is_base_of<base, derived>::value, t>

#define CONVERTIBLE_TO(from, to) \
typename std::enable_if<std::is_convertible<from, to>::value>::type* = nullptr