/// \file unique.h
/// \brief Classes and functions used to manage unique objects.
///
/// \author Raffaele D. Facendola

#pragma once

namespace gi_lib{

	/// \brief Class of unique instances.

	/// This class generates instances which are guaranteed to be unique. Those instances can be tested for equality via equality operators.
	/// \tparam TTag Tag associated to this class. Used to specialize unique instances.
	/// \author Raffaele D. Facendola
	template <typename TTag = void>
	class Unique{

	public:

		/// \brief Create a new unique instance.
		Unique(){

			key_ = ++counter_;

		}

		/// \brief Tests for equality.
		/// \param other The other instance to test against.
		/// \return Returns true if this object and "other" are the same, returns false otherwise.
		bool operator==(const Unique & other) const{

			return key_ == other.key_;

		}

		/// \brief Tests for inequality.
		/// \param other The other instance to test against.
		/// \return Returns true if this object and "other" are NOT the same, returns false otherwise.
		bool operator!=(const Unique & other) const{

			return key_ != other.key_;

		}

	private:

		// Counter used to generate unique instances.
		static unsigned int counter_;

		// Key of this instance.
		unsigned int key_;

	};

	template <typename TTag>
	unsigned int Unique<TTag>::counter_ = 0;

}