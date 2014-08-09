/// \file exceptions.h
/// \brief Defines classes and macros used for exception handling.
///
/// \author Raffaele D. Facendola

#pragma once

#ifdef _WIN32

#include <Windows.h>

#endif

#include <string>
#include <iomanip>
#include <string>
#include <sstream>
#include <map>

#include <StackWalker.h>

using ::std::wstring;
using ::std::stringstream;
using ::std::map;

#ifdef _WIN32

/// If expr fails, throws a runtime exception with detailed informations. "expr" must be of type "HRESULT"; the error code is defined by the value of the expression itself.
#define THROW_ON_FAIL(expr) do{ \
								HRESULT hr = expr; \
								if(FAILED(hr)) { \
									std::wstringstream stream; \
									stream << L"\"" << #expr << "\" failed with 0x" << std::hex << hr << std::dec << std::endl \
										   << __FILE__ << std::endl \
										   << __FUNCTION__ << L" @ " << __LINE__ << std::endl; \
									throw RuntimeException(stream.str(), {{L"error_code", std::to_wstring(hr)}}); \
								} \
							}while (0)


/// If "expr" raises an error, throws a runtime exception with detailed informations. "expr" must support the unary operator "!"; the error code must be returned by GetLastError().
#define THROW_ON_ERROR(expr) do { \
								auto hr = expr; \
								if(!hr){ \
									std::wstringstream stream; \
									stream << L"\"" << #expr << "\" failed with 0x" << std::hex << GetLastError() << std::dec << std::endl \
										   << __FILE__ << std::endl \
										   << __FUNCTION__ << L" @ " << __LINE__ << std::endl; \
									throw RuntimeException(stream.str(), {{L"error_code", std::to_wstring(GetLastError())}}); } \
							}while (0)

/// If expr fails returns from the routine with the fail code
#define RETURN_ON_FAIL(expr) do{ \
								HRESULT hr = expr; \
								if (FAILED(hr)){ \
									return hr; \
								} \
							 }while (0)

#endif

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
	/// \note A convenient way to throw a runtime exception is the following:
	/// \code {.cpp}
	/// throw RuntimeException("message", {{L"arg0", L"value0"}, {L"arg1", L"value1"}})
	/// \endcode
	class RuntimeException{

	public:

		/// \brief Type of the extra arguments.
		typedef map<wstring, wstring> TErrorArgs;

		/// \brief Creates a new exception.
		/// \param error_message The error message associated to the exception
		RuntimeException(const wstring & error_message){

			StackTrace e;

			stack_trace_ = e.GetStackTrace();
			error_message_ = error_message;

		}

		/// \brief Creates a new exception.
		/// \param error_message The error message associated to the exception
		/// \param extras Extra arguments to store within the exception
		RuntimeException(const wstring & error_message, TErrorArgs && extras){

			StackTrace e;

			stack_trace_ = e.GetStackTrace();
			error_message_ = error_message;
			
			extras_ = std::move(extras);

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

		/// \brief Get an extra value by key.
		/// \param key Key of the extra to retrieve.
		/// \return Returns a value corresponding to the extra whose key is the specified one. If no extra is found, an empty string is returned instead.
		inline const wstring & GetExtra(wstring key) const{

			auto it = extras_.find(key);

			if (it != extras_.end()){

				return it->second;

			}else{

				return kEmptyExtra;

			}

		}
	private:

		const wstring kEmptyExtra = L"";

		wstring error_message_;

		wstring stack_trace_;

		TErrorArgs extras_;

	};

}