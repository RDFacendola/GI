/// \file tag.h
/// \brief This file contains the classes used to handle hashed strings.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>

namespace gi_lib{

	/// \brief Represents an hashed string.
	/// \author Raffaele D. Facendola.
	class Tag{

	public:

		/// \brief Create a new tag from a string.
		/// \param string The string used to create the tag.
		Tag(const std::string& string);

		/// \brief Create a new tag from a null-terminated string.
		/// \param string The string used to create the tag.
		Tag(const char* string, size_t size);

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
		
	};

}

/////////////////////////////////// TAG ///////////////////////////////////////

bool gi_lib::Tag::operator==(const Tag& other) const{

	return tag_ == other.tag_;

}

bool gi_lib::Tag::operator!=(const Tag& other) const {

	return tag_ != other.tag_;

}

bool gi_lib::Tag::operator<=(const Tag& other) const{

	return tag_ <= other.tag_;

}

bool gi_lib::Tag::operator>=(const Tag& other) const{

	return tag_ >= other.tag_;

}

bool gi_lib::Tag::operator<(const Tag& other) const{

	return tag_ < other.tag_;

}

bool gi_lib::Tag::operator>(const Tag& other) const{

	return tag_ > other.tag_;

}