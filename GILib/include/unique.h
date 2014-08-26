/// \file unique.h
/// \brief Classes and functions used to manage unique objects.
///
/// \author Raffaele D. Facendola

#pragma once

#include <atomic>

namespace gi_lib{

	/// \brief Class of unique instances.

	/// This class generates instances which are guaranteed to be unique. Those instances can be tested for equality via equality operators.
	/// \tparam TTag Tag associated to this class. Used to specialize unique instances.
	/// \author Raffaele D. Facendola
	template <typename TTag = void>
	class Unique{

	public:

		/// \brief Null object associated to the unique class.
		static const Unique kNull;

		Unique() :
			Unique(kNull){}

		/// \brief Create a new unique instance.
		static Unique MakeUnique(){

			static std::atomic_uint counter_{ 0 };

			return Unique(counter_.fetch_add(1, std::memory_order_relaxed));

		}
		
		/// \brief Tests for equality.
		/// \param other The other instance to test against.
		/// \return Returns true if this object and "other" are the same, returns false otherwise.
		inline bool operator==(const Unique & other) const{

			return key_ == other.key_;

		}

		/// \brief Tests for inequality.
		/// \param other The other instance to test against.
		/// \return Returns true if this object and "other" are NOT the same, returns false otherwise.
		inline bool operator!=(const Unique & other) const{

			return key_ != other.key_;

		}

		/// \brief Less-than operator.
		/// \param other The other instance to test against.
		/// \return Returns true if this object's key is lesser than other's key. Returns false otherwise.
		inline bool operator <(const Unique & other) const{

			return key_ < other.key_;

		}

	private:

		Unique(unsigned int key){

			key_ = key;

		}

		// Key of this instance.
		unsigned int key_;

	};

	template<typename TTag>
	const Unique<TTag> Unique<TTag>::kNull = Unique<TTag>::MakeUnique();

}