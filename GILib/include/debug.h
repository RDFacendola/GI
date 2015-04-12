/// \file debug.h
/// \brief Debug definitions
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef SUPPRESS_TODOS

/// \brief TODO. The application should work, functionalities are limited though.
#define TODO(x)

#else

/// \brief TODO. You'd be better to implement that functionality...
#define TODO(x) static_assert(false, x)

#endif