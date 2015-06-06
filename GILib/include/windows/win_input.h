/// \file win_os.h
/// \brief Windows-specific interfaces.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <set>

#include "..\input.h"

using std::set;

namespace gi_lib{

	namespace windows{

		/// \brief Represents the status of a mouse under windows.
		/// \author Raffaele D. Facendola
		class Mouse : public IMouse{

		public:

			Mouse();

			virtual bool IsDown(unsigned short button_code) const override;

			virtual bool IsPressed(unsigned short button_code) const override;

			virtual bool IsReleased(unsigned short button_code) const override;

			virtual float GetWheelDelta() const override;

			virtual Vector2i GetPosition() const override;

			virtual Vector2i GetMovement() const override;
			
			/// \brief Flush the state of the mouse, discarding any temporary state.
			/// The flush is necessary because there could be more than one update of the state per application frame.
			void Flush();

			/// \brief Update the mouse status.
			void UpdateStatus(const RAWMOUSE& mouse_status);

		private:

			set<unsigned short> down_buttons_;						///< \brief Buttons being held down.

			set<unsigned short> pressed_buttons_;					///< \brief Buttons that were pressed during the last frame.

			set<unsigned short> released_buttons_;					///< \brief Buttons that were released during the last frame.

			float wheel_delta_;										///< \brief Wheel delta since last frame.

			Vector2i position_;										///< \brief Absolute position of the mouse.

			Vector2i movement_;										///< \brief Movement of the cursor relative to the last frame.

		};

		/// \brief Represents the status of the keyboard under windows.
		/// \author Raffaele D. Facendola.
		class Keyboard : public IKeyboard{

		public:

			Keyboard();

			virtual bool IsDown(unsigned short key_code) const override;

			virtual bool IsPressed(unsigned short key_code) const override;

			virtual bool IsReleased(unsigned short key_code) const override;

			/// \brief Flush the state of the keyboard, discarding any temporary state.
			/// The flush is necessary because there could be more than one update of the state per application frame.
			void Flush();
						
			/// \brief Update the keyboard status.
			/// The commit is necessary because there could be more than one update of the state per application frame.
			void UpdateStatus(const RAWKEYBOARD& keyboard_status);

		private:

			set<unsigned short> down_keys_;				///< \brief Keys being held down.

			set<unsigned short> pressed_keys_;			///< \brief Keys that were pressed during the last frame.

			set<unsigned short> released_keys_;			///< \brief Keys that were released during the last frame.

		};

		/// \brief Represents the input status under windows.
		/// \author Raffaele D. Facendola
		class Input : public IInput{

		public:

			/// \brief Get the mouse interface.
			virtual const Mouse& GetMouseStatus() const override;

			/// \brief Get the keyboard interface.
			virtual const Keyboard& GetKeyboardStatus() const override;

			/// \brief Flush the state of the input peripherals, discarding any temporary state.
			/// The flush is necessary because there could be more than one update of the state per application frame.
			void Flush();

			/// \brief Process a windows message.
			/// \param result Output parameter. Result of the message processing.
			/// \return Returns true if the message was processed, returns false otherwise.
			bool ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter, LRESULT& result);

		private:

			Mouse mouse_;				///< \brief Mouse state

			Keyboard keyboard_;			///< \brief Keyboard state

		};

		///////////////////////////////// INPUT ////////////////////////////////////////////

		inline const Mouse& Input::GetMouseStatus() const{

			return mouse_;
			
		}

		inline const Keyboard& Input::GetKeyboardStatus() const{

			return keyboard_;

		}

		inline void Input::Flush(){

			mouse_.Flush();
			keyboard_.Flush();

		}

		//////////////////////////////// MOUSE //////////////////////////////////////////////

		inline bool Mouse::IsDown(unsigned short button_code) const{

			return down_buttons_.find(button_code) != down_buttons_.end();
			
		}

		inline bool Mouse::IsPressed(unsigned short button_code) const{

			return pressed_buttons_.find(button_code) != pressed_buttons_.end();

		}

		inline bool Mouse::IsReleased(unsigned short button_code) const{

			return released_buttons_.find(button_code) != released_buttons_.end();

		}

		inline float Mouse::GetWheelDelta() const{

			return wheel_delta_;

		}

		inline Vector2i Mouse::GetPosition() const{

			return position_;

		}

		inline Vector2i Mouse::GetMovement() const{

			return movement_;

		}

		/////////////////////////////// KEYBOARD ////////////////////////////////////

		inline bool Keyboard::IsDown(unsigned short button_code) const{

			return down_keys_.find(button_code) != down_keys_.end();

		}

		inline bool Keyboard::IsPressed(unsigned short button_code) const{

			return pressed_keys_.find(button_code) != pressed_keys_.end();

		}

		inline bool Keyboard::IsReleased(unsigned short button_code) const{

			return released_keys_.find(button_code) != released_keys_.end();

		}
		
	}

}

#endif