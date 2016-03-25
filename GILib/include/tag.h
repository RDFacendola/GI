/// \file tag.h
/// \brief This file contains the classes used to handle hashed strings.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include "macros.h"

namespace gi_lib{

	/// \brief Represents an hashed string.
	/// \author Raffaele D. Facendola.
	class Tag{

	public:

		/// \brief Create an empty tag.
		Tag();

		/// \brief Create a new tag from a string.
		/// \param string The string used to create the tag.
		Tag(const std::string& string);

		/// \brief Create a new tag from a wide string.
		/// \param string The string used to create the tag.
		Tag(const std::wstring& string);

		/// \brief Create a new tag from a null-terminated string.
		/// \param string Pointer to a null-terminated string used to create the tag.
		Tag(const char* string);

		/// \brief Create a new tag from a null-terminated wide string.
		/// \param string Pointer to a null-terminated string used to create the tag.
		Tag(const wchar_t* string);

		/// \brief Implicit cast to size_t type.
		operator size_t() const;

		/// \brief Equality operator.
		/// \return Returns true if the two tags are identical, returns false otherwise.
		bool operator==(const Tag& other) const;

		/// \brief Inequality operator.
		/// \return Returns true if the two tags are different, returns false otherwise.
		bool operator!=(const Tag& other) const;

		/// \brief Less-equal operator.
		/// \return Returns true if the current instance is less than or equal to the provided tag, returns false otherwise.
		bool operator<=(const Tag& other) const;

		/// \brief Greater-equal operator.
		/// \return Returns true if the current instance is greater than or equal to the provided tag, returns false otherwise.
		bool operator>=(const Tag& other) const;

		/// \brief Less-than operator.
		/// \return Returns true if the current instance is less than the provided tag, returns false otherwise.
		bool operator<(const Tag& other) const;

		/// \brief Greater-than operator.
		/// \return Returns true if the current instance is greater than the provided tag, returns false otherwise.
		bool operator>(const Tag& other) const;

	private:

		size_t tag_;	///< \brief The actual tag.

		DEBUG_ONLY(std::string name_;)		///< \brief Name which generated the given tag.
		
	};

	/////////////////////////////////// TAG ///////////////////////////////////////

	inline Tag::Tag() : tag_(0){}

	inline Tag::operator size_t() const{

		return tag_;

	}

	inline bool Tag::operator==(const Tag& other) const{

		return tag_ == other.tag_;

	}

	inline bool Tag::operator!=(const Tag& other) const {

		return tag_ != other.tag_;

	}

	inline bool Tag::operator<=(const Tag& other) const{

		return tag_ <= other.tag_;

	}

	inline bool Tag::operator>=(const Tag& other) const{

		return tag_ >= other.tag_;

	}

	inline bool Tag::operator<(const Tag& other) const{

		return tag_ < other.tag_;

	}

	inline bool Tag::operator>(const Tag& other) const{

		return tag_ > other.tag_;

	}

}

