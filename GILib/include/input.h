/// \file input.h
/// \brief Classes to manage the user input.
///
/// \author Raffaele D. Facendola

#pragma once

#include "gimath.h"

namespace gi_lib{

	class IInput;
	class IMouse;
	class IKeyboard;

	/// \brief Represents the input status.
	/// \author Raffaele D. Facendola
	class IInput{

	public:

		/// \brief Get the mouse interface.
		virtual const IMouse& GetMouseStatus() const = 0;

		/// \brief Get the keyboard interface.
		virtual const IKeyboard& GetKeyboardStatus() const = 0;
		
	};

	/// \brief Represents the status of a mouse.
	/// \author Raffaele D. Facendola
	class IMouse{

	public:

		/// \brief Check whether a button is currently being pressed.
		/// \param button_code Code associated to the button to check,
		/// \return Returns true if the button associated to the specified code is currently being pressed, returns false otherwise.
		virtual bool IsDown(unsigned short button_code) const = 0;

		/// \brief Check whether a button was pressed this frame.
		/// \param button_code Code associated to the button to check,
		/// \return Returns true if the button associated to the specified code has been pressed in the current frame, returns false otherwise.
		virtual bool IsPressed(unsigned short button_code) const = 0;

		/// \brief Check whether a button was released this frame.
		/// \param key_code Code associated to the button to check.
		/// \return Returns true if the button associated to the specified code has been released in the current frame, returns false otherwise.
		virtual bool IsReleased(unsigned short button_code) const = 0;

		/// \brief Get mouse wheel scrolling value.
		/// If the wheel was scrolled up during the last frame, the returned value is positive.
		/// If the wheel was scrolled down, instead, the returned value is negative.
		/// If no scrolling was performed, zero is returned.
		/// \return Returns the value associated to the wheel scroll.
		virtual float GetWheelDelta() const = 0;

		/// \brief Get the mouse coordinates.
		/// \return Returns the mouse coordinates.
		virtual Vector2f GetPosition() const = 0;

		/// \brief Get the mouse movement during the last frame.
		/// \return Returns the mouse movement during the last frame.
		virtual Vector2f GetMovement() const = 0;
		
	};

	/// \brief Represents the status of the keyboard.
	/// \author Raffaele D. Facendola.
	class IKeyboard{

	public:

		/// \brief Check whether a key is currently being pressed.
		/// \param key_code Code associated to the key to check.
		/// \return Returns true if the key associated to the specified code is currently being pressed, returns false otherwise.
		virtual bool IsDown(unsigned short key_code) const = 0;

		/// \brief Check whether a key was pressed this frame.
		/// \param key_code Code associated to the key to check.
		/// \return Returns true if the key associated to the specified code has been pressed in the current frame, returns false otherwise.
		virtual bool IsPressed(unsigned short key_code) const = 0;

		/// \brief Check whether a key was released this frame.
		/// \param key_code Code associated to the key to check.
		/// \return Returns true if the key associated to the specified code has been released in the current frame, returns false otherwise.
		virtual bool IsReleased(unsigned short key_code) const = 0;
		
	};

}