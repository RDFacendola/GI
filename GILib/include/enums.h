/// \file enums.h
/// \brief Interfaces related to enumerables.
///
/// \author Raffaele D. Facendola

#pragma once

/// \brief Macro to declare an enum class with flags operations.

/// Usage:
/// ENUM_FLAGS(Foo, int){ a, b, ... }	 // Declaration
/// auto k = a | b;						 // Operator|, used to compose many flags.
/// auto k = a & b;						 // Operator&, used to intersect many flags at once.
/// auto k = a; k |= b;					 // Operator|=, same as above.
/// auto k = a; k &= b;					 // Operator&, same as above.
/// if(k && a) { ... }					 // Operator&&, used to check whether a flag is set.
#define ENUM_FLAGS(__name, __type) \
enum class __name : __type;	\
inline __name operator|(__name a, __name b){ return static_cast<__name>	(static_cast<__type>(a) | static_cast<__type>(b)); }; \
inline __name operator&(__name a, __name b){ return static_cast<__name> (static_cast<__type>(a) & static_cast<__type>(b)); }; \
inline __name&	operator|=(__name& a, __name b){ a = a | b;	return a; }; \
inline __name&	operator&=(__name& a, __name b){ a = a & b;	return a; }; \
inline bool operator&&(const __name& e, const __name& flag){ return (static_cast<__type>(e) & static_cast<__type>(flag)) != 0; } \
enum class __name : __type
