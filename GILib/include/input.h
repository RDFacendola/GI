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

    /// \brief Key-codes for keyboard inputs.
    /// \remarks Those are valid only for UK layout
    enum class KeyCode : unsigned short {

        KEY_Q = 16,
        KEY_W = 17,
        KEY_E = 18,
        KEY_R = 19,
        KEY_T = 20,
        KEY_Y = 21,
        KEY_U = 22,
        KEY_I = 23,
        KEY_O = 24,
        KEY_P = 25,

        KEY_A = 30,
        KEY_S = 31,
        KEY_D = 32,
        KEY_F = 33,
        KEY_G = 34,
        KEY_H = 35,
        KEY_J = 36,
        KEY_K = 37,
        KEY_L = 38,

        KEY_Z = 44,
        KEY_X = 45,
        KEY_C = 46,
        KEY_V = 47,
        KEY_B = 48,
        KEY_N = 49,
        KEY_M = 50,

        KEY_UP_ARROW = 72,
        KEY_LEFT_ARROW = 75,
        KEY_DOWN_ARROW = 80,
        KEY_RIGHT_ARROW = 77

    };

    /// \brief Button-codes for mouse inputs.
    enum class ButtonCode : unsigned short {

        LEFT_BUTTON = 0,
        RIGHT_BUTTON = 1,
        MIDDLE_BUTTON = 2,
        THUMB_BUTTON_0 = 3,
        THUMB_BUTTON_1 = 4

    };

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
        virtual bool IsDown(ButtonCode button_code) const = 0;

        /// \brief Check whether a button was pressed this frame.
        /// \param button_code Code associated to the button to check,
        /// \return Returns true if the button associated to the specified code has been pressed in the current frame, returns false otherwise.
        virtual bool IsPressed(ButtonCode button_code) const = 0;

        /// \brief Check whether a button was released this frame.
        /// \param key_code Code associated to the button to check.
        /// \return Returns true if the button associated to the specified code has been released in the current frame, returns false otherwise.
        virtual bool IsReleased(ButtonCode button_code) const = 0;

        /// \brief Get mouse wheel scrolling value.
        /// If the wheel was scrolled up during the last frame, the returned value is positive.
        /// If the wheel was scrolled down, instead, the returned value is negative.
        /// If no scrolling was performed, zero is returned.
        /// \return Returns the value associated to the wheel scroll.
        virtual float GetWheelDelta() const = 0;

        /// \brief Get the mouse coordinates.
        /// \return Returns the mouse coordinates.
        virtual Vector2i GetPosition() const = 0;

        /// \brief Get the mouse movement during the last frame.
        /// \return Returns the mouse movement during the last frame.
        virtual Vector2i GetMovement() const = 0;
        
    };

    /// \brief Represents the status of the keyboard.
    /// \author Raffaele D. Facendola.
    class IKeyboard{

    public:

        /// \brief Check whether a key is currently being pressed.
        /// \param key_code Code associated to the key to check.
        /// \return Returns true if the key associated to the specified code is currently being pressed, returns false otherwise.
        virtual bool IsDown(KeyCode key_code) const = 0;

        /// \brief Check whether a key was pressed this frame.
        /// \param key_code Code associated to the key to check.
        /// \return Returns true if the key associated to the specified code has been pressed in the current frame, returns false otherwise.
        virtual bool IsPressed(KeyCode key_code) const = 0;

        /// \brief Check whether a key was released this frame.
        /// \param key_code Code associated to the key to check.
        /// \return Returns true if the key associated to the specified code has been released in the current frame, returns false otherwise.
        virtual bool IsReleased(KeyCode key_code) const = 0;
        
    };

}