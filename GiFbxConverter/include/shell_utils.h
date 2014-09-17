/// \file shell_utils.h
/// \brief Methods and utility classes for shell commands.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>

using std::string;

namespace gi_lib{

	/// \brief Console utility class.
	class ShellUtils{

	public:

		/// \brief Execute a command silently.
		/// \param command Command to issue.
		/// \return Returns the output of the command.
		static string Execute(const string & command);

	};

}
