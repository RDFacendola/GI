/// \file win_os.h
/// \brief Windows-specific interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <Windows.h>

#include "..\input.h"

namespace gi_lib{

	namespace windows{

		/// \brief Represents the status of a mouse under windows os.
		/// \author Raffaele D. Facendola
		class Mouse : public IMouse{

		public:

			Mouse();

			virtual bool IsDown(unsigned short button_code) override;

			virtual bool IsPressed(unsigned short button_code) override;

			virtual bool IsReleased(unsigned short button_code) override;

			virtual float GetWheelDelta() override;

			virtual Vector2f GetPosition() override;

			virtual Vector2f GetMovement() override;

		};

		/// \brief Represents the status of the keyboard under windows os.
		/// \author Raffaele D. Facendola.
		class Keyboard : public IKeyboard{

		public:

			Keyboard();

			virtual bool IsDown(unsigned short key_code) const override;

			virtual bool IsPressed(unsigned short key_code) const override;

			virtual bool IsReleased(unsigned short key_code) const override;

		};

	}

}

#endif