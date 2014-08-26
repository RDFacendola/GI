/// \file macros.h
/// \brief Bad and ugly macros are declared here :D
///
/// \author Raffaele D. Facendola

#ifdef _WIN32

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

#endif