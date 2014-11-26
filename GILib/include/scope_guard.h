/// \file scope_guard.h
/// \brief Defines guards that wraps various RAII functionalities
///
/// \author Raffaele D. Facendola

#pragma once

#include <functional>

using ::std::function;

namespace gi_lib{

	/// \brief Guards that executes a routine upon destruction unless it was dismissed.

	/// C++ and Beyond 2012: Systematic Error Handling in C++ - Andrei Alexandrescu
	/// http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
	/// \author Andrei Alexandrescu
	template <typename TFunctor>
	class ScopeGuard{

	public:

		/// \brief No default constructor.
		ScopeGuard() = delete;

		/// \brief No copy constructor.
		ScopeGuard(const ScopeGuard &) = delete;

		/// \brief No assignment operator.
		ScopeGuard & operator=(const ScopeGuard &) = delete;

		/// \brief Create a new scope guard.
		/// \tparam TFunctor Type of the functor.
		/// \param functor Functor that wraps the routine to be executed upon this instance's destruction.
		ScopeGuard(TFunctor functor) :
			functor_(std::move(functor)),
			dismissed_(false){}

		/// \brief Move constructor. The original copy gets dismissed.
		/// \param other Original scope guard to move.
		ScopeGuard(ScopeGuard && other) :
			functor_(std::move(other.functor_)),
			dismissed_(other.dismissed_){

			other.Dismiss();

		}

		/// \brief Destroy this instance and calls the functor unless the guard has been dismissed.
		~ScopeGuard(){

			if (!dismissed_){

				//Call the functor if not dismissed
				functor_();

			}

		}

		/// \brief Dismiss the scope guard. The routine won't be called upon guard's destruction anymore.
		inline void Dismiss(){

			dismissed_ = true;

		}

	private:

		TFunctor functor_;

		bool dismissed_;

	};

	/// \brief Create a new scope guard.
	/// \tparam TFunctor Type of the functor the ScopeGuard will call upon destruction.
	template <typename TFunctor>
	ScopeGuard<TFunctor> make_scope_guard(TFunctor functor){

		return ScopeGuard<TFunctor>(std::move(functor));

	}

}