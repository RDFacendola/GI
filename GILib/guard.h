/// \file guard.h
/// \brief Defines guards that wraps various RAII functionalities
///
/// \author Raffaele D. Facendola

#pragma once

#include <functional>

using ::std::function;

namespace gi_lib{

	/// /brief Guards that executes a routine upon destruction unless it was dismissed.
	class ScopeGuard{

	public:

		/// \brief Create a new scope guard.
		/// \tparam TFunctor Type of the functor.
		/// \param functor Functor that wraps the routine to be executed upon this instance's destruction.
		template <typename TFunctor>
		ScopeGuard(TFunctor functor) :
			functor_(functor),
			dismissed_(false){}

		/// \brief Copy constructor. The original copy gets dismissed.
		/// \param other Original scope guard to copy from.
		ScopeGuard(ScopeGuard & other) :
			functor_(other.functor_),
			dismissed_(other.dismissed_){

			other.Dismiss();

		}

		/// \brief Destroy this instance and calls the functor it wraps unless the guard has been dismissed.
		~ScopeGuard(){

			if (!dismissed_){

				///Call the functor if not dismissed
				functor_();

			}

		}

		/// \brief Dismiss the scope guard. The routine won't be called upon guard's destruction anymore.
		inline void Dismiss(){

			dismissed_ = true;

		}

	private:

		std::function<void(void)> functor_;

		bool dismissed_;

	};

}