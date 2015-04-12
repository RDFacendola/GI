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

		/// \brief Default constructor.
		Unique();

		/// \brief Copy constructor.
		/// \param other The unique object to move
		Unique(const Unique & other);

		/// \brief Create a new unique instance.
		static Unique MakeUnique();

		/// \brief Tests for equality.
		/// \param other The other instance to test against.
		/// \return Returns true if this object and "other" are the same, returns false otherwise.
		bool operator==(const Unique& other) const;

		/// \brief Tests for inequality.
		/// \param other The other instance to test against.
		/// \return Returns true if this object and "other" are NOT the same, returns false otherwise.
		bool operator!=(const Unique& other) const;

		/// \brief Less-than operator.
		/// \param other The other instance to test against.
		/// \return Returns true if this object's key is lesser than other's key. Returns false otherwise.
		bool operator <(const Unique& other) const;

	private:

		Unique(unsigned int key){

			key_ = key;

		}

		// Key of this instance.
		unsigned int key_;

	};

	//

	template<typename TTag>
	Unique<TTag> Unique<TTag>::MakeUnique(){

		static std::atomic_uint counter_{ 0 };

		return Unique(counter_.fetch_add(1, std::memory_order_relaxed));

	}

	template<typename TTag>
	const Unique<TTag> Unique<TTag>::kNull = Unique<TTag>::MakeUnique();

	template<typename TTag>
	Unique<TTag>::Unique() :
		Unique(kNull){}

	template<typename TTag>
	Unique<TTag>::Unique(const Unique<TTag> & other){
	
		key_ = other.key_;
	
	}

	template<typename TTag>
	inline bool Unique<TTag>::operator==(const Unique<TTag> & other) const{

		return key_ == other.key_;

	}

	template<typename TTag>
	inline bool Unique<TTag>::operator!=(const Unique<TTag> & other) const{

		return key_ != other.key_;

	}

	template<typename TTag>
	inline bool Unique<TTag>::operator <(const Unique<TTag> & other) const{

		return key_ < other.key_;

	}
	
}