/// \file timer.h
/// \brief Defines classes used to manage the application's time.
///
/// \author Raffaele D. Facendola

#pragma once

#include <ctime>
#include <ratio>
#include <chrono>

using ::std::chrono::high_resolution_clock;
using ::std::chrono::duration;

namespace gi_lib{

	class Time;

	/// \brief High resolution timer.
	/// \author Raffaele D. Facendola
	class Timer
	{

	public:

		/// \brief Default constructor.

		/// The timer starts to count automatically upon construction.
		Timer(){

			Restart();

		}

		/// \brief Restart the timer
		inline void Restart(){

			start_ = clock_.now();
			last_ = start_;

		}

		/// \brief Get the current time.

		/// The time returned holds informations about the total amount of seconds
		/// passed since the last restart and the number of seconds passed since the last
		/// time GetTime() was called.
		/// \return Returns the current time.
		Time GetTime();

	private:

		high_resolution_clock clock_;

		high_resolution_clock::time_point start_;

		high_resolution_clock::time_point last_;

	};

	/// \brief Hold informations about the application time.
	/// \author Raffaele D. Facendola
	class Time{

	public:

		/// \brief Measure unit for time
		typedef duration<float> Seconds;

		/// \brief Class default constructor.
		Time() :
			total_seconds_(0),
			delta_seconds_(0){}

		/// \brief Class constructor.

		/// \param total_seconds Seconds passed since the beginning of the application.
		/// \param delta_seconds Seconds passed since the last update.
		Time(Seconds total_seconds, Seconds delta_seconds) :
			total_seconds_(total_seconds),
			delta_seconds_(delta_seconds){}

		/// \brief Get the seconds passed since the beginning of the application.

		/// \return Returns the seconds passed since the beginning of the application.
		inline float GetTotalSeconds() const{

			return total_seconds_.count();

		}

		/// \brief Get the seconds passed since the last update.

		/// \return Returns the seconds passed since the last update.
		inline float GetDeltaSeconds() const{

			return delta_seconds_.count();

		}

	private:

		Seconds total_seconds_;

		Seconds delta_seconds_;

	};

}

