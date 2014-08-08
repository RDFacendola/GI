#pragma once

#include "application.h"

using ::gi_lib::Window;
using ::gi_lib::Timer;

class Timer;

///Application's logic
class GILogic : public Window{

public:

	GILogic();

	virtual ~GILogic();

	/// \brief Update the window logic.
	/// \param time The application-coherent time.
	virtual void Update(const Timer::Time & time);

#ifdef _WIN32

	/// \brief Handle a Windows message.
	/// \param message_id The message.
	/// \param wparameter Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
	/// \param lparameterAdditional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
	/// \return The return value specifies the result of the message processing and depends on the message sent.
	virtual LRESULT ReceiveMessage(unsigned int message_id, WPARAM wparameter, LPARAM lparameter);

#endif

};