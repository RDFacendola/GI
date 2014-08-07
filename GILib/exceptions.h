/// \file exceptions.h
/// \brief Defines classes and macros used for exception handling.
///
/// \author Raffaele D. Facendola

#pragma once

#include <string>
#include <iomanip>
#include <string>
#include <sstream>

#include "StackWalker.h"

using ::std::wstring;
using ::std::stringstream;

/// If expr fails throws a runtime exception with detailed informations
#define THROW_ON_FAIL(expr) do \
							{ \
								HRESULT hr = expr; \
								if(FAILED(hr)) \
								{ \
									std::wstringstream stream; \
									stream << "FAILED! " << #expr << " (0x" << std::hex << hr << std::dec << ")" << std::endl \
										   << __FILE__ << std::endl \
										   << __FUNCTIONW__ << L" (" << __LINE__ << ")" << std::endl; \
									throw RuntimeException(stream.str()); \
								} \
							}while (0)

/// If expr fails returns from the routine with the fail code
#define RETURN_ON_FAIL(expr) do \
							 { \
								HRESULT hr = expr; \
								if (FAILED(hr)){ \
									return hr; \
								} \
							 }while (0)

namespace gi_lib{

	/// \brief Manages the stack trace.
	class StackTrace : public StackWalker{

	public:

		/// \brief Default constructor.
		StackTrace() : StackWalker(StackWalkOptions::RetrieveNone) {}

		/// \brief Get the current stack trace.
		/// \return Returns the current stack trace as a string.
		inline wstring GetStackTrace(){

			//Remove the content of the stack trace
			stack_trace.str("");

			ShowCallstack();

			auto trace = stack_trace.str();

			return wstring(trace.begin(), trace.end());

		}

	protected:

		/// \brief Callback used to log each stack entry.
		/// \param eType Type of the entry.
		/// \param entry Details about the entry.
		virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry){

			StackWalker::OnCallstackEntry(eType, entry);

			if (entry.offset == 0){

				return;	//Invalid entry

			}

			stack_trace << entry.moduleName
						<< " - "
						<< entry.undFullName
						<< " ("
						<< std::to_string(entry.lineNumber)
						<< ")"
						<< std::endl;

		}

	private:

		stringstream stack_trace;

	};

	/// \brief Runtime exception
	class RuntimeException{

	public:

		/// \brief Creates a new exception.
		/// \param error_message The error message associated to the exception
		RuntimeException(const wstring & error_message){

			StackTrace e;

			stack_trace_ = e.GetStackTrace();
			error_message_ = error_message;

		}

		/// \brief Get the error message.
		/// \return Returns the error message.
		const wstring & GetErrorMessage() const{

			return error_message_;

		}

		/// \brief Get the stack trace.
		/// \return Returns the stack trace.
		const wstring & GetStackTrace() const{

			return stack_trace_;

		}

	private:

		wstring error_message_;

		wstring stack_trace_;

	};

}